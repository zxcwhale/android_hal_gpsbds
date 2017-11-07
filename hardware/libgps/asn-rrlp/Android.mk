LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_SHARED_LIBRARIES := liblog libcutils libhardware libc libutils

LOCAL_SRC_FILES := \
	VelocityEstimate.c	\
	ExtensionContainer.c	\
	Ext-GeographicalInformation.c	\
	GPSAcquisAssist-R12-Ext-Element.c \
	MsrPosition-Req.c	\
	MsrPosition-Rsp.c	\
	AssistanceData.c	\
	ProtocolError.c	\
	PositionInstruct.c	\
	MethodType.c	\
	AccuracyOpt.c	\
	Accuracy.c	\
	PositionMethod.c	\
	MeasureResponseTime.c	\
	UseMultipleSets.c	\
	EnvironmentCharacter.c	\
	ReferenceAssistData.c	\
	BTSPosition.c	\
	BCCHCarrier.c	\
	BSIC.c	\
	TimeSlotScheme.c	\
	ModuloTimeSlot.c	\
	MsrAssistData.c	\
	SeqOfMsrAssistBTS.c	\
	MsrAssistBTS.c	\
	MultiFrameOffset.c	\
	RoughRTD.c	\
	SystemInfoAssistData.c	\
	SeqOfSystemInfoAssistBTS.c	\
	SystemInfoAssistBTS.c	\
	AssistBTSData.c	\
	CalcAssistanceBTS.c	\
	ReferenceWGS84.c	\
	FineRTD.c	\
	RelDistance.c	\
	RelativeAlt.c	\
	MultipleSets.c	\
	ReferenceRelation.c	\
	ReferenceIdentity.c	\
	SeqOfReferenceIdentityType.c	\
	ReferenceIdentityType.c	\
	BSICAndCarrier.c	\
	RequestIndex.c	\
	SystemInfoIndex.c	\
	CellIDAndLAC.c	\
	CellID.c	\
	LAC.c	\
	OTD-MeasureInfo.c	\
	SeqOfOTD-MsrElementRest.c	\
	OTD-MsrElementFirst.c	\
	SeqOfOTD-FirstSetMsrs.c	\
	OTD-MsrElementRest.c	\
	SeqOfOTD-MsrsOfOtherSets.c	\
	TOA-MeasurementsOfRef.c	\
	RefQuality.c	\
	NumOfMeasurements.c	\
	StdResolution.c	\
	OTD-FirstSetMsrs.c	\
	OTD-MsrsOfOtherSets.c	\
	OTD-Measurement.c	\
	OTD-MeasurementWithID.c	\
	EOTDQuality.c	\
	NeighborIdentity.c	\
	MultiFrameCarrier.c	\
	OTDValue.c	\
	LocationInfo.c	\
	FixType.c	\
	GPS-MeasureInfo.c	\
	SeqOfGPS-MsrSetElement.c	\
	GPS-MsrSetElement.c	\
	GPSTOW24b.c	\
	SeqOfGPS-MsrElement.c	\
	GPS-MsrElement.c	\
	MpathIndic.c	\
	LocationError.c	\
	LocErrorReason.c	\
	AdditionalAssistanceData.c	\
	GPSAssistanceData.c	\
	GANSSAssistanceData.c	\
	ErrorCodes.c	\
	GPS-AssistData.c	\
	MoreAssDataToBeSent.c	\
	ControlHeader.c	\
	ReferenceTime.c	\
	GPSTime.c	\
	GPSTOW23b.c	\
	GPSWeek.c	\
	GPSTOWAssist.c	\
	GPSTOWAssistElement.c	\
	TLMWord.c	\
	AntiSpoofFlag.c	\
	AlertFlag.c	\
	TLMReservedBits.c	\
	GSMTime.c	\
	FrameNumber.c	\
	TimeSlot.c	\
	BitNumber.c	\
	RefLocation.c	\
	DGPSCorrections.c	\
	SeqOfSatElement.c	\
	SatElement.c	\
	SatelliteID.c	\
	NavigationModel.c	\
	SeqOfNavModelElement.c	\
	NavModelElement.c	\
	SatStatus.c	\
	UncompressedEphemeris.c	\
	EphemerisSubframe1Reserved.c	\
	IonosphericModel.c	\
	UTCModel.c	\
	Almanac.c	\
	SeqOfAlmanacElement.c	\
	AlmanacElement.c	\
	AcquisAssist.c	\
	SeqOfAcquisElement.c	\
	TimeRelation.c	\
	AcquisElement.c	\
	AddionalDopplerFields.c	\
	AddionalAngleFields.c	\
	SeqOf-BadSatelliteSet.c	\
	Rel98-MsrPosition-Req-Extension.c	\
	Rel98-AssistanceData-Extension.c	\
	Rel98-Ext-ExpOTD.c	\
	MsrAssistData-R98-ExpOTD.c	\
	SeqOfMsrAssistBTS-R98-ExpOTD.c	\
	MsrAssistBTS-R98-ExpOTD.c	\
	SystemInfoAssistData-R98-ExpOTD.c	\
	SeqOfSystemInfoAssistBTS-R98-ExpOTD.c	\
	SystemInfoAssistBTS-R98-ExpOTD.c	\
	AssistBTSData-R98-ExpOTD.c	\
	ExpectedOTD.c	\
	ExpOTDUncertainty.c	\
	GPSReferenceTimeUncertainty.c	\
	GPSTimeAssistanceMeasurements.c	\
	Rel-98-MsrPosition-Rsp-Extension.c	\
	OTD-MeasureInfo-R98-Ext.c	\
	OTD-MsrElementFirst-R98-Ext.c	\
	SeqOfOTD-FirstSetMsrs-R98-Ext.c	\
	Rel-5-MsrPosition-Rsp-Extension.c	\
	Extended-reference.c	\
	OTD-MeasureInfo-5-Ext.c	\
	UlPseudoSegInd.c	\
	Rel5-MsrPosition-Req-Extension.c	\
	Rel5-AssistanceData-Extension.c	\
	Rel-5-ProtocolError-Extension.c	\
	Rel7-MsrPosition-Req-Extension.c	\
	GANSSPositioningMethod.c	\
	GANSS-AssistData.c	\
	GANSS-ControlHeader.c	\
	GANSSCommonAssistData.c	\
	SeqOfGANSSGenericAssistDataElement.c	\
	GANSSGenericAssistDataElement.c	\
	GANSSReferenceTime.c	\
	GANSSRefTimeInfo.c	\
	GANSSTOD.c	\
	GANSSTODUncertainty.c	\
	GANSSTOD-GSMTimeAssociation.c	\
	FrameDrift.c	\
	GANSSRefLocation.c	\
	GANSSIonosphericModel.c	\
	GANSSIonosphereModel.c	\
	GANSSIonoStormFlags.c	\
	SeqOfGANSSTimeModel.c	\
	GANSSTimeModelElement.c	\
	TA0.c	\
	TA1.c	\
	TA2.c	\
	GANSSDiffCorrections.c	\
	SeqOfSgnTypeElement.c	\
	SgnTypeElement.c	\
	GANSSSignalID.c	\
	SeqOfDGANSSSgnElement.c	\
	DGANSSSgnElement.c	\
	SVID.c	\
	GANSSNavModel.c	\
	SeqOfGANSSSatelliteElement.c	\
	GANSSSatelliteElement.c	\
	GANSSOrbitModel.c	\
	NavModel-KeplerianSet.c	\
	GANSSClockModel.c	\
	SeqOfStandardClockModelElement.c	\
	StandardClockModelElement.c	\
	GANSSRealTimeIntegrity.c	\
	SeqOfBadSignalElement.c	\
	BadSignalElement.c	\
	GANSSDataBitAssist.c	\
	SeqOf-GANSSDataBits.c	\
	GANSSDataBit.c	\
	GANSSRefMeasurementAssist.c	\
	SeqOfGANSSRefMeasurementElement.c	\
	GANSSRefMeasurementElement.c	\
	AdditionalDopplerFields.c	\
	GANSSAlmanacModel.c	\
	SeqOfGANSSAlmanacElement.c	\
	GANSSAlmanacElement.c	\
	Almanac-KeplerianSet.c	\
	GANSSUTCModel.c	\
	RequiredResponseTime.c	\
	Rel-7-MsrPosition-Rsp-Extension.c	\
	GANSSLocationInfo.c	\
	PositionData.c	\
	GANSSTODm.c	\
	ReferenceFrame.c	\
	GANSSMeasureInfo.c	\
	SeqOfGANSS-MsrSetElement.c	\
	GANSS-MsrSetElement.c	\
	SeqOfGANSS-SgnTypeElement.c	\
	GANSS-SgnTypeElement.c	\
	SeqOfGANSS-SgnElement.c	\
	GANSS-SgnElement.c	\
	Rel7-AssistanceData-Extension.c	\
	PDU.c	\
	GANSSAlmanacModel-R12-Ext.c \
	GLONASSclockModel.c \
	GANSSRefMeasurementAssist-R12-Ext.c \
	BDS-GridModelParameter-r12.c \
	GANSSRefMeasurementAssist-R10-Ext.c \
	PosCapability-Req.c \
	NavModel-CNAVKeplerianSet.c \
	SeqOfGanssDataBitsElement.c \
	SeqOfGANSSTimeModel-R10-Ext.c \
	GANSSSignals.c \
	GANSSAuxiliaryInformation.c \
	GANSSEphemerisExtension.c \
	NAVclockModel.c \
	GANSSAddIonosphericModel.c \
	NavModel-SBASecef.c \
	GANSSDiffCorrectionsValidityPeriod.c \
	NavModel-GLONASSecef.c \
	GANSSAddUTCModel.c \
	GANSSReferenceTime-R10-Ext.c \
	GANSSEphemerisExtensionCheck.c \
	Add-GPS-AssistData.c \
	Almanac-BDSAlmanacSet-r12.c \
	Almanac-NAVKeplerianSet.c \
	GANSSEarthOrientParam.c \
	BDSClockModel-r12.c \
	SBASclockModel.c \
	PosCapability-Rsp.c \
	Almanac-MidiAlmanacSet.c \
	NavModel-BDSKeplerianSet-r12.c \
	SeqOfGANSS-MsrElement.c \
	BDS-DiffCorrections-r12.c \
	CNAVclockModel.c \
	GANSSAlmanacModel-R10-Ext.c \
	Almanac-ECEFsbasAlmanacSet.c \
	Almanac-GlonassAlmanacSet.c \
	Almanac-ReducedKeplerianSet.c \
	NavModel-NAVKeplerianSet.c \
	GANSSTimeModelElement-R10-Ext.c \
	PosCapabilities.c \
	GANSSEphemerisDeltaMatrix.c \
	GANSS-ID1.c \
	GANSSSatEventsInfo.c \
	UTCmodelSet4.c \
	GANSSRefMeasurement-R10-Ext-Element.c \
	SeqOfGANSSRefOrbit.c \
	AssistanceSupported.c \
	GANSSPositionMethods.c \
	BDS-SgnTypeList-r12.c \
	DGANSSExtensionSgnTypeElement.c \
	UTCmodelSet5-r12.c \
	SeqOfGANSSRefMeasurementElement-R12.c \
	GanssDataBitsElement.c \
	GANSSEphemerisExtensionTime.c \
	UTCmodelSet3.c \
	AssistanceNeeded.c \
	GANSS-MsrElement.c \
	Add-GPS-ControlHeader.c \
	GridIonList-r12.c \
	GANSS-ID3.c \
	GANSSEphemerisExtensionHeader.c \
	UTCmodelSet2.c \
	GANSSAssistanceSet.c \
	MultipleMeasurementSets.c \
	SeqOfDGANSSExtensionSgnElement.c \
	GANSS-ID3-element.c \
	GANSSRefMeasurement-R12-Ext-Element.c \
	GANSS-ID1-element.c \
	GPSEphemerisExtension.c \
	GANSSAdditionalAssistanceChoices.c \
	GridIonElement-r12.c \
	GANSSPositionMethod.c \
	GPSAlmanac-R10-Ext.c \
	NonGANSSPositionMethods.c \
	GANSSReferenceOrbit.c \
	GPSEphemerisExtensionCheck.c \
	GPSAcquisAssist-R12-Ext.c \
	BDS-SgnTypeElement-r12.c \
	GPSAssistance.c \
	GPSAcquisAssist-R10-Ext.c \
	GPSReferenceTime-R10-Ext.c \
	GANSSEphemerisDeltaEpoch.c \
	DGPSCorrectionsValidityPeriod.c \
	Seq-OfGANSSDataBitsSgn.c \
	GANSSDeltaEpochHeader.c \
	DGANSSExtensionSgnElement.c \
	DGPSExtensionSatElement.c \
	GPSEphemerisExtensionTime.c \
	SpecificGANSSAssistance.c \
	GANSSDeltaElementList.c \
	ReferenceNavModel.c \
	GPSSatEventsInfo.c \
	GPSEphemerisDeltaMatrix.c \
	SBASID.c \
	GPSEphemerisExtensionHeader.c \
	GANSSPositioningMethodTypes.c \
	SeqOfGPSRefOrbit.c \
	GPSAcquisAssist-R10-Ext-Element.c \
	GANSSAdditionalAssistanceChoicesForOneGANSS.c \
	CommonGANSSAssistance.c \
	SeqOfGPSAcquisAssist-R12-Ext.c \
	GANSSDataBitsSgnElement.c \
	DBDS-CorrectionList-r12.c \
	GANSSAssistanceForOneGANSS.c \
	GANSSEphemerisDeltaScales.c \
	GANSSModelID.c \
	GPSReferenceOrbit.c \
	DBDS-CorrectionElement-r12.c \
	GANSSEphemerisDeltaBitSizes.c \
	GPSEphemerisDeltaEpoch.c \
	GPSClockModel.c \
	GPSDeltaElementList.c \
	GPSDeltaEpochHeader.c \
	GANSSAssistance.c \
	GPSEphemerisDeltaScales.c \
	GPSEphemerisDeltaBitSizes.c \
	RRLP-Component.c


LOCAL_SRC_FILES+=BOOLEAN.c
LOCAL_SRC_FILES+=INTEGER.c
LOCAL_SRC_FILES+=NULL.c
LOCAL_SRC_FILES+=NativeEnumerated.c
LOCAL_SRC_FILES+=NativeInteger.c
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


LOCAL_MODULE := libasnrrlp
LOCAL_MODULE_TAGS := debug eng
include $(BUILD_SHARED_LIBRARY)
