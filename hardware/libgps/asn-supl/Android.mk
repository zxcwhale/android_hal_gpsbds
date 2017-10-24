LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := liblog libcutils libhardware libc libutils

LOCAL_SRC_FILES :=  Version.c \
    SessionID.c	\
	SetSessionID.c	\
	SETId.c	\
	SlpSessionID.c	\
	IPAddress.c	\
	SLPAddress.c	\
	FQDN.c	\
	Ver.c	\
	LocationId.c	\
	Status.c	\
	CellInfo.c	\
	Position.c	\
	PositionEstimate.c	\
	AltitudeInfo.c	\
	CdmaCellInformation.c	\
	GsmCellInformation.c	\
	WcdmaCellInformation.c	\
	FrequencyInfo.c	\
	FrequencyInfoFDD.c	\
	FrequencyInfoTDD.c	\
	UARFCN.c	\
	NMR.c	\
	NMRelement.c	\
	MeasuredResultsList.c	\
	MeasuredResults.c	\
	CellMeasuredResultsList.c	\
	UTRA-CarrierRSSI.c	\
	CellMeasuredResults.c	\
	CellParametersID.c	\
	TGSN.c	\
	PrimaryCCPCH-RSCP.c	\
	TimeslotISCP.c	\
	TimeslotISCP-List.c	\
	PrimaryCPICH-Info.c	\
	CPICH-Ec-N0.c	\
	CPICH-RSCP.c	\
	Pathloss.c	\
	QoP.c	\
	Velocity.c	\
	Horvel.c	\
	Horandvervel.c	\
	Horveluncert.c	\
	Horandveruncert.c	\
	StatusCode.c	\
	PosMethod.c	\
	SUPLEND.c	\
	SUPLINIT.c	\
	Notification.c	\
	NotificationType.c	\
	EncodingType.c	\
	FormatIndicator.c	\
	SLPMode.c	\
	MAC.c	\
	KeyIdentity.c	\
	SUPLPOS.c	\
	PosPayLoad.c	\
	SUPLPOSINIT.c	\
	RequestedAssistData.c	\
	XNavigationModel.c	\
	SatelliteInfo.c	\
	SatelliteInfoElement.c	\
	SUPLRESPONSE.c	\
	SETAuthKey.c	\
	KeyIdentity4.c	\
	SUPLSTART.c	\
	SETCapabilities.c	\
	PosTechnology.c	\
	PrefMethod.c	\
	PosProtocol.c	\
	ULP-PDU.c	\
	UlpMessage.c	\
	DUMMY.c	

LOCAL_SRC_FILES+=BOOLEAN.c
LOCAL_SRC_FILES+=GeneralizedTime.c
LOCAL_SRC_FILES+=IA5String.c
LOCAL_SRC_FILES+=INTEGER.c
LOCAL_SRC_FILES+=NativeEnumerated.c
LOCAL_SRC_FILES+=NativeInteger.c
LOCAL_SRC_FILES+=UTCTime.c
LOCAL_SRC_FILES+=VisibleString.c
LOCAL_SRC_FILES+=asn_SEQUENCE_OF.c
LOCAL_SRC_FILES+=asn_SET_OF.c
LOCAL_SRC_FILES+=constr_CHOICE.c
LOCAL_SRC_FILES+=constr_SEQUENCE.c
LOCAL_SRC_FILES+=constr_SEQUENCE_OF.c
LOCAL_SRC_FILES+=constr_SET_OF.c
LOCAL_SRC_FILES+=OCTET_STRING.c
LOCAL_SRC_FILES+=BIT_STRING.c
LOCAL_SRC_FILES+=asn_codecs_prim.c
LOCAL_SRC_FILES+=ber_tlv_length.c
LOCAL_SRC_FILES+=ber_tlv_tag.c
LOCAL_SRC_FILES+=ber_decoder.c
LOCAL_SRC_FILES+=der_encoder.c
LOCAL_SRC_FILES+=constr_TYPE.c
LOCAL_SRC_FILES+=constraints.c
LOCAL_SRC_FILES+=xer_support.c
LOCAL_SRC_FILES+=xer_decoder.c
LOCAL_SRC_FILES+=xer_encoder.c
LOCAL_SRC_FILES+=per_support.c
LOCAL_SRC_FILES+=per_decoder.c
LOCAL_SRC_FILES+=per_encoder.c
LOCAL_SRC_FILES+=per_opentype.c

LOCAL_MODULE := libasnsupl
LOCAL_MODULE_TAGS := debug eng
include $(BUILD_SHARED_LIBRARY)
