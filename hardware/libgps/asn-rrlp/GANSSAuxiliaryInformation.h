/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "RRLP-Components"
 * 	found in "../rrlp-components.asn"
 * 	`asn1c -gen-PER`
 */

#ifndef	_GANSSAuxiliaryInformation_H_
#define	_GANSSAuxiliaryInformation_H_


#include <asn_application.h>

/* Including external dependencies */
#include "GANSS-ID1.h"
#include "GANSS-ID3.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum GANSSAuxiliaryInformation_PR {
	GANSSAuxiliaryInformation_PR_NOTHING,	/* No components present */
	GANSSAuxiliaryInformation_PR_ganssID1,
	GANSSAuxiliaryInformation_PR_ganssID3,
	/* Extensions may appear below */
	
} GANSSAuxiliaryInformation_PR;

/* GANSSAuxiliaryInformation */
typedef struct GANSSAuxiliaryInformation {
	GANSSAuxiliaryInformation_PR present;
	union GANSSAuxiliaryInformation_u {
		GANSS_ID1_t	 ganssID1;
		GANSS_ID3_t	 ganssID3;
		/*
		 * This type is extensible,
		 * possible extensions are below.
		 */
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} GANSSAuxiliaryInformation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_GANSSAuxiliaryInformation;

#ifdef __cplusplus
}
#endif

#endif	/* _GANSSAuxiliaryInformation_H_ */
#include <asn_internal.h>