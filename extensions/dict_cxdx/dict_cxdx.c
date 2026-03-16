/*********************************************************************************************************
 * Software License Agreement (BSD License)                                                               *
 * Author: Norberto R. de Goes Jr. 						 *
 *													 *
 * Copyright (c) 2011, Norberto R. de Goes Jr..                                                      	 *
 *										 *
 * All rights reserved.											 *
 * 													 *
 * Redistribution and use of this software in source and binary forms, with or without modification, are  *
 * permitted provided that the following conditions are met:						 *
 * 													 *
 * * Redistributions of source code must retain the above 						 *
 *   copyright notice, this list of conditions and the 							 *
 *   following disclaimer.										 *
 *    													 *
 * * Redistributions in binary form must reproduce the above 						 *
 *   copyright notice, this list of conditions and the 							 *
 *   following disclaimer in the documentation and/or other						 *
 *   materials provided with the distribution.								 *
 * 													 *
 * * Neither the name of the Teraoka Laboratory nor the 							 *
 *   names of its contributors may be used to endorse or 						 *
 *   promote products derived from this software without 						 *
 *   specific prior written permission of Teraoka Laboratory 						 *
 *   													 *
 * 													 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED *
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR *
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 	 *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 	 *
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR *
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF   *
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.								 *
 *********************************************************************************************************/


/*********************************************************************************************************

 === CpqD/DRC  -  Projeto ADRIMS  -  Mar/2011 ===
 === Dicionario Dx/Cx ===
 Baseado no "dict_sip" do FreeDiameter (www.freediameter.net) 
                                                                                 Norberto R Goes Jr
*********************************************************************************************************/


#include <freeDiameter/extension.h>



/* The content of this file follows the same structure as dict_base_proto.c */

#define CHECK_dict_new( _type, _data, _parent, _ref )			\
  CHECK_FCT(  fd_dict_new( fd_g_config->cnf_dict, (_type), (_data), (_parent), (_ref))  );

#define CHECK_dict_search( _type, _criteria, _what, _result )		\
  CHECK_FCT(  fd_dict_search( fd_g_config->cnf_dict, (_type), (_criteria), (_what), (_result), ENOENT) );


struct local_rules_definition 
{
  char 			*avp_name;
  enum rule_position	 position;
  int 			 min;
  int			 max;
};

/*==================================================================*/

#define RULE_ORDER( _position ) ((((_position) == RULE_FIXED_HEAD) || ((_position) == RULE_FIXED_TAIL)) ? 1 : 0 )

/*==================================================================*/

#define PARSE_loc_rules( _rulearray, _parent, _avp_search_flag) {	\
    int __ar;								\
    for (__ar=0; __ar < sizeof(_rulearray) / sizeof((_rulearray)[0]); __ar++) {	\
      struct dict_rule_data __data = { NULL,				\
				       (_rulearray)[__ar].position,	\
				       0,				\
				       (_rulearray)[__ar].min,		\
				       (_rulearray)[__ar].max};		\
      __data.rule_order = RULE_ORDER(__data.rule_position);		\
                                                                        \
      CHECK_FCT(  fd_dict_search(					\
				 fd_g_config->cnf_dict,			\
				 DICT_AVP,				\
				 _avp_search_flag,			\
				 (_rulearray)[__ar].avp_name,		\
				 &__data.rule_avp, 0 ) );		\
      if ( !__data.rule_avp ) {						\
	TRACE_DEBUG(INFO, "AVP Not found: '%s'", (_rulearray)[__ar].avp_name );	\
	return ENOENT;							\
      }									\
                                                                        \
      CHECK_FCT_DO( fd_dict_new( fd_g_config->cnf_dict, DICT_RULE, &__data, _parent, NULL), \
		    {							\
		      TRACE_DEBUG(INFO, "Error on rule with AVP '%s'",	\
				  (_rulearray)[__ar].avp_name );	\
		      return EINVAL;					\
		    } );						\
    }									\
  }

#define enumval_def_u32( _val_, _str_ )		\
  { _str_, 		{ .u32 = _val_ }}

#define enumval_def_os( _len_, _val_, _str_ )				\
  { _str_, 		{ .os = { .data = (unsigned char *)_val_, .len = _len_ }}}


/*==================================================================*/
/*==================================================================*/
/*==================================================================*/
/*==================================================================*/

int cxdx_dict_init(char * conffile)
{

#define VENDOR_3GPP_Id  10415


  struct dict_object * vendor_dict;
  {
    struct dict_vendor_data vendor_data = { VENDOR_3GPP_Id, "3GPP" };
    CHECK_dict_new (DICT_VENDOR, &vendor_data, NULL, &vendor_dict);
  }


  struct dict_object * cxdx_dict;
  {
    struct dict_application_data data  = { 16777216 /* NRGJ */, "Diameter CxDx Application"	};
    CHECK_dict_new (DICT_APPLICATION, &data, vendor_dict, &cxdx_dict);
  }


  /* ##### AVP section #################################### */
  {
    struct dict_object * UTF8String_type;
    struct dict_object * DiameterURI_type;

    CHECK_dict_search( DICT_TYPE, TYPE_BY_NAME, "UTF8String",  &UTF8String_type);
    CHECK_dict_search( DICT_TYPE, TYPE_BY_NAME, "DiameterURI", &DiameterURI_type);

    /* Digest AVPs:  */

    /* Visited-Network-Identifier */
    {
      struct dict_avp_data data = 
	{ 
	  600, 					/* Code */
	  VENDOR_3GPP_Id,  			/* Vendor */
	  "Visited-Network-Identifier",         /* Name */
	  AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
	  AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flag values */
	  AVP_TYPE_OCTETSTRING 			/* base type of data */
	};
      CHECK_dict_new( DICT_AVP, &data , UTF8String_type, NULL);
    }

    /* Public-Identity */
    {
      struct dict_avp_data data = 
	{ 
	  601, 					/* Code */
	  VENDOR_3GPP_Id,			/* Vendor */
	  "Public-Identity", 		        /* Name */
	  AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
	  AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR,	/* Fixed flag values */
	  AVP_TYPE_OCTETSTRING 			/* base type of data */
	};
      CHECK_dict_new( DICT_AVP, &data , UTF8String_type, NULL);
    }


    /* Server-Name */
    {
      struct dict_avp_data data = 
	{ 
	  602, 					/* Code */
	  VENDOR_3GPP_Id,			/* Vendor */
	  "Server-Name", 		        /* Name */
	  AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
	  AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR,	/* Fixed flag values */
	  AVP_TYPE_OCTETSTRING 			/* base type of data */
	};
      CHECK_dict_new( DICT_AVP, &data , DiameterURI_type/*UTF8String_type*/, NULL);
    }


    /* Mandatory-Capability */
    {
      struct dict_avp_data data =
	{
	  604, 					/* Code */
	  VENDOR_3GPP_Id,			/* Vendor */
	  "Mandatory-Capability",	        /* Name */
	  AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
	  AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR,	/* Fixed flag values */
	  AVP_TYPE_UNSIGNED32 			/* base type of data */
	};
      CHECK_dict_new( DICT_AVP, &data , NULL, NULL);
    }


    /* Optional-Capability */
    {
      struct dict_avp_data data =
	{
	  605, 					/* Code */
	  VENDOR_3GPP_Id,			/* Vendor */
	  "Optional-Capability",	        /* Name */
	  AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
	  AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR,	/* Fixed flag values */
	  AVP_TYPE_UNSIGNED32 			/* base type of data */
	};
      CHECK_dict_new( DICT_AVP, &data , NULL, NULL);
    }


    /* Feature-List-ID (629) and Feature-List (630) are defined by dict_dcca_3gpp */
    /* User-Authorization-Type (623) is also defined by dict_dcca_3gpp */
    /* SIP AVPs 606-626 (User-Data, SIP-Auth-Data-Item, Server-Assignment-Type, */
    /* Deregistration-Reason, Charging-Information, etc.) are in dict_dcca_3gpp  */
    /* UAR-Flags (637), LIA-Flags (653), Associated-Identities (632),            */
    /* Wildcarded-Public-Identity (634), Wildcarded-IMPU (636) are in            */
    /* dict_dcca_3gpp as well.                                                    */

    /* Server-Capabilities */
    {
      struct dict_object * avp;
      struct dict_avp_data data = 
	{ 
	  603, 					/* Code */
	  VENDOR_3GPP_Id,			/* Vendor */
	  "Server-Capabilities", 		/* Name */
	  AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR, /* Fixed flags */
	  AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR,	/* Fixed flag values */
	  AVP_TYPE_GROUPED 			/* base type of data */
	};

      struct local_rules_definition rules[] =
	{
	  {  "Mandatory-Capability", RULE_OPTIONAL, -1, -1 },
	  {  "Optional-Capability",  RULE_OPTIONAL, -1, -1 },
	  {  "Server-Name",          RULE_OPTIONAL, -1, -1 }
	};

      CHECK_dict_new (DICT_AVP, &data , NULL, &avp);
      PARSE_loc_rules(rules, avp, AVP_BY_NAME_ALL_VENDORS );
    }



    /* User-Authorization-Type (623) defined by dict_dcca_3gpp */


    /* Supported-Features (628) is defined by dict_dcca_3gpp; add Cx/Dx rules here */
    {
      struct dict_object * avp;
      struct local_rules_definition rules[] =
	{
	  {  "Vendor-Id", 	 RULE_REQUIRED, -1, 1 },
	  {  "Feature-List-ID",  RULE_REQUIRED, -1, 1 },
	  {  "Feature-List", 	 RULE_REQUIRED, -1, 1 }
	};

      CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME_AND_VENDOR,
                 & (struct dict_avp_request){ .avp_vendor = VENDOR_3GPP_Id, .avp_name = "Supported-Features" },
                 &avp, ENOENT) );
      PARSE_loc_rules(rules, avp, AVP_BY_NAME_ALL_VENDORS );
    }
  



  } /* end AVP section */



  /* ### Command section ############################# */
  {
    /* User-Authorization-Request (UAR) Command */
    {
      /*
	The User-Authorization-Request (UAR) is indicated by the Command-Code
	set to 283 and the Command Flags' 'R' bit set.  The Diameter client
	in a SIP server sends this command to the Diameter server to request
	authorization for the SIP User Agent to route a SIP REGISTER request.
	Because the SIP REGISTER request implicitly carries a permission to
	bind an AOR to a contact address, the Diameter client uses the
	Diameter UAR as a first authorization request towards the Diameter
	server to authorize the registration.  For instance, the Diameter
	server can verify that the AOR is a legitimate user of the realm.

	The Diameter client in the SIP server requests authorization for one
	of the possible values defined in the SIP-User-Authorization-Type AVP
	(Section 9.10).

	The user name used for authentication of the user is conveyed in a
	User-Name AVP (defined in the Diameter base protocol, RFC 3588
	[RFC3588]).  The location of the authentication user name in the SIP
	REGISTER request varies depending on the authentication mechanism.
	When the authentication mechanism is HTTP Digest as defined in RFC
	2617 [RFC2617], the authentication user name is found in the
	"username" directive of the SIP Authorization header field value.
	This Diameter SIP application only provides support for HTTP Digest
	authentication in SIP; other authentication mechanisms are not
	currently supported.

	The SIP or SIPS URI to be registered is conveyed in the SIP-AOR AVP
	(Section 9.8).  Typically this SIP or SIPS URI is found in the To
	header field value of the SIP REGISTER request that triggered the
	Diameter UAR message.

	The SIP-Visited-Network-Id AVP indicates the network that is
	providing SIP services (e.g., SIP proxy functionality or any other
	kind of services) to the SIP User Agent.

	The Message Format of the UAR command is as follows:


	< UAR> ::=< Diameter Header: 300, REQ, PXY, 16777216 >
	< Session-Id >
	{ Vendor-Specific-Application-Id }
	{ Auth-Session-State }
	{ Origin-Host }
	{ Origin-Realm }
	[ Destination-Host ]
	{ Destination-Realm }
	{ User-Name }
	*[ Supported-Features ]
	{ Public-Identity }
	{ Visited-Network-Identifier }
	[ User-Authorization-Type ]
	[ UAR-Flags ]
	*[ AVP ]
	*[ Proxy-Info ]
	*[ Route-Record ]
	*/

      struct dict_object   * cmd;
      struct dict_cmd_data   data = 
	{ 
	  300,                   	/* Code */
	  "User-Authorization-Request", /* Name */
	  CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR, /* Fixed flags */
	  CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE  /* Fixed flag values */
	};

      struct local_rules_definition rules[] = 
	{
	  {  "Session-Id", 			RULE_FIXED_HEAD, -1,  1 },
	  {  "Vendor-Specific-Application-Id",	RULE_REQUIRED,   -1,  1 },
	  {  "Auth-Session-State", 		RULE_REQUIRED,   -1,  1 },
	  {  "Origin-Host", 			RULE_REQUIRED,   -1,  1 },
	  {  "Origin-Realm", 			RULE_REQUIRED,   -1,  1 },
	  //	  {  "Destination-Host", 		RULE_OPTIONAL,   -1,  1 },
	  {  "Destination-Realm", 		RULE_REQUIRED,   -1,  1 },
	  {  "User-Name", 			RULE_REQUIRED,   -1,  1 },
	  //	  {  "Supported-Features", 		RULE_OPTIONAL,   -1, -1 },
	  {  "Public-Identity", 		RULE_REQUIRED,   -1,  1 },
	  {  "Visited-Network-Identifier",      RULE_REQUIRED,   -1,  1 },
	  //	  {  "UAR-Flags",                       RULE_OPTIONAL,   -1,  1 },
	  //	  {  "User-Authorization-Type", 	RULE_OPTIONAL,   -1,  1 },
	  //	  {  "Proxy-Info", 			RULE_OPTIONAL,   -1, -1 },
	  //	  {  "Route-Record", 			RULE_OPTIONAL,   -1, -1 }
	};
			
      CHECK_dict_new( DICT_COMMAND, &data , cxdx_dict, &cmd);
      PARSE_loc_rules( rules, cmd, AVP_BY_NAME_ALL_VENDORS);
    }

    /* User-Authorization-Answer (UAA) Command */
    {
      /*
	The User-Authorization-Answer (UAA) is indicated by the Command-Code
	set to 283 and the Command Flags' 'R' bit cleared.  The Diameter
	server sends this command in response to a previously received
	Diameter User-Authorization-Request (UAR) command.  The Diameter
	server indicates the result of the requested registration
	authorization.  Additionally, the Diameter server may indicate a
	collection of SIP capabilities that assists the Diameter client to
	select a SIP proxy to the AOR under registration.


	In addition to the values already defined in RFC 3588 [RFC3588], the
	Result-Code AVP may contain one of the values defined in
	Section 10.1.

	Whenever the Diameter server fails to process the Diameter UAR
	message, it MUST stop processing and return the relevant error in the
	Diameter UAA message.  When there is success in the process, the
	Diameter server MUST set the code to DIAMETER_SUCCESS in the Diameter
	UAA message.

	If the Diameter server requires a User-Name AVP value to process the
	Diameter UAR request, but the Diameter UAR message did not contain a
	User-Name AVP value, the Diameter server MUST set the Result-Code AVP
	value to DIAMETER_USER_NAME_REQUIRED (see Section 10.1.2) and return
	it in a Diameter UAA message.  Upon reception of this Diameter UAA
	message with the Result-Code AVP value set to
	DIAMETER_USER_NAME_REQUIRED, the SIP server typically requests
	authentication by sending a SIP 401 (Unauthorized) or SIP 407 (Proxy
	Authentication Required) response back to the originator.

	When the authorization procedure succeeds, the Diameter server
	constructs a User-Authorization-Answer (UAA) message that MUST
	include (1) the address of the SIP server already assigned to the
	user name, (2) the capabilities needed by the SIP server (Diameter
	client) to select another SIP server for the user, or (3) a
	combination of the previous two options.

	If the Diameter server is already aware of a SIP server allocated to
	the user, the Diameter UAA message contains the address of that SIP
	server.

	The Diameter UAA message contains the capabilities required by a SIP
	server to trigger and execute services.  It is required that these
	capabilities are present in the Diameter UAA message due to the
	possibility that the Diameter client (in the SIP server) allocates a
	different SIP server to trigger and execute services for that
	particular user.

	If a User-Name AVP is present in the Diameter UAR message, then the
	Diameter server MUST verify the existence of the user in the realm,
	i.e., the User-Name AVP value is a valid user within that realm.  If
	the Diameter server does not recognize the user name received in the
	User-Name AVP, the Diameter server MUST build a Diameter User-
	Authorization-Answer (UAA) message and MUST set the Result-Code AVP
	to DIAMETER_ERROR_USER_UNKNOWN.


	If a User-Name AVP is present in the Diameter UAR message, then the
	Diameter server MUST authorize that User-Name AVP value is able to
	register the SIP or SIPS URI included in the SIP-AOR AVP.  If this
	authorization fails, the Diameter server must set the Result-Code AVP
	to DIAMETER_ERROR_IDENTITIES_DONT_MATCH and send it in a Diameter
	User-Authorization-Answer (UAA) message.

	Note: Correlation between User-Name and SIP-AOR AVP values is
	required in order to avoid registration of a SIP-AOR allocated to
	another user.

	If there is a SIP-Visited-Network-Id AVP in the Diameter UAR message,
	and the SIP-User-Authorization-Type AVP value received in the
	Diameter UAR message is set to REGISTRATION or REGISTRATION&
	CAPABILITIES, then the Diameter server SHOULD verify whether the user
	is allowed to roam into the network specified in the
	SIP-Visited-Network-Id AVP in the Diameter UAR message.  If the user
	is not allowed to roam into that network, the Diameter AAA server
	MUST set the Result-Code AVP value in the Diameter UAA message to
	DIAMETER_ERROR_ROAMING_NOT_ALLOWED.

	If the SIP-User-Authorization-Type AVP value received in the Diameter
	UAR message is set to REGISTRATION or REGISTRATION&CAPABILITIES, then
	the Diameter server SHOULD verify whether the SIP-AOR AVP value is
	authorized to register in the Home Realm.  Where the SIP AOR is not
	authorized to register in the Home Realm, the Diameter server MUST
	set the Result-Code AVP to DIAMETER_AUTHORIZATION_REJECTED and send
	it in a Diameter UAA message.

	When the SIP-User-Authorization-Type AVP is not present in the
	Diameter UAR message, or when it is present and its value is set to
	REGISTRATION, then:

	o  If the Diameter server is not aware of any previous registration
	of the user name (including registrations of other SIP AORs
	allocated to the same user name), then the Diameter server does
	not know of any SIP server allocated to the user.  In this case,
	the Diameter server MUST set the Result-Code AVP value to
	DIAMETER_FIRST_REGISTRATION in the Diameter UAA message, and the
	Diameter server SHOULD include the required SIP server
	capabilities in the SIP-Server-Capabilities AVP value in the
	Diameter UAA message.  The SIP-Server-Capabilities AVP assists the
	Diameter client (SIP server) to select an appropriate SIP server
	for the user, according to the required capabilities.

	o  In some cases, the Diameter server is aware of a previously
	assigned SIP server for the same or different SIP AORs allocated
	to the same user name.  In these cases, re-assignment of a new SIP
	server may or may not be needed, depending on the capabilities of
	the SIP server.  The Diameter server MUST always include the
	allocated SIP server URI in the SIP-Server-URI AVP of the UAA
	message.  If the Diameter server does not return the SIP
	capabilities, the Diameter server MUST set the Result-Code AVP in
	the Diameter UAA message to DIAMETER_SUBSEQUENT_REGISTRATION.
	Otherwise (i.e., if the Diameter server includes a
	SIP-Server-Capabilities AVP), then the Diameter server MUST set
	the Result-Code AVP in the Diameter UAA message to
	DIAMETER_SERVER_SELECTION.  Then the Diameter client determines,
	based on the received information, whether it needs to select a
	new SIP server.

	When the SIP-User-Authorization-Type AVP value received in the
	Diameter UAR message is set to REGISTRATION&CAPABILITIES, then
	Diameter Server MUST return the list of capabilities in the
	SIP-Server-Capabilities AVP value of the Diameter UAA message, it
	MUST set the Result-Code to DIAMETER_SUCCESS, and it MUST NOT return
	a SIP-Server-URI AVP.  The SIP-Server-Capabilities AVP enables the
	SIP server (Diameter client) to select another appropriate SIP server
	for invoking and executing services for the user, depending on the
	required capabilities.  The Diameter server MAY leave the list of
	capabilities empty to indicate that any SIP server can be selected.

	When the SIP-User-Authorization-Type AVP value received in the
	Diameter UAR message is set to DEREGISTRATION, then:

	o  If the Diameter server is aware of a SIP server assigned to the
	SIP AOR under deregistration, the Diameter server MUST set the
	Result-Code AVP to DIAMETER_SUCCESS and MUST set the
	SIP-Server-URI AVP value to the known SIP server, and return them
	in the Diameter UAA message.

	o  If the Diameter server is not aware of a SIP server assigned to
	the SIP AOR under deregistration, then the Diameter server MUST
	set the Result-Code AVP in the Diameter UAA message to
	DIAMETER_ERROR_IDENTITY_NOT_REGISTERED.

	The Message Format of the UAA command is as follows:

	<UAA> ::= < Diameter Header: 283, PXY >
	< Session-Id >
	{ Auth-Application-Id }
	{ Auth-Session-State }
	{ Result-Code }
	{ Origin-Host }
	{ Origin-Realm }
	[ SIP-Server-URI ]
	[ SIP-Server-Capabilities ]
	[ Authorization-Lifetime ]
	[ Auth-Grace-Period ]
	[ Redirect-Host ]
	[ Redirect-Host-Usage ]
	[ Redirect-Max-Cache-Time ]
	* [ Proxy-Info ]
	* [ Route-Record ]
	* [ AVP ]


	*/
      struct dict_object * cmd;
      struct dict_cmd_data data = 
	{ 
	  300, 					/* Code */
	  "User-Authorization-Answer", 		/* Name */
	  CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR, /* Fixed flags */
	  CMD_FLAG_PROXIABLE 					  /* Fixed flag values */
	};

      struct local_rules_definition rules[] =
	{
	  {  "Session-Id", 			RULE_FIXED_HEAD, -1,  1 },
	  {  "Vendor-Specific-Application-Id",	RULE_REQUIRED,   -1,  1 },
	  {  "Auth-Session-State", 		RULE_REQUIRED,   -1,  1 },
	  {  "Result-Code", 			RULE_REQUIRED,   -1,  1 },
	  {  "Origin-Host", 			RULE_REQUIRED,   -1,  1 },
	  {  "Origin-Realm", 			RULE_REQUIRED,   -1,  1 },
	  {  "Proxy-Info", 			RULE_OPTIONAL,   -1, -1 },
	  {  "Route-Record", 			RULE_OPTIONAL,   -1, -1 }
	};

      CHECK_dict_new( DICT_COMMAND, &data , cxdx_dict, &cmd);
      PARSE_loc_rules( rules, cmd, AVP_BY_NAME_ALL_VENDORS );
    }





/* Commands defined in 3GPP TS 29.229 (Cx/Dx interface) */

    /*
     * <MAR> ::= < Diameter Header: 303, REQ, PXY, 16777216 >
       *           < Session-Id >
       *           { Vendor-Specific-Application-Id }
       *           { Auth-Session-State }
       *           { Origin-Host }
       *           { Origin-Realm }
       *           { Destination-Realm }
       *           { User-Name }
       *           { Public-Identity }
       *           { SIP-Auth-Data-Item }
       *           { SIP-Number-Auth-Items }
       *           { Server-Name }
       *           [ Destination-Host ]
       *         * [ Supported-Features ]
       *         * [ Proxy-Info ]
       *         * [ Route-Record ]
       */

      struct dict_object * cmd;

      /* MAR - Multimedia-Auth-Request (303, REQ) */
      {
        struct dict_cmd_data data = {
          303,
          "Multimedia-Auth-Request",
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
           { "Session-Id",                    RULE_FIXED_HEAD, -1,  1 }
          ,{ "Vendor-Specific-Application-Id",RULE_REQUIRED,   -1,  1 }
          ,{ "Auth-Session-State",             RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Host",                    RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Realm",                   RULE_REQUIRED,   -1,  1 }
          ,{ "Destination-Realm",              RULE_REQUIRED,   -1,  1 }
          ,{ "User-Name",                      RULE_REQUIRED,   -1,  1 }
          ,{ "Public-Identity",                RULE_REQUIRED,   -1,  1 }
          ,{ "SIP-Auth-Data-Item",             RULE_REQUIRED,   -1,  1 }
          ,{ "SIP-Number-Auth-Items",          RULE_REQUIRED,   -1,  1 }
          ,{ "Server-Name",                    RULE_REQUIRED,   -1,  1 }
          ,{ "Destination-Host",               RULE_OPTIONAL,   -1,  1 }
          ,{ "Supported-Features",             RULE_OPTIONAL,   -1, -1 }
          ,{ "Proxy-Info",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Route-Record",                   RULE_OPTIONAL,   -1, -1 }
        };
        CHECK_dict_new( DICT_COMMAND, &data, cxdx_dict, &cmd );
        PARSE_loc_rules( rules, cmd, AVP_BY_NAME_ALL_VENDORS );
      }

      /* MAA - Multimedia-Auth-Answer (303, ANS) */
      {
        struct dict_cmd_data data = {
          303,
          "Multimedia-Auth-Answer",
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,
          CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
           { "Session-Id",                    RULE_FIXED_HEAD, -1,  1 }
          ,{ "Vendor-Specific-Application-Id",RULE_REQUIRED,   -1,  1 }
          ,{ "Result-Code",                   RULE_OPTIONAL,   -1,  1 }
          ,{ "Experimental-Result",           RULE_OPTIONAL,   -1,  1 }
          ,{ "Auth-Session-State",             RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Host",                    RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Realm",                   RULE_REQUIRED,   -1,  1 }
          ,{ "User-Name",                      RULE_OPTIONAL,   -1,  1 }
          ,{ "Public-Identity",                RULE_OPTIONAL,   -1,  1 }
          ,{ "SIP-Auth-Data-Item",             RULE_OPTIONAL,   -1, -1 }
          ,{ "SIP-Number-Auth-Items",          RULE_OPTIONAL,   -1,  1 }
          ,{ "Supported-Features",             RULE_OPTIONAL,   -1, -1 }
          ,{ "Failed-AVP",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Proxy-Info",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Route-Record",                   RULE_OPTIONAL,   -1, -1 }
        };
        CHECK_dict_new( DICT_COMMAND, &data, cxdx_dict, &cmd );
        PARSE_loc_rules( rules, cmd, AVP_BY_NAME_ALL_VENDORS );
      }

      /* SAR - Server-Assignment-Request (301, REQ) */
      {
        struct dict_cmd_data data = {
          301,
          "Server-Assignment-Request",
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
           { "Session-Id",                    RULE_FIXED_HEAD, -1,  1 }
          ,{ "Vendor-Specific-Application-Id",RULE_REQUIRED,   -1,  1 }
          ,{ "Auth-Session-State",             RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Host",                    RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Realm",                   RULE_REQUIRED,   -1,  1 }
          ,{ "Destination-Realm",              RULE_REQUIRED,   -1,  1 }
          ,{ "Server-Name",                    RULE_REQUIRED,   -1,  1 }
          ,{ "Server-Assignment-Type",         RULE_REQUIRED,   -1,  1 }
          ,{ "User-Data-Already-Available",    RULE_REQUIRED,   -1,  1 }
          ,{ "Destination-Host",               RULE_OPTIONAL,   -1,  1 }
          ,{ "User-Name",                      RULE_OPTIONAL,   -1,  1 }
          ,{ "Public-Identity",                RULE_OPTIONAL,   -1, -1 }
          ,{ "Wildcarded-Public-Identity",     RULE_OPTIONAL,   -1,  1 }
          ,{ "Wildcarded-IMPU",                RULE_OPTIONAL,   -1,  1 }
          ,{ "Supported-Features",             RULE_OPTIONAL,   -1, -1 }
          ,{ "Proxy-Info",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Route-Record",                   RULE_OPTIONAL,   -1, -1 }
        };
        CHECK_dict_new( DICT_COMMAND, &data, cxdx_dict, &cmd );
        PARSE_loc_rules( rules, cmd, AVP_BY_NAME_ALL_VENDORS );
      }

      /* SAA - Server-Assignment-Answer (301, ANS) */
      {
        struct dict_cmd_data data = {
          301,
          "Server-Assignment-Answer",
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,
          CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
           { "Session-Id",                    RULE_FIXED_HEAD, -1,  1 }
          ,{ "Vendor-Specific-Application-Id",RULE_REQUIRED,   -1,  1 }
          ,{ "Result-Code",                   RULE_OPTIONAL,   -1,  1 }
          ,{ "Experimental-Result",           RULE_OPTIONAL,   -1,  1 }
          ,{ "Auth-Session-State",             RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Host",                    RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Realm",                   RULE_REQUIRED,   -1,  1 }
          ,{ "User-Name",                      RULE_OPTIONAL,   -1,  1 }
          ,{ "User-Data",                      RULE_OPTIONAL,   -1,  1 }
          ,{ "Charging-Information",           RULE_OPTIONAL,   -1,  1 }
          ,{ "Associated-Identities",          RULE_OPTIONAL,   -1,  1 }
          ,{ "Loose-Route-Indication",         RULE_OPTIONAL,   -1,  1 }
          ,{ "SCSCF-Restoration-Info",         RULE_OPTIONAL,   -1, -1 }
          ,{ "Associated-Registered-Identities", RULE_OPTIONAL, -1,  1 }
          ,{ "Server-Name",                    RULE_OPTIONAL,   -1,  1 }
          ,{ "Wildcarded-Public-Identity",     RULE_OPTIONAL,   -1,  1 }
          ,{ "Wildcarded-IMPU",                RULE_OPTIONAL,   -1,  1 }
          ,{ "Supported-Features",             RULE_OPTIONAL,   -1, -1 }
          ,{ "Failed-AVP",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Proxy-Info",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Route-Record",                   RULE_OPTIONAL,   -1, -1 }
        };
        CHECK_dict_new( DICT_COMMAND, &data, cxdx_dict, &cmd );
        PARSE_loc_rules( rules, cmd, AVP_BY_NAME_ALL_VENDORS );
      }

      /* LIR - Location-Info-Request (302, REQ) */
      {
        struct dict_cmd_data data = {
          302,
          "Location-Info-Request",
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
           { "Session-Id",                    RULE_FIXED_HEAD, -1,  1 }
          ,{ "Vendor-Specific-Application-Id",RULE_REQUIRED,   -1,  1 }
          ,{ "Auth-Session-State",             RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Host",                    RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Realm",                   RULE_REQUIRED,   -1,  1 }
          ,{ "Destination-Realm",              RULE_REQUIRED,   -1,  1 }
          ,{ "Public-Identity",                RULE_REQUIRED,   -1,  1 }
          ,{ "Destination-Host",               RULE_OPTIONAL,   -1,  1 }
          ,{ "User-Authorization-Type",        RULE_OPTIONAL,   -1,  1 }
          ,{ "Supported-Features",             RULE_OPTIONAL,   -1, -1 }
          ,{ "Proxy-Info",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Route-Record",                   RULE_OPTIONAL,   -1, -1 }
        };
        CHECK_dict_new( DICT_COMMAND, &data, cxdx_dict, &cmd );
        PARSE_loc_rules( rules, cmd, AVP_BY_NAME_ALL_VENDORS );
      }

      /* LIA - Location-Info-Answer (302, ANS) */
      {
        struct dict_cmd_data data = {
          302,
          "Location-Info-Answer",
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,
          CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
           { "Session-Id",                    RULE_FIXED_HEAD, -1,  1 }
          ,{ "Vendor-Specific-Application-Id",RULE_REQUIRED,   -1,  1 }
          ,{ "Result-Code",                   RULE_OPTIONAL,   -1,  1 }
          ,{ "Experimental-Result",           RULE_OPTIONAL,   -1,  1 }
          ,{ "Auth-Session-State",             RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Host",                    RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Realm",                   RULE_REQUIRED,   -1,  1 }
          ,{ "Server-Name",                    RULE_OPTIONAL,   -1,  1 }
          ,{ "Server-Capabilities",            RULE_OPTIONAL,   -1,  1 }
          ,{ "Wildcarded-Public-Identity",     RULE_OPTIONAL,   -1,  1 }
          ,{ "Wildcarded-IMPU",                RULE_OPTIONAL,   -1,  1 }
          ,{ "LIA-Flags",                      RULE_OPTIONAL,   -1,  1 }
          ,{ "Supported-Features",             RULE_OPTIONAL,   -1, -1 }
          ,{ "Failed-AVP",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Proxy-Info",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Route-Record",                   RULE_OPTIONAL,   -1, -1 }
        };
        CHECK_dict_new( DICT_COMMAND, &data, cxdx_dict, &cmd );
        PARSE_loc_rules( rules, cmd, AVP_BY_NAME_ALL_VENDORS );
      }

      /* RTR - Registration-Termination-Request (304, REQ) */
      {
        struct dict_cmd_data data = {
          304,
          "Registration-Termination-Request",
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
           { "Session-Id",                    RULE_FIXED_HEAD, -1,  1 }
          ,{ "Vendor-Specific-Application-Id",RULE_REQUIRED,   -1,  1 }
          ,{ "Auth-Session-State",             RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Host",                    RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Realm",                   RULE_REQUIRED,   -1,  1 }
          ,{ "Destination-Realm",              RULE_REQUIRED,   -1,  1 }
          ,{ "Destination-Host",               RULE_REQUIRED,   -1,  1 }
          ,{ "User-Name",                      RULE_REQUIRED,   -1,  1 }
          ,{ "Deregistration-Reason",          RULE_REQUIRED,   -1,  1 }
          ,{ "Associated-Identities",          RULE_OPTIONAL,   -1,  1 }
          ,{ "Public-Identity",                RULE_OPTIONAL,   -1, -1 }
          ,{ "Supported-Features",             RULE_OPTIONAL,   -1, -1 }
          ,{ "Proxy-Info",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Route-Record",                   RULE_OPTIONAL,   -1, -1 }
        };
        CHECK_dict_new( DICT_COMMAND, &data, cxdx_dict, &cmd );
        PARSE_loc_rules( rules, cmd, AVP_BY_NAME_ALL_VENDORS );
      }

      /* RTA - Registration-Termination-Answer (304, ANS) */
      {
        struct dict_cmd_data data = {
          304,
          "Registration-Termination-Answer",
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,
          CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
           { "Session-Id",                    RULE_FIXED_HEAD, -1,  1 }
          ,{ "Vendor-Specific-Application-Id",RULE_REQUIRED,   -1,  1 }
          ,{ "Result-Code",                   RULE_OPTIONAL,   -1,  1 }
          ,{ "Experimental-Result",           RULE_OPTIONAL,   -1,  1 }
          ,{ "Auth-Session-State",             RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Host",                    RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Realm",                   RULE_REQUIRED,   -1,  1 }
          ,{ "Associated-Identities",          RULE_OPTIONAL,   -1,  1 }
          ,{ "Supported-Features",             RULE_OPTIONAL,   -1, -1 }
          ,{ "Failed-AVP",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Proxy-Info",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Route-Record",                   RULE_OPTIONAL,   -1, -1 }
        };
        CHECK_dict_new( DICT_COMMAND, &data, cxdx_dict, &cmd );
        PARSE_loc_rules( rules, cmd, AVP_BY_NAME_ALL_VENDORS );
      }

      /* PPR - Push-Profile-Request (305, REQ) */
      {
        struct dict_cmd_data data = {
          305,
          "Push-Profile-Request",
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
           { "Session-Id",                    RULE_FIXED_HEAD, -1,  1 }
          ,{ "Vendor-Specific-Application-Id",RULE_REQUIRED,   -1,  1 }
          ,{ "Auth-Session-State",             RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Host",                    RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Realm",                   RULE_REQUIRED,   -1,  1 }
          ,{ "Destination-Realm",              RULE_REQUIRED,   -1,  1 }
          ,{ "Destination-Host",               RULE_REQUIRED,   -1,  1 }
          ,{ "User-Name",                      RULE_REQUIRED,   -1,  1 }
          ,{ "User-Data",                      RULE_OPTIONAL,   -1,  1 }
          ,{ "Charging-Information",           RULE_OPTIONAL,   -1,  1 }
          ,{ "SIP-Auth-Data-Item",             RULE_OPTIONAL,   -1,  1 }
          ,{ "Supported-Features",             RULE_OPTIONAL,   -1, -1 }
          ,{ "Proxy-Info",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Route-Record",                   RULE_OPTIONAL,   -1, -1 }
        };
        CHECK_dict_new( DICT_COMMAND, &data, cxdx_dict, &cmd );
        PARSE_loc_rules( rules, cmd, AVP_BY_NAME_ALL_VENDORS );
      }

      /* PPA - Push-Profile-Answer (305, ANS) */
      {
        struct dict_cmd_data data = {
          305,
          "Push-Profile-Answer",
          CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR,
          CMD_FLAG_PROXIABLE
        };
        struct local_rules_definition rules[] = {
           { "Session-Id",                    RULE_FIXED_HEAD, -1,  1 }
          ,{ "Vendor-Specific-Application-Id",RULE_REQUIRED,   -1,  1 }
          ,{ "Result-Code",                   RULE_OPTIONAL,   -1,  1 }
          ,{ "Experimental-Result",           RULE_OPTIONAL,   -1,  1 }
          ,{ "Auth-Session-State",             RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Host",                    RULE_REQUIRED,   -1,  1 }
          ,{ "Origin-Realm",                   RULE_REQUIRED,   -1,  1 }
          ,{ "Supported-Features",             RULE_OPTIONAL,   -1, -1 }
          ,{ "Failed-AVP",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Proxy-Info",                     RULE_OPTIONAL,   -1, -1 }
          ,{ "Route-Record",                   RULE_OPTIONAL,   -1, -1 }
        };
        CHECK_dict_new( DICT_COMMAND, &data, cxdx_dict, &cmd );
        PARSE_loc_rules( rules, cmd, AVP_BY_NAME_ALL_VENDORS );
      }

  }  /* end Command section */


	
  TRACE_DEBUG(INFO, "Extension 'Dictionary CxDx' initialized");
  return 0;
}


EXTENSION_ENTRY("dict_cxdx", cxdx_dict_init, "dict_dcca_3gpp");
