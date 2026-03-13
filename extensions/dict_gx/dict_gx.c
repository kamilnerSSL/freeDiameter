/*********************************************************************************************************
 * Software License Agreement (BSD License)                                                               *
 * Author: Sebastien Decugis <sdecugis@freediameter.net>                                                  *
 *                                                                                                        *
 * Copyright (c) 2023, WIDE Project and NICT                                                              *
 * All rights reserved.                                                                                   *
 *                                                                                                        *
 * Redistribution and use of this software in source and binary forms, with or without modification, are  *
 * permitted provided that the following conditions are met:                                              *
 *                                                                                                        *
 * * Redistributions of source code must retain the above                                                 *
 *   copyright notice, this list of conditions and the                                                    *
 *   following disclaimer.                                                                                *
 *                                                                                                        *
 * * Redistributions in binary form must reproduce the above                                              *
 *   copyright notice, this list of conditions and the                                                    *
 *   following disclaimer in the documentation and/or other                                               *
 *   materials provided with the distribution.                                                            *
 *                                                                                                        *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED *
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR *
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT     *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    *
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR *
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF   *
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                             *
 *********************************************************************************************************/

/* 
 * Dictionary definitions of objects specified in 3GPP TS 29.212 (Gx).
 */
#include <freeDiameter/extension.h>

/* The content of this file follows the same structure as dict_base_proto.c */

#define CHECK_dict_new( _type, _data, _parent, _ref )                                     \
    {                                                                                     \
        int _ret = fd_dict_new(fd_g_config->cnf_dict, (_type), (_data), (_parent), (_ref)); \
        if (_ret != 0 && _ret != EEXIST) {                                                \
            CHECK_FCT(_ret);                                                              \
        }                                                                                 \
    }

#define CHECK_dict_search( _type, _criteria, _what, _result )	\
	CHECK_FCT(  fd_dict_search( fd_g_config->cnf_dict, (_type), (_criteria), (_what), (_result), ENOENT) );

struct local_rules_definition {
	struct dict_avp_request avp_vendor_plus_name;
	enum rule_position	position;
	int 			min;
	int			max;
};

#define RULE_ORDER( _position ) ((((_position) == RULE_FIXED_HEAD) || ((_position) == RULE_FIXED_TAIL)) ? 1 : 0 )

#define PARSE_loc_rules( _rulearray, _parent) {								\
	int __ar;											\
	for (__ar=0; __ar < sizeof(_rulearray) / sizeof((_rulearray)[0]); __ar++) {			\
		struct dict_rule_data __data = { NULL, 							\
			(_rulearray)[__ar].position,							\
			0, 										\
			(_rulearray)[__ar].min,								\
			(_rulearray)[__ar].max};							\
		__data.rule_order = RULE_ORDER(__data.rule_position);					\
		CHECK_FCT(  fd_dict_search( 								\
			fd_g_config->cnf_dict,								\
			DICT_AVP, 									\
			AVP_BY_NAME_AND_VENDOR, 							\
			&(_rulearray)[__ar].avp_vendor_plus_name,					\
			&__data.rule_avp, 0 ) );							\
		if ( !__data.rule_avp ) {								\
			TRACE_DEBUG(INFO, "AVP Not found: '%s'", (_rulearray)[__ar].avp_vendor_plus_name.avp_name);		\
			return ENOENT;									\
		}											\
		CHECK_FCT_DO( fd_dict_new( fd_g_config->cnf_dict, DICT_RULE, &__data, _parent, NULL),	\
			{							        		\
				TRACE_DEBUG(INFO, "Error on rule with AVP '%s'",      			\
					    (_rulearray)[__ar].avp_vendor_plus_name.avp_name);		\
				return EINVAL;					      			\
			} );							      			\
	}									      			\
}

static int dict_gx_init(char * conffile)
{
    struct dict_object * gx;
    struct dict_object * vendor;
    struct dict_object * cmd;
    struct dict_object * avp;
    
    TRACE_ENTRY("%p", conffile);

    CHECK_FCT(fd_dict_search(fd_g_config->cnf_dict, DICT_VENDOR, VENDOR_BY_NAME, "3GPP", &vendor, ENOENT));
    struct dict_application_data app_data = { 16777238, "Gx" };
    CHECK_dict_new(DICT_APPLICATION, &app_data, vendor, &gx);

    /* Supported-Features rules - The AVP is defined in dict_dcca_3gpp */
    {
        struct dict_avp_request req = { 10415, 628, "Supported-Features" };
        CHECK_dict_search(DICT_AVP, AVP_BY_NAME_AND_VENDOR, &req, &avp);
        struct local_rules_definition rules[] = {
            { { .avp_vendor = 0, .avp_name = "Vendor-Id" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Feature-List-ID" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Feature-List" }, RULE_REQUIRED, -1, 1 }
        };
        PARSE_loc_rules(rules, avp);
    }

    /* Credit-Control-Request (CCR) */
    {
        struct dict_cmd_data data = {
            272,
            "Credit-Control-Request",
            CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,
            CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
            { { .avp_vendor = 0, .avp_name = "Session-Id" }, RULE_FIXED_HEAD, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Auth-Application-Id" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Origin-Host" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Origin-Realm" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Destination-Realm" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "CC-Request-Type" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "CC-Request-Number" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Destination-Host" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Origin-State-Id" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Subscription-Id" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Supported-Features" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Network-Request-Support" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Packet-Filter-Information" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Packet-Filter-Operation" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Bearer-Identifier" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Bearer-Operation" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Dynamic-Address-Flag" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Dynamic-Address-Flag-Extension" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "PDN-Connection-ID" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Framed-IP-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Framed-IPv6-Prefix" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "IP-CAN-Type" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-RAT-Type" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "RAT-Type" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Termination-Cause" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "User-Equipment-Info" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "QoS-Information" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "QoS-Negotiation" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "QoS-Upgrade" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Default-EPS-Bearer-QoS" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "AN-GW-Address" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "AN-GW-Status" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-SGSN-MCC-MNC" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-SGSN-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-SGSN-Ipv6-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-GGSN-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-GGSN-Ipv6-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-Selection-Mode" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "RAI" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-User-Location-Info" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "User-Location-Info-Time" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "User-CSG-Information" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "TWAN-User-Location-Info" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-MS-TimeZone" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "RAN-NAS-Release-Cause" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-Charging-Characteristics" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Called-Station-Id" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "PDN-Connection-Charging-ID" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Bearer-Usage" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Online" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Offline" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "TFT-Packet-Filter-Information" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Charging-Rule-Report" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Application-Detection-Information" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Event-Trigger" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Event-Report-Indication" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Access-Network-Charging-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Access-Network-Charging-Identifier-Gx" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "CoA-Information" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Usage-Monitoring-Information" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Routing-Rule-Install" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Routing-Rule-Remove" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "HeNB-Local-IP-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "UE-Local-IP-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "UDP-Source-Port" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "TCP-Source-Port" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Presence-Reporting-Area-Information" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 0, .avp_name = "Proxy-Info" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 0, .avp_name = "Route-Record" }, RULE_OPTIONAL, -1, -1 }
        };
        CHECK_dict_new(DICT_COMMAND, &data, gx, &cmd);
        PARSE_loc_rules(rules, cmd);
    }

    /* Credit-Control-Answer (CCA) */
    {
        struct dict_cmd_data data = {
            272,
            "Credit-Control-Answer",
            CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE,
            CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
            { { .avp_vendor = 0, .avp_name = "Session-Id" }, RULE_FIXED_HEAD, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Auth-Application-Id" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Origin-Host" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Origin-Realm" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Result-Code" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Experimental-Result" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "CC-Request-Type" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "CC-Request-Number" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Supported-Features" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Bearer-Control-Mode" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Event-Trigger" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Event-Report-Indication" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Origin-State-Id" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Charging-Rule-Remove" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Charging-Rule-Install" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Charging-Information" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Online" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Offline" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "QoS-Information" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Revalidation-Time" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Default-EPS-Bearer-QoS" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Bearer-Usage" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-User-Location-Info" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "User-Location-Info-Time" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "User-CSG-Information" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-MS-TimeZone" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Usage-Monitoring-Information" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Tunnel-Information" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Tunnel-Header-Filter" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Tunnel-Header-Length" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "TDF-Destination-Realm" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "TDF-Destination-Host" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "TDF-IP-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "CSG-Information-Reporting" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Packet-Filter-Information" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Packet-Filter-Operation" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Access-Network-Charging-Identifier-Gx" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "CoA-Information" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Routing-Rule-Install" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Routing-Rule-Remove" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "HeNB-Local-IP-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "UE-Local-IP-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "UDP-Source-Port" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "TCP-Source-Port" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Presence-Reporting-Area-Information" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 0, .avp_name = "Failed-AVP" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Proxy-Info" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 0, .avp_name = "Route-Record" }, RULE_OPTIONAL, -1, -1 }
        };
        CHECK_dict_new(DICT_COMMAND, &data, gx, &cmd);
        PARSE_loc_rules(rules, cmd);
    }

    /* Re-Auth-Request (RAR) */
    {
        struct dict_cmd_data data = {
            258,
            "Re-Auth-Request",
            CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,
            CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
            { { .avp_vendor = 0, .avp_name = "Session-Id" }, RULE_FIXED_HEAD, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Auth-Application-Id" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Origin-Host" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Origin-Realm" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Destination-Realm" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Destination-Host" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Re-Auth-Request-Type" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Session-Release-Cause" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Origin-State-Id" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Event-Trigger" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Event-Report-Indication" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Charging-Rule-Remove" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Charging-Rule-Install" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Default-EPS-Bearer-QoS" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "QoS-Information" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "Revalidation-Time" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Usage-Monitoring-Information" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 10415, .avp_name = "PCSCF-Restoration-Indication" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "PRA-Install" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "PRA-Remove" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Proxy-Info" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 0, .avp_name = "Route-Record" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 0, .avp_name = "AVP" }, RULE_OPTIONAL, -1, -1 }
        };
        CHECK_dict_new(DICT_COMMAND, &data, gx, &cmd);
        PARSE_loc_rules(rules, cmd);
    }

    /* Re-Auth-Answer (RAA) */
    {
        struct dict_cmd_data data = {
            258,
            "Re-Auth-Answer",
            CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE,
            CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
            { { .avp_vendor = 0, .avp_name = "Session-Id" }, RULE_FIXED_HEAD, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Origin-Host" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Origin-Realm" }, RULE_REQUIRED, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Result-Code" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Experimental-Result" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Origin-State-Id" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "IP-CAN-Type" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "RAT-Type" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "AN-GW-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "AN-GW-Status" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-SGSN-MCC-MNC" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-SGSN-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-SGSN-Ipv6-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-GGSN-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-GGSN-Ipv6-Address" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "RAI" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-User-Location-Info" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "User-Location-Info-Time" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "NetLoc-Access-Support" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "User-CSG-Information" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "3GPP-MS-TimeZone" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Default-QoS-Information" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 10415, .avp_name = "Charging-Rule-Report" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 0, .avp_name = "Error-Message" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Error-Reporting-Host" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Failed-AVP" }, RULE_OPTIONAL, -1, 1 },
            { { .avp_vendor = 0, .avp_name = "Proxy-Info" }, RULE_OPTIONAL, -1, -1 },
            { { .avp_vendor = 0, .avp_name = "AVP" }, RULE_OPTIONAL, -1, -1 }
        };
        CHECK_dict_new(DICT_COMMAND, &data, gx, &cmd);
        PARSE_loc_rules(rules, cmd);
    }

    LOG_D( "Extension 'Dictionary definitions for Gx (3GPP)' initialized");
    return 0;
}

EXTENSION_ENTRY("dict_gx", dict_gx_init, "dict_dcca_3gpp");