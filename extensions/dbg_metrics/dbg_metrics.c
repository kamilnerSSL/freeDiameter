/*
 * metrics.c
 * Prometheus Exporter for freeDiameter Standalone Daemon
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// freeDiameter includes
#include <freeDiameter/extension.h>
#include <freeDiameter/libfdcore.h>
#include <freeDiameter/libfdproto.h>
#include "../libfdcore/fdcore-internal.h"

// HTTP Server include
#include <microhttpd.h>

#define EXPORTER_PORT 9090

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

/* --- 3. HTTP Handler --- */
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

    // -- Header --
    buf_append(&buf, "# HELP fd_peer_state Peer State: 0=New, 1=Open, 2=Closed, 3=Closing, 4=WaitCnxAck, 5=WaitCnxAckElec, 6=WaitCEA, 8=Suspect, 9=Reopen, 12=Zombie\n");
    buf_append(&buf, "# TYPE fd_peer_state gauge\n");

    // -- Access freeDiameter Internals --
    // We must lock the list to safely iterate it
    int ret = pthread_rwlock_rdlock(&fd_g_peers_rw);
    if (ret != 0) {
        buf_append(&buf, "# Error: Could not lock peer list\n");
    } else {
        struct fd_list *li;

        // Iterate over all peers (in any state)
        for (li = fd_g_peers.next; li != &fd_g_peers; li = li->next) {
            
            // Extract the peer header from the list node
            struct peer_hdr *peer_hdr = (struct peer_hdr *)li->o;

            // Extract Peer Identity (Diameter Identity)
            char *peer_id = peer_hdr->info.pi_diamid;
            if (!peer_id) peer_id = "unknown";

            // Extract State using the public API
            enum peer_state state = fd_peer_getstate(peer_hdr);
            int state_int = map_state_to_int(state);

            // Add to buffer
            buf_append(&buf, "fd_peer_state{peer=\"%s\"} %d\n", peer_id, state_int);
        }
        
        pthread_rwlock_unlock(&fd_g_peers_rw);
    }

    // -- Create Response --
    struct MHD_Response *response;
    response = MHD_create_response_from_buffer(buf.size, (void *)buf.data, MHD_RESPMEM_MUST_COPY);
    
    // Clean up our local buffer (MHD made a copy)
    buf_free(&buf);

    int mhd_ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    
    return mhd_ret;
}

/* --- 4. Extension Entry Point --- */
static struct MHD_Daemon *http_daemon = NULL;

// Called when freeDiameter loads the extension
static int metrics_entry(char * conffile) {
    
    fd_log_debug("[metrics] Loading Metrics Extension on port %d...", EXPORTER_PORT);

    // Start the HTTP Server in a background thread
    http_daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, EXPORTER_PORT, NULL, NULL, 
                                   &handle_request, NULL, MHD_OPTION_END);

    if (http_daemon == NULL) {
        fd_log_debug("[metrics] Failed to start HTTP server.");
        return 1; // Error
    }

    fd_log_debug("[metrics] HTTP server started successfully.");
    return 0; // Success
}

// Called when freeDiameter shuts down (Optional but good practice)
void fd_ext_fini(void) {
    if (http_daemon) {
        MHD_stop_daemon(http_daemon);
    }
}

// Register the entry point
EXTENSION_ENTRY("metrics", metrics_entry);