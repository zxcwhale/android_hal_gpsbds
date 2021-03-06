/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "Ver2-ULP-Components"
 * 	found in "../ver2-ulp-components.asn"
 * 	`asn1c -fcompound-names -gen-PER`
 */

#ifndef	_SPCTID_H_
#define	_SPCTID_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>
#include "FQDN.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SPCTID */
typedef struct SPCTID {
	BIT_STRING_t	 rAND;
	FQDN_t	 slpFQDN;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SPCTID_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SPCTID;

#ifdef __cplusplus
}
#endif

#endif	/* _SPCTID_H_ */
#include <asn_internal.h>
