/*
 * This file is machine-generated from the 3GPP TS 29.329 specification.
 * Do not edit.
 */

#include <freeDiameter/extension.h>

#define CHECK_dict_new( _type, _data, _parent, _ref ) \
	CHECK_FCT(  fd_dict_new( fd_g_config->cnf_dict, (_type), (_data), (_parent), (_ref))  );

#define CHECK_dict_search( _type, _criteria, _what, _result ) \
	CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, (_type), (_criteria), (_what), (_result), ENOENT) );

int add_sh_avps()
{
	struct dict_object * UTF8String_type = NULL;
	CHECK_dict_search(DICT_TYPE, TYPE_BY_NAME, "UTF8String", &UTF8String_type);
	struct dict_object * Time_type = NULL;
	CHECK_dict_search(DICT_TYPE, TYPE_BY_NAME, "Time", &Time_type);

	/*==================================================================*/
	/* 3GPP TS 29.329 V15.2.0 (2019-09)                                 */
	/* Sh Interface based on the Diameter protocol;                     */
	/* Protocol details                                                 */
	/*==================================================================*/

	/* User-Identity, Grouped, code 700, section 6.3.1                  */
	{
		struct dict_avp_data data = {
			700,	/* Code */
			10415,	/* Vendor */
			"User-Identity",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flag values */
			AVP_TYPE_GROUPED	/* base type of data */
		};
		CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
	};

	/* MSISDN, OctetString, code 701, section 6.3.2                     */
	{
		struct dict_avp_data data = {
			701,	/* Code */
			10415,	/* Vendor */
			"MSISDN",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flag values */
			AVP_TYPE_OCTETSTRING	/* base type of data */
		};
		CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
	};

	/* Note: Name conflict with 3GPP TS 29.229 User-Data (606).         */
	/* User-Data-29.329, OctetString, code 702, section 6.3.3           */
	{
		struct dict_avp_data data = {
			702,	/* Code */
			10415,	/* Vendor */
			"User-Data-29.329",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flag values */
			AVP_TYPE_OCTETSTRING	/* base type of data */
		};
		CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
	};

	/* Data-Reference, Enumerated, code 703, section 6.3.4              */
	{
		struct dict_avp_data data = {
			703,	/* Code */
			10415,	/* Vendor */
			"Data-Reference",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flag values */
			AVP_TYPE_INTEGER32	/* base type of data */
		};
		struct dict_object	*type;
		struct dict_type_data	 tdata = { AVP_TYPE_INTEGER32, "Enumerated(3GPP/Data-Reference)", NULL, NULL, NULL };
		CHECK_dict_new(DICT_TYPE, &tdata, NULL, &type);
		CHECK_dict_new(DICT_AVP, &data, type, NULL);
	};

	/* Service-Indication, OctetString, code 704, section 6.3.5         */
	{
		struct dict_avp_data data = {
			704,	/* Code */
			10415,	/* Vendor */
			"Service-Indication",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flag values */
			AVP_TYPE_OCTETSTRING	/* base type of data */
		};
		CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
	};

	/* Subs-Req-Type, Enumerated, code 705, section 6.3.6               */
	{
		struct dict_avp_data data = {
			705,	/* Code */
			10415,	/* Vendor */
			"Subs-Req-Type",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flag values */
			AVP_TYPE_INTEGER32	/* base type of data */
		};
		struct dict_object	*type;
		struct dict_type_data	 tdata = { AVP_TYPE_INTEGER32, "Enumerated(3GPP/Subs-Req-Type)", NULL, NULL, NULL };
		CHECK_dict_new(DICT_TYPE, &tdata, NULL, &type);
		CHECK_dict_new(DICT_AVP, &data, type, NULL);
	};

	/* Requested-Domain, Enumerated, code 706, section 6.3.7            */
	{
		struct dict_avp_data data = {
			706,	/* Code */
			10415,	/* Vendor */
			"Requested-Domain",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flag values */
			AVP_TYPE_INTEGER32	/* base type of data */
		};
		struct dict_object	*type;
		struct dict_type_data	 tdata = { AVP_TYPE_INTEGER32, "Enumerated(3GPP/Requested-Domain)", NULL, NULL, NULL };
		CHECK_dict_new(DICT_TYPE, &tdata, NULL, &type);
		CHECK_dict_new(DICT_AVP, &data, type, NULL);
	};

	/* Current-Location, Enumerated, code 707, section 6.3.8            */
	{
		struct dict_avp_data data = {
			707,	/* Code */
			10415,	/* Vendor */
			"Current-Location",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flag values */
			AVP_TYPE_INTEGER32	/* base type of data */
		};
		struct dict_object	*type;
		struct dict_type_data	 tdata = { AVP_TYPE_INTEGER32, "Enumerated(3GPP/Current-Location)", NULL, NULL, NULL };
		CHECK_dict_new(DICT_TYPE, &tdata, NULL, &type);
		CHECK_dict_new(DICT_AVP, &data, type, NULL);
	};

	/* Identity-Set, Enumerated, code 708, section 6.3.10               */
	{
		struct dict_avp_data data = {
			708,	/* Code */
			10415,	/* Vendor */
			"Identity-Set",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_INTEGER32	/* base type of data */
		};
		struct dict_object	*type;
		struct dict_type_data	 tdata = { AVP_TYPE_INTEGER32, "Enumerated(3GPP/Identity-Set)", NULL, NULL, NULL };
		CHECK_dict_new(DICT_TYPE, &tdata, NULL, &type);
		CHECK_dict_new(DICT_AVP, &data, type, NULL);
	};

	/* Expiry-Time, Time, code 709, section 6.3.16                      */
	{
		struct dict_avp_data data = {
			709,	/* Code */
			10415,	/* Vendor */
			"Expiry-Time",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_OCTETSTRING	/* base type of data */
		};
		CHECK_dict_new(DICT_AVP, &data, Time_type, NULL);
	};

	/* Send-Data-Indication, Enumerated, code 710, section 6.3.17       */
	{
		struct dict_avp_data data = {
			710,	/* Code */
			10415,	/* Vendor */
			"Send-Data-Indication",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_INTEGER32	/* base type of data */
		};
		struct dict_object	*type;
		struct dict_type_data	 tdata = { AVP_TYPE_INTEGER32, "Enumerated(3GPP/Send-Data-Indication)", NULL, NULL, NULL };
		CHECK_dict_new(DICT_TYPE, &tdata, NULL, &type);
		CHECK_dict_new(DICT_AVP, &data, type, NULL);
	};

	/* DSAI-Tag, OctetString, code 711, section 6.3.18                  */
	{
		struct dict_avp_data data = {
			711,	/* Code */
			10415,	/* Vendor */
			"DSAI-Tag",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flag values */
			AVP_TYPE_OCTETSTRING	/* base type of data */
		};
		CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
	};

	/* One-Time-Notification, Enumerated, code 712, section 6.3.22      */
	{
		struct dict_avp_data data = {
			712,	/* Code */
			10415,	/* Vendor */
			"One-Time-Notification",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_INTEGER32	/* base type of data */
		};
		struct dict_object	*type;
		struct dict_type_data	 tdata = { AVP_TYPE_INTEGER32, "Enumerated(3GPP/One-Time-Notification)", NULL, NULL, NULL };
		CHECK_dict_new(DICT_TYPE, &tdata, NULL, &type);
		CHECK_dict_new(DICT_AVP, &data, type, NULL);
	};

	/* Requested-Nodes, Unsigned32, code 713, section 6.3.7A            */
	{
		struct dict_avp_data data = {
			713,	/* Code */
			10415,	/* Vendor */
			"Requested-Nodes",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_UNSIGNED32	/* base type of data */
		};
		CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
	};

	/* Serving-Node-Indication, Enumerated, code 714, section 6.3.23    */
	{
		struct dict_avp_data data = {
			714,	/* Code */
			10415,	/* Vendor */
			"Serving-Node-Indication",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_INTEGER32	/* base type of data */
		};
		struct dict_object	*type;
		struct dict_type_data	 tdata = { AVP_TYPE_INTEGER32, "Enumerated(3GPP/Serving-Node-Indication)", NULL, NULL, NULL };
		CHECK_dict_new(DICT_TYPE, &tdata, NULL, &type);
		CHECK_dict_new(DICT_AVP, &data, type, NULL);
	};

	/* Repository-Data-ID, Grouped, code 715, section 6.3.24            */
	{
		struct dict_avp_data data = {
			715,	/* Code */
			10415,	/* Vendor */
			"Repository-Data-ID",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_GROUPED	/* base type of data */
		};
		CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
	};

	/* Sequence-Number, Unsigned32, code 716, section 6.3.25            */
	{
		struct dict_avp_data data = {
			716,	/* Code */
			10415,	/* Vendor */
			"Sequence-Number",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_UNSIGNED32	/* base type of data */
		};
		CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
	};

	/* Pre-paging-Supported, Enumerated, code 717, section 6.3.26       */
	{
		struct dict_avp_data data = {
			717,	/* Code */
			10415,	/* Vendor */
			"Pre-paging-Supported",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_INTEGER32	/* base type of data */
		};
		struct dict_object	*type;
		struct dict_type_data	 tdata = { AVP_TYPE_INTEGER32, "Enumerated(3GPP/Pre-paging-Supported)", NULL, NULL, NULL };
		CHECK_dict_new(DICT_TYPE, &tdata, NULL, &type);
		CHECK_dict_new(DICT_AVP, &data, type, NULL);
	};

	/* Local-Time-Zone-Indication, Enumerated, code 718, section 6.3.27 */
	{
		struct dict_avp_data data = {
			718,	/* Code */
			10415,	/* Vendor */
			"Local-Time-Zone-Indication",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_INTEGER32	/* base type of data */
		};
		struct dict_object	*type;
		struct dict_type_data	 tdata = { AVP_TYPE_INTEGER32, "Enumerated(3GPP/Local-Time-Zone-Indication)", NULL, NULL, NULL };
		CHECK_dict_new(DICT_TYPE, &tdata, NULL, &type);
		CHECK_dict_new(DICT_AVP, &data, type, NULL);
	};

	/* UDR-Flags, Unsigned32, code 719, section 6.3.28                  */
	{
		struct dict_avp_data data = {
			719,	/* Code */
			10415,	/* Vendor */
			"UDR-Flags",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_UNSIGNED32	/* base type of data */
		};
		CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
	};

	/* Call-Reference-Info, Grouped, code 720, section 6.3.29           */
	{
		struct dict_avp_data data = {
			720,	/* Code */
			10415,	/* Vendor */
			"Call-Reference-Info",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_GROUPED	/* base type of data */
		};
		CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
	};

	/* Call-Reference-Number, OctetString, code 721, section 6.3.30     */
	{
		struct dict_avp_data data = {
			721,	/* Code */
			10415,	/* Vendor */
			"Call-Reference-Number",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_OCTETSTRING	/* base type of data */
		};
		CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
	};

	/* AS-Number, OctetString, code 722, section 6.3.31                 */
	{
		struct dict_avp_data data = {
			722,	/* Code */
			10415,	/* Vendor */
			"AS-Number",	/* Name */
			AVP_FLAG_VENDOR |AVP_FLAG_MANDATORY,	/* Fixed flags */
			AVP_FLAG_VENDOR,	/* Fixed flag values */
			AVP_TYPE_OCTETSTRING	/* base type of data */
		};
		CHECK_dict_new(DICT_AVP, &data, NULL, NULL);
	};

	return 0;
}