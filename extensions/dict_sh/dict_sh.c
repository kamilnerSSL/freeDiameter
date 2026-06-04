/*********************************************************************************************************
 * Software License Agreement (BSD License)                                                               *
 * Author: Gemini Code Assist                                                                           *
 *                                                                                                        *
 * Copyright (c) 2024, The freeDiameter Project                                                          *
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
 * Dictionary for 3GPP Sh Interface.
 *
 * This dictionary defines the AVPs and Commands for the 3GPP Sh interface,
 * as specified in 3GPP TS 29.329.
 */

#include <freeDiameter/extension.h>

#define CHECK_dict_new( _type, _data, _parent, _ref )	\
	CHECK_FCT(  fd_dict_new( fd_g_config->cnf_dict, (_type), (_data), (_parent), (_ref))  );

#define CHECK_dict_search( _type, _criteria, _what, _result )	\
	CHECK_FCT(  fd_dict_search( fd_g_config->cnf_dict, (_type), (_criteria), (_what), (_result), ENOENT) );

struct local_rules_definition {
	vendor_id_t		vendor;
	char *			avp_name;
	enum rule_position	position;
	int 			min;
	int			max;
};

#define RULE_ORDER( _position ) ((((_position) == RULE_FIXED_HEAD) || ((_position) == RULE_FIXED_TAIL)) ? 1 : 0 )

/* Attention! Uses AVP_BY_NAME_AND_VENDOR so vendor-specific AVPs are found correctly. */
#define PARSE_loc_rules( _rulearray, _parent) {						\
	int __ar;									\
	for (__ar=0; __ar < sizeof(_rulearray) / sizeof((_rulearray)[0]); __ar++) {	\
		struct dict_avp_request __avp = {					\
			.avp_vendor = (_rulearray)[__ar].vendor,			\
			.avp_name   = (_rulearray)[__ar].avp_name,			\
		};									\
		struct dict_rule_data __data = { NULL, 					\
			(_rulearray)[__ar].position,					\
			0, 								\
			(_rulearray)[__ar].min,						\
			(_rulearray)[__ar].max};					\
		__data.rule_order = RULE_ORDER(__data.rule_position);			\
		CHECK_FCT(  fd_dict_search( 						\
			fd_g_config->cnf_dict,						\
			DICT_AVP, 							\
			AVP_BY_NAME_AND_VENDOR, 					\
			&__avp,								\
			&__data.rule_avp, 0 ) );					\
		if ( !__data.rule_avp ) {						\
			LOG_E("AVP Not found: '%s'", __avp.avp_name );			\
			return ENOENT;							\
		}									\
		CHECK_FCT_DO( fd_dict_new( fd_g_config->cnf_dict, DICT_RULE, &__data, _parent, NULL),	\
			{								\
				LOG_E("Error on rule with AVP '%s'", __avp.avp_name );	\
				return EINVAL;						\
			} );								\
	}										\
}

/* 3GPP Sh Application */
int sh_app_init(void)
{
	struct dict_object *sh_app;
	struct dict_application_data app_data = { 16777217, "Diameter Sh Application" };
	CHECK_dict_new(DICT_APPLICATION, &app_data, NULL, &sh_app);

	/* User-Data-Request */
	{
		struct dict_object *cmd;
		struct dict_cmd_data data = { 306, "User-Data-Request", CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE, CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE };
		CHECK_dict_new(DICT_COMMAND, &data, sh_app, &cmd);
		struct local_rules_definition rules[] =
		{
			{ 0,     "Session-Id",				RULE_FIXED_HEAD, 1, 1 },
			{ 0,     "Vendor-Specific-Application-Id",	RULE_REQUIRED,   1, 1 },
			{ 0,     "Auth-Session-State",			RULE_REQUIRED,   1, 1 },
			{ 0,     "Origin-Host",				RULE_REQUIRED,   1, 1 },
			{ 0,     "Origin-Realm",			RULE_REQUIRED,   1, 1 },
			{ 0,     "Destination-Realm",			RULE_REQUIRED,   1, 1 },
			{ 10415, "User-Identity",			RULE_REQUIRED,   1, 1 },
			{ 10415, "Data-Reference",			RULE_REQUIRED,   1, -1 },
			{ 0,     "Destination-Host",			RULE_OPTIONAL,   0, 1 },
			{ 10415, "Supported-Features",			RULE_OPTIONAL,   0, -1 },
			{ 10415, "Server-Name",				RULE_OPTIONAL,   0, 1 },
			{ 10415, "Service-Indication",			RULE_OPTIONAL,   0, -1 },
			{ 10415, "Identity-Set",			RULE_OPTIONAL,   0, -1 },
			{ 10415, "Requested-Domain",			RULE_OPTIONAL,   0, 1 },
			{ 10415, "Current-Location",			RULE_OPTIONAL,   0, 1 },
			{ 10415, "UDR-Flags",				RULE_OPTIONAL,   0, 1 },
			{ 10415, "Requested-Nodes",			RULE_OPTIONAL,   0, 1 },
			{ 10415, "Serving-Node-Indication",		RULE_OPTIONAL,   0, 1 },
			{ 10415, "Pre-paging-Supported",		RULE_OPTIONAL,   0, 1 },
			{ 10415, "Local-Time-Zone-Indication",		RULE_OPTIONAL,   0, 1 },
			{ 0,     "Proxy-Info",				RULE_OPTIONAL,   0, -1 },
			{ 0,     "Route-Record",			RULE_OPTIONAL,   0, -1 }
		};
		PARSE_loc_rules(rules, cmd);
	}

	/* User-Data-Answer */
	{
		struct dict_object *cmd;
		struct dict_cmd_data data = { 306, "User-Data-Answer", CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE, CMD_FLAG_PROXIABLE };
		CHECK_dict_new(DICT_COMMAND, &data, sh_app, &cmd);
		struct local_rules_definition rules[] =
		{
			{ 0,     "Session-Id",				RULE_FIXED_HEAD, 1, 1 },
			{ 0,     "Vendor-Specific-Application-Id",	RULE_REQUIRED,   1, 1 },
			{ 0,     "Auth-Session-State",			RULE_REQUIRED,   1, 1 },
			{ 0,     "Origin-Host",				RULE_REQUIRED,   1, 1 },
			{ 0,     "Origin-Realm",			RULE_REQUIRED,   1, 1 },
			{ 0,     "Result-Code",				RULE_OPTIONAL,   0, 1 },
			{ 0,     "Experimental-Result",			RULE_OPTIONAL,   0, 1 },
			{ 10415, "Supported-Features",			RULE_OPTIONAL,   0, -1 },
			{ 10415, "User-Data-29.329",			RULE_OPTIONAL,   0, 1 },
			{ 0,     "Failed-AVP",				RULE_OPTIONAL,   0, 1 },
			{ 0,     "Proxy-Info",				RULE_OPTIONAL,   0, -1 },
			{ 0,     "Route-Record",			RULE_OPTIONAL,   0, -1 }
		};
		PARSE_loc_rules(rules, cmd);
	}

	return 0;
}

extern int add_sh_avps(void);

static int dict_sh_entry(char * conffile)
{
	CHECK_FCT(add_sh_avps());
	CHECK_FCT(sh_app_init());
	LOG_D("Extension 'Dictionary definitions for 3GPP Sh Interface' initialized");
	return 0;
}

EXTENSION_ENTRY("dict_sh", dict_sh_entry, "dict_dcca_3gpp");
