/*
 * metrics.c
 * Prometheus Exporter for freeDiameter Standalone Daemon
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// freeDiameter includes
#include <freeDiameter/extension.h>
#include <freeDiameter/libfdcore.h>
#include <freeDiameter/libfdproto.h>
#include "../libfdcore/fdcore-internal.h"

// HTTP Server include
#include <microhttpd.h>

#define EXPORTER_PORT 9090
#define EXPORTER_ADDR "127.0.0.1"

/* Global configuration */
static uint16_t metrics_port = EXPORTER_PORT;
static const char *metrics_addr = EXPORTER_ADDR;

/* --- 1. Dynamic Buffer Helper --- 
 * (Allows us to construct large responses safely) 
 */
struct resp_buffer {
    char *data;
    size_t size;
    size_t capacity;
};

void buf_init(struct resp_buffer *b) {
    b->capacity = 4096; // Start with 4KB
    b->data = malloc(b->capacity);
    b->size = 0;
    if (b->data) b->data[0] = '\0';
}

void buf_append(struct resp_buffer *b, const char *fmt, ...) {
    if (!b->data) return;

    va_list args;
    va_start(args, fmt);
    
    // Calculate required size
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (len < 0) return;

    // Resize if necessary
    if (b->size + len + 1 >= b->capacity) {
        size_t new_cap = b->capacity * 2 + len;
        char *new_data = realloc(b->data, new_cap);
        if (!new_data) return; // OOM check
        b->data = new_data;
        b->capacity = new_cap;
    }

    // Append actual string
    va_start(args, fmt);
    vsnprintf(b->data + b->size, b->capacity - b->size, fmt, args);
    va_end(args);
    
    b->size += len;
}

void buf_free(struct resp_buffer *b) {
    if (b->data) free(b->data);
}

/* --- 2. State Mapping Helper --- */
int map_state_to_int(enum peer_state state) {
    switch (state) {
        /* Stable states */
        case STATE_NEW:             return 0;
        case STATE_OPEN:            return 1;
        
        /* Peer state machine */
        case STATE_CLOSED:          return 2;
        case STATE_CLOSING:         return 3;
        case STATE_WAITCNXACK:      return 4;
        case STATE_WAITCNXACK_ELEC: return 5;
        case STATE_WAITCEA:         return 6;
        
        /* Debug / Transitional */
        case STATE_OPEN_HANDSHAKE:  return 7;
        
        /* Failover state machine */
        case STATE_SUSPECT:         return 8;
        case STATE_REOPEN:          return 9;
        
        /* Ordering & Grace periods */
        case STATE_OPEN_NEW:        return 10;
        case STATE_CLOSING_GRACE:   return 11;
        
        /* Error state */
        case STATE_ZOMBIE:          return 12;
        
        default:                    return -1; // Unknown state
    }
}

/* --- 3. Config File Parser --- */
static int parse_config(const char *conffile) {
    FILE *f;
    char buffer[256];
    char *line;
    int ret = 0;

    if (!conffile) {
        fd_log_debug("[metrics] No configuration file provided, using defaults (port=%d, addr=%s)", metrics_port, metrics_addr);
        return 0;
    }

    f = fopen(conffile, "r");
    if (!f) {
        fd_log_debug("[metrics] Could not open configuration file '%s': %s", conffile, strerror(errno));
        return 0;  // Use defaults if file doesn't exist
    }

    while (fgets(buffer, sizeof(buffer), f)) {
        line = buffer;
        
        // Skip whitespace and comments
        while (*line && (*line == ' ' || *line == '\t')) line++;
        if (!*line || *line == '#' || *line == '\n') continue;

        // Parse "port = <number>"
        if (strncasecmp(line, "port", 4) == 0) {
            char *val = line + 4;
            while (*val && (*val == ' ' || *val == '=' || *val == '\t')) val++;
            unsigned long port_val = strtoul(val, NULL, 10);
            if (port_val > 0 && port_val <= 65535) {
                metrics_port = (uint16_t)port_val;
                fd_log_debug("[metrics] Config: port = %u", metrics_port);
            } else {
                fd_log_debug("[metrics] Config: Invalid port value '%s'", val);
                ret = 1;
            }
            continue;
        }

        // Parse "addr = <address>"
        if (strncasecmp(line, "addr", 4) == 0 || strncasecmp(line, "address", 7) == 0) {
            char *val = strchr(line, '=');
            if (!val) continue;
            val++;  // Skip '='
            while (*val && (*val == ' ' || *val == '\t')) val++;  // Skip whitespace
            
            // Remove trailing newline/whitespace
            char *end = val + strlen(val) - 1;
            while (end > val && (*end == '\n' || *end == '\r' || *end == ' ' || *end == '\t')) end--;
            *(end + 1) = '\0';
            
            metrics_addr = strdup(val);
            if (!metrics_addr) {
                fd_log_debug("[metrics] Memory allocation failed for address");
                ret = 1;
            } else {
                fd_log_debug("[metrics] Config: addr = %s", metrics_addr);
            }
            continue;
        }
    }

    fclose(f);
    return ret;
}

/* --- 4. Queue Stats Helper --- */
void append_queue_metrics(struct resp_buffer *buf, const char *queue_name, 
                          int current, int limit, int highest, long long total) {
    buf_append(buf, "fd_queue_current{queue=\"%s\"} %d\n", queue_name, current);
    buf_append(buf, "fd_queue_limit{queue=\"%s\"} %d\n", queue_name, limit);
    buf_append(buf, "fd_queue_highest{queue=\"%s\"} %d\n", queue_name, highest);
    buf_append(buf, "fd_queue_total{queue=\"%s\"} %lld\n", queue_name, total);
}

/* --- 5. HTTP Handler --- */
static int handle_request(void *cls, struct MHD_Connection *connection,
                          const char *url, const char *method,
                          const char *version, const char *upload_data,
                          size_t *upload_data_size, void **con_cls) {

    if (strcmp(method, "GET") != 0) {
        return MHD_NO;
    }

    // Health check endpoint
    if (strcmp(url, "/") == 0 || strcmp(url, "/health") == 0) {
        const char *health_response = "OK";
        struct MHD_Response *response;
        response = MHD_create_response_from_buffer(strlen(health_response), 
                                                    (void *)health_response, 
                                                    MHD_RESPMEM_PERSISTENT);
        int mhd_ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return mhd_ret;
    }

    // Metrics endpoint
    if (strcmp(url, "/metrics") != 0) {
        return MHD_NO;
    }

    struct resp_buffer buf;
    buf_init(&buf);

    // -- Peer State Metrics --
    buf_append(&buf, "# HELP fd_peer_state Peer State: 0=New, 1=Open, 2=Closed, 3=Closing, 4=WaitCnxAck, 5=WaitCnxAckElec, 6=WaitCEA, 7=OpenHandshake, 8=Suspect, 9=Reopen, 10=OpenNew, 11=ClosingGrace, 12=Zombie, 13=Unknown\n");
    buf_append(&buf, "# TYPE fd_peer_state gauge\n");
    buf_append(&buf, "# HELP fd_peer_messages_received_total Total messages received from peer (processed through state machine)\n");
    buf_append(&buf, "# TYPE fd_peer_messages_received_total counter\n");
    buf_append(&buf, "# HELP fd_peer_messages_sent_total Total messages sent to peer\n");
    buf_append(&buf, "# TYPE fd_peer_messages_sent_total counter\n");
    buf_append(&buf, "# HELP fd_peer_application_support Application support: 1=auth, 2=acct, 3=both\n");
    buf_append(&buf, "# TYPE fd_peer_application_support gauge\n");

    int ret = pthread_rwlock_rdlock(&fd_g_peers_rw);
    if (ret != 0) {
        buf_append(&buf, "# Error: Could not lock peer list\n");
    } else {
        struct fd_list *li;

        // Iterate over all peers (in any state)
        for (li = fd_g_peers.next; li != &fd_g_peers; li = li->next) {
            struct peer_hdr *peer_hdr = (struct peer_hdr *)li->o;
            char *peer_id = peer_hdr->info.pi_diamid;
            if (!peer_id) peer_id = "unknown";

            enum peer_state state = fd_peer_getstate(peer_hdr);
            int state_int = map_state_to_int(state);

            buf_append(&buf, "fd_peer_state{peer=\"%s\"} %d\n", peer_id, state_int);

            // Get per-peer message counts
            long long psm_total = 0, tosend_total = 0;
            int dummy_current, dummy_limit, dummy_highest;
            struct timespec dummy_time;

            if (fd_stat_getstats(STAT_P_PSM, peer_hdr, &dummy_current, &dummy_limit, &dummy_highest, &psm_total, &dummy_time, &dummy_time, &dummy_time) == 0) {
                buf_append(&buf, "fd_peer_messages_received_total{peer=\"%s\"} %lld\n", peer_id, psm_total);
            }

            if (fd_stat_getstats(STAT_P_TOSEND, peer_hdr, &dummy_current, &dummy_limit, &dummy_highest, &tosend_total, &dummy_time, &dummy_time, &dummy_time) == 0) {
                buf_append(&buf, "fd_peer_messages_sent_total{peer=\"%s\"} %lld\n", peer_id, tosend_total);
            }

            // Get peer supported applications
            struct fd_list *app_li;
            for (app_li = peer_hdr->info.runtime.pir_apps.next; app_li != &peer_hdr->info.runtime.pir_apps; app_li = app_li->next) {
                struct fd_app *app = (struct fd_app *)app_li;
                if (app) {
                    // app->flags.auth = 1 if authentication application, acct = 1 if accounting
                    // Use a combined value: 1=auth only, 2=acct only, 3=both
                    int app_type = (app->flags.auth ? 1 : 0) + (app->flags.acct ? 2 : 0);
                    if (app_type > 0) {
                        buf_append(&buf, "fd_peer_application_support{peer=\"%s\",appid=\"%u\"} %d\n", peer_id, app->appid, app_type);
                    }
                }
            }
        }
        
        pthread_rwlock_unlock(&fd_g_peers_rw);
    }

    // -- Queue Statistics --
    buf_append(&buf, "# HELP fd_queue_current Current queue depth\n");
    buf_append(&buf, "# TYPE fd_queue_current gauge\n");
    buf_append(&buf, "# HELP fd_queue_limit Queue size limit\n");
    buf_append(&buf, "# TYPE fd_queue_limit gauge\n");
    buf_append(&buf, "# HELP fd_queue_highest Highest queue depth reached\n");
    buf_append(&buf, "# TYPE fd_queue_highest gauge\n");
    buf_append(&buf, "# HELP fd_queue_total Total items processed\n");
    buf_append(&buf, "# TYPE fd_queue_total counter\n");

    int current_count, limit_count, highest_count;
    long long total_count;
    struct timespec total, blocking, last;

    // Global queues
    if (fd_stat_getstats(STAT_G_LOCAL, NULL, &current_count, &limit_count, &highest_count, &total_count, &total, &blocking, &last) == 0) {
        append_queue_metrics(&buf, "local_delivery", current_count, limit_count, highest_count, total_count);
    }

    if (fd_stat_getstats(STAT_G_INCOMING, NULL, &current_count, &limit_count, &highest_count, &total_count, &total, &blocking, &last) == 0) {
        append_queue_metrics(&buf, "total_received", current_count, limit_count, highest_count, total_count);
    }

    if (fd_stat_getstats(STAT_G_OUTGOING, NULL, &current_count, &limit_count, &highest_count, &total_count, &total, &blocking, &last) == 0) {
        append_queue_metrics(&buf, "total_sending", current_count, limit_count, highest_count, total_count);
    }

    // Per-peer queues
    ret = pthread_rwlock_rdlock(&fd_g_peers_rw);
    if (ret == 0) {
        struct fd_list *li;
        for (li = fd_g_peers.next; li != &fd_g_peers; li = li->next) {
            struct peer_hdr *peer_hdr = (struct peer_hdr *)li->o;
            char *peer_id = peer_hdr->info.pi_diamid;
            if (!peer_id) peer_id = "unknown";

            // PSM queue (events)
            if (fd_stat_getstats(STAT_P_PSM, peer_hdr, &current_count, &limit_count, &highest_count, &total_count, &total, &blocking, &last) == 0) {
                buf_append(&buf, "fd_peer_psm_queue_current{peer=\"%s\"} %d\n", peer_id, current_count);
                buf_append(&buf, "fd_peer_psm_queue_limit{peer=\"%s\"} %d\n", peer_id, limit_count);
                buf_append(&buf, "fd_peer_psm_queue_highest{peer=\"%s\"} %d\n", peer_id, highest_count);
                buf_append(&buf, "fd_peer_psm_queue_total{peer=\"%s\"} %lld\n", peer_id, total_count);
            }

            // Outgoing queue
            if (fd_stat_getstats(STAT_P_TOSEND, peer_hdr, &current_count, &limit_count, &highest_count, &total_count, &total, &blocking, &last) == 0) {
                buf_append(&buf, "fd_peer_tosend_queue_current{peer=\"%s\"} %d\n", peer_id, current_count);
                buf_append(&buf, "fd_peer_tosend_queue_limit{peer=\"%s\"} %d\n", peer_id, limit_count);
                buf_append(&buf, "fd_peer_tosend_queue_highest{peer=\"%s\"} %d\n", peer_id, highest_count);
                buf_append(&buf, "fd_peer_tosend_queue_total{peer=\"%s\"} %lld\n", peer_id, total_count);
            }
        }
        pthread_rwlock_unlock(&fd_g_peers_rw);
    }

    // -- Create Response --
    struct MHD_Response *response;
    response = MHD_create_response_from_buffer(buf.size, (void *)buf.data, MHD_RESPMEM_MUST_COPY);
    
    buf_free(&buf);

    int mhd_ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    
    return mhd_ret;
}

/* --- 6. Extension Entry Point --- */
static struct MHD_Daemon *http_daemon = NULL;

static int metrics_entry(char * conffile) {
    struct sockaddr_in addr;

    // Parse configuration file
    if (parse_config(conffile) != 0) {
        fd_log_debug("[metrics] Configuration parsing had errors but continuing with settings");
    }
    
    fd_log_debug("[metrics] Loading Metrics Extension on %s:%u...", metrics_addr, metrics_port);

    // Setup address binding
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(metrics_port);
    
    // Parse the IP address
    if (inet_pton(AF_INET, metrics_addr, &addr.sin_addr) != 1) {
        fd_log_debug("[metrics] Invalid IPv4 address '%s', using 127.0.0.1", metrics_addr);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    }

    http_daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, metrics_port, NULL, NULL, 
                                   &handle_request, NULL, MHD_OPTION_SOCK_ADDR, &addr, MHD_OPTION_END);

    if (http_daemon == NULL) {
        fd_log_debug("[metrics] Failed to start HTTP server on %s:%u.", metrics_addr, metrics_port);
        return 1;
    }

    fd_log_debug("[metrics] HTTP server started successfully on %s:%u.", metrics_addr, metrics_port);
    return 0;
}

void fd_ext_fini(void) {
    if (http_daemon) {
        MHD_stop_daemon(http_daemon);
    }
}

EXTENSION_ENTRY("metrics", metrics_entry);