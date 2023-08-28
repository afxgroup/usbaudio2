#ifndef __AUDIO2_H
#define __AUDIO2_H

#include <usb/usb.h>

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
   #ifdef __PPC__
    #pragma pack(1)
   #endif
#elif defined(__VBCC__)
   #pragma pack(1)
#endif

/****************************************************************************/

extern int32 Verbose;

/* Audio interface class code */
#define AUDIO_CLASS 1

/* Audio interface subclass codes */
#define AUDIO_SUBCLASS_AUDIOCONTROL      1
#define AUDIO_SUBCLASS_AUDIOSTREAMING    2
#define AUDIO_SUBCLASS_MIDISTREAMING     3

/* Audio class specific request codes */
#define REQUEST_CODE_UNDEFINED	0x00
#define CUR                   0x01
#define RANGE                 0x02
#define MEM                   0x03

#define CS_CONTROL_UNDEFINED 0x00
#define CS_SAM_FREQ_CONTROL   0x01
#define CS_CLOCK_VALID_CONTROL 0x02

/* Format Type Codes	*/
#define FORMAT_TYPE_I	0x01
#define FORMAT_TYPE_II	0x02
#define FORMAT_TYPE_III	0x03
#define FORMAT_TYPE_IV	0x04
#define EXT_FORMAT_TYPE_I		0x81
#define EXT_FORMAT_TYPE_II		0x82
#define EXT_FORMAT_TYPE_III	0x83

/* Audio Format type I bit allocations	*/
#define TYPE_I_PCM			0x00000001
#define TYPE_I_PCM8			0x00000002
#define TYPE_I_IEEE_FLOAT	0x00000004
#define TYPE_I_ALAW			0x00000008
#define TYPE_I_MULAW			0x00000010
#define TYPE_I_RAW_DATA		0x80000000

/* Audio Format type II bit allocations	*/
#define TYPE_II_MPEG			0x00000001
#define TYPE_II_AC_3			0x00000002
#define TYPE_II_WMA			0x00000004
#define TYPE_II_DTS			0x00000008
#define TYPE_II_RAW_DATA	0x80000000

/* Audio Format type III bit allocations	*/
#define TYPE_III_IEC61937_AC_3					0x00000001
#define TYPE_III_IEC61937_MPEG_1_Layer1		0x00000002
#define TYPE_III_IEC61937_MPEG_1_Layer2_3		0x00000004
#define TYPE_III_IEC61937_MPEG_2_NOEXT			0x00000008
#define TYPE_III_IEC61937_MPEG_2_EXT			0x00000010
#define TYPE_III_IEC61937_MPEG_2_AAC_ADTS		0x00000020
#define TYPE_III_IEC61937_MPEG_2_Layer1_LS	0x00000040
#define TYPE_III_IEC61937_MPEG_2_Layer2_3_LS	0x00000080
#define TYPE_III_IEC61937_DTS_I					0x00000100
#define TYPE_III_IEC61937_DTS_II					0x00000200
#define TYPE_III_IEC61937_DTS_III				0x00000400
#define TYPE_III_IEC61937_ATRAC					0x00000800
#define TYPE_III_IEC61937_ATRAC2_3				0x00001000
#define TYPE_III_IEC61937_TYPE_III_WMA			0x00002000

/* Audio Format type IV bit allocations	*/
#define TYPE_IV_PCM	0x00000001

/* Audio class-specific descriptor types (as returned by GET_DESCRIPTOR) */
#define AUDIODESC_UNDEFINED		0x20
#define AUDIODESC_DEVICE			0x21
#define AUDIODESC_CONFIGURATION	0x22
#define AUDIODESC_STRING			0x23
#define AUDIODESC_INTERFACE		0x24
#define AUDIODESC_ENDPOINT			0x25

/* Sub descriptor types */
#define AUDIOSUBDESC_HEADER      0x01
#define AUDIOSUBDESC_INPUT       0x02
#define AUDIOSUBDESC_OUTPUT      0x03
#define AUDIOSUBDESC_MIXER       0x04
#define AUDIOSUBDESC_SELECTOR    0x05
#define AUDIOSUBDESC_FEATURE     0x06
#define AUDIOSUBDESC_EFFECT      0x07
#define AUDIOSUBDESC_PROCESSING  0x08
#define AUDIOSUBDESC_EXTENSION   0x09
#define AUDIOSUBDESC_CLOCK_SOURCE 0x0a
#define AUDIOSUBDESC_CLOCK_SELECTOR 0x0B
#define AUDIOSUBDESC_CLOCK_MULTIPLIER 0x0C
#define AUDIOSUBDESC_SAMPLE_RATE_CONVERTER 0x0D

/* Audio class-specific audiostream interface descriptor subtypes */
#define AUDIOSTREAMSUBSEC_GENERAL 			0x01
#define AUDIOSTREAMSUBSEC_FORMATTYPE 		0x02
#define AUDIOSTREAMSUBSEC_ENCODER      	0x03
#define AUDIOSTREAMSUBSEC_DECODER      	0x04

/* MPEG control selectors */
#define MPEGCONTROL_DUALCHANNEL		0x01
#define MPEGCONTROL_SECOND_STEREO	0x02
#define MPEGCONTROL_MULTILINGUAL		0x03
#define MPEGCONTROL_DYN_RANGE			0x04
#define MPEGCONTROL_SCALING			0x05
#define MPEGCONTROL_HILO_SCALING		0x06

/* AC-3 control selectors */
#define AC3CONTROL_MODE				0x01
#define AC3CONTROL_DYN_RANGE		0x02
#define AC3CONTROL_SCALING			0x03
#define AC3CONTROL_HILO_SCALING	0x04

/* Audio processing unit types */
#define AUDIOPROCESSTYPE_UPDOWNMIX 			0x01
#define AUDIOPROCESSTYPE_DOLBYPROLOGIC 	0x02
#define AUDIOPROCESSTYPE_3DSTEREOEXTENDER 0x03
#define AUDIOPROCESSTYPE_REVERB 				0x04
#define AUDIOPROCESSTYPE_CHORUS 				0x05
#define AUDIOPROCESSTYPE_RANGE_COMPRESSOR 0x06

#define FU_MUTE				0x01
#define FU_VOLUME				0x02
#define FU_BASS				0x03
#define FU_MID					0x04
#define FU_TREBLE				0x05
#define FU_GRAPHICEQ			0x06
#define FU_AUTOGAIN			0x07
#define FU_DELAY				0x08
#define FU_BASSBOOST			0x09
#define FU_LOUDNESS			0x0A
#define FU_INPUT_GAIN		0x0B
#define FU_INPUT_GAIN_PAD	0x0C
#define FU_PHASE_INVERTER	0x0D
#define FU_UNDERFLOW			0x0E
#define FU_OVERFLOW			0x0F
#define FU_LATENCY			0x10

/* USB Terminal Types */
#define USBTERMINALTYPE_STREAMING 	0x0101
#define USBTERMINALTYPE_VENDOR 		0x01FF


/* Input Terminal types */
#define AUDIOINTERMINALTYPE_MICROPHONE 			0x0201
#define AUDIOINTERMINALTYPE_DESKMICROPHONE 		0x0202
#define AUDIOINTERMINALTYPE_PERSONALMICROPHONE	0x0203
#define AUDIOINTERMINALTYPE_OMNIMICROPHONE 		0x0204
#define AUDIOINTERMINALTYPE_MICARRAY 				0x0205
#define AUDIOINTERMINALTYPE_PROCESSEDMICARRAY 	0x0206

/* Output Terminal Types */
#define AUDIOOUTTERMINALTYPE_SPEAKER 		0x0301
#define AUDIOOUTTERMINALTYPE_HEADPHONES 	0x0302
#define AUDIOOUTTERMINALTYPE_HMD 			0x0303
#define AUDIOOUTTERMINALTYPE_DESKSPEAKER 	0x0304
#define AUDIOOUTTERMINALTYPE_ROOMSPEAKER 	0x0305
#define AUDIOOUTTERMINALTYPE_COMMSPEAKER 	0x0306
#define AUDIOOUTTERMINALTYPE_SUBWOOFER 	0x0307

/* BiDirectional Terminal types */
#define AUDIOBITERMINALTYPE_HANDSET 			0x0401
#define AUDIOBITERMINALTYPE_HEADSET 			0x0402
#define AUDIOBITERMINALTYPE_SPEAKERPHONE 		0x0403
#define AUDIOBITERMINALTYPE_ESSPEAKERPHONE 	0x0404
#define AUDIOBITERMINALTYPE_ECSPEAKERPHONE	0x0405

/* Telephone Terminal Types*/
#define AUDIOPHONETERMINALTYPE_PHONELINE		0x0501
#define AUDIOPHONETERMINALTYPE_TELEPHONE		0x0502
#define AUDIOPHONETERMINALTYPE_DOWNLINEPHONE 0x0503

/* External Terminal types */
#define EXT_AUDIOTERMINAL_ANALOG		0x0601
#define EXT_AUDIOTERMINAL_DIGITAL	0x0602
#define EXT_AUDIOTERMINAL_LINE		0x0603
#define EXT_AUDIOTERMINAL_LEGACY		0x0604
#define EXT_AUDIOTERMINAL_SPDIF		0x0605
#define EXT_AUDIOTERMINAL_DASTREAM	0x0606
#define EXT_AUDIOTERMINAL_DVSTREAM	0x0607

/* Internal Terminal Types */
#define EMB_AUDIOTERMINALTYPE_CALNOISE		0x0701
#define EMB_AUDIOTERMINALTYPE_EQNOISE		0x0702
#define EMB_AUDIOTERMINALTYPE_CDPLAYER		0x0703
#define EMB_AUDIOTERMINALTYPE_DAT			0x0704
#define EMB_AUDIOTERMINALTYPE_DCC			0x0705
#define EMB_AUDIOTERMINALTYPE_MD				0x0706
#define EMB_AUDIOTERMINALTYPE_ANALOGTAPE	0x0707
#define EMB_AUDIOTERMINALTYPE_PHONO			0x0708
#define EMB_AUDIOTERMINALTYPE_VCR			0x0709
#define EMB_AUDIOTERMINALTYPE_VIDDISC		0x070A
#define EMB_AUDIOTERMINALTYPE_DVD			0x070B
#define EMB_AUDIOTERMINALTYPE_TV				0x070C
#define EMB_AUDIOTERMINALTYPE_SAT			0x070D
#define EMB_AUDIOTERMINALTYPE_CABLE			0x070E
#define EMB_AUDIOTERMINALTYPE_DSS			0x070F
#define EMB_AUDIOTERMINALTYPE_RADIORX		0x0710
#define EMB_AUDIOTERMINALTYPE_RADIOTX		0x0711
#define EMB_AUDIOTERMINALTYPE_MULTITRACK	0x0712
#define EMB_AUDIOTERMINALTYPE_SYNTH 		0x0713

// alternate configuration query/control
#define AS_AUDIO_DATA_FORMAT_CONTROL      0x03	// pp 117,118
#define AS_ACT_ALT_SETTING_CONTROL        0x01
#define AS_VAL_ALT_SETTINGS_CONTROL       0x02


struct AudioDscHead
{
    struct USBBusDscHead    Head;
    uint8 subType;				/* Descriptor subtype */
};

struct AudioChannelCluster
{
	uint8 bNrChannels;
	uint32 bmChannelConfig;
	uint8 iChannelNames;		// string index
};

// audio device descriptor will use standard device descriptor
// with the following values:
// 0xEF, 0x02, 0x01, for class, subclass and protocol

// device qualifier is also 0xEF, 0x02, and 0x01

// config descriptor is standard

// other_speed descriptor is also standard


struct USBIntAssocDsc
{
	uint8 bLength;					// 8
	uint8 bDescriptorType;		// INTERFACE_ASSOCIATION
	uint8 bFirstInterface;
	uint8 bInterfaceCount;
	uint8 bFunctionClass;		// AUDIO_FUNCTION_...
	uint8 bFunctionSubClass;	// FUNCTION_SUBCLASS_UNDEFINED
	uint8 bFunctionProtocol;	// AF_VERSION_02_00
	uint8 iFunction;
};


/* AUDIOSUBDESC_HEADER */
struct AudioHeaderDsc
{
	struct AudioDscHead Head;
	uint16 bcdADC;				/* Revision of class specification (1.0) */
	uint16 totalLength;			/* Total size of class specific descriptors */
	uint8 bInCollection;		/* Number of streaming interfaces */
	uint8 baInterfaceNr[1];     /* AudioStreaming interface 1 belongs to this AudioControl interface */
};

/* AUDIOSUBDESC_INPUT */
struct AudioInputDscv1
{	// length == 11
	struct AudioDscHead Head;
	uint8   bTerminalId;
	uint16   wTerminalType;
	uint8   bAssocTerminal;
	uint8   bNrChannels;
	uint16   wChannelConfig;
	uint8   iChannelNames;
};

/* AUDIOSUBDESC_INPUT */
struct AudioInputDscv2
{		// length == 17
	struct AudioDscHead Head;
	uint8   bTerminalId;
	uint16   wTerminalType;
	uint8   bAssocTerminal;
	uint8   bCSourceID;
	uint8   bNrChannels;
	uint32  bmChannelConfig;
	uint8   iChannelNames;	// string to first logical channel
	uint16  bmControls;
	uint8   iTerminal;	// name of input terminal
};

/* AUDIOSUBDESC_OUTPUT */
struct AudioOutputDscv1
{
	struct AudioDscHead Head;
	uint8   bTerminalId;
	uint16   wTerminalType;
	uint8   bAssocTerminal;
	uint8	  bSourceId;
	uint8 	iTerminal;
};
/* AUDIOSUBDESC_OUTPUT */
struct AudioOutputDscv2
{
	struct AudioDscHead Head;
	uint8   bTerminalId;
	uint16  wTerminalType;
	uint8   bAssocTerminal;
	uint8	  bSourceId;
	uint8	  bCSourceId;
	uint16  bmControls;
	uint8   iTerminal;
};

// AC interface header descriptor
struct AudioClassInterfaceHeaderDsc
{
	struct AudioDscHead Head;
	uint16	bcdADC;
	uint8		bCategory;
	uint16	wTotalLength;
	uint8 	bmControls;
};

/* AUDIOSUBDESC_CLOCK_SOURCE */
struct AudioClockSourceDsc
{
	struct AudioDscHead Head;
	uint8   bClockId;
	uint8   bmAttributes;
	uint8   bmControls;
	uint8	  bAssocTerminal;	// terminal ID
	uint8   iClockSource;	// index to string
};

/* AUDIOSUBDESC_CLOCK_SELECTOR */
struct AudioClockSelectorDsc
{
	struct AudioDscHead Head;
	uint8   bClockId;
	uint8   bNrInPins;	// p
	uint8   baCSourceID1;	//(1)
//...
	uint8   baCSourceIDp;	//(p)
	uint8	  bmControls;
	uint8   iClockSelector;	// index to string
};

/* AUDIOSUBDESC_CLOCK_MULTIPLIER */
struct AudioClockMultiplierDsc
{
	struct AudioDscHead Head;
	uint8   bClockId;
	uint8   bCSourceID;
	uint8   bmControls;
	uint8   iClockMultiplier;	// index to string
};

/* AUDIOSUBDESC_FEATURE */
struct AudioFeatureDsc
{
	struct AudioDscHead Head;
	uint8 bUnitId;
	uint8 bSourceId;
	uint32 bmaControls;
	/* A feature index follows */
};

/* AUDIOSUBDESC_MIXER */
struct AudioMixerDsc
{
	struct AudioDscHead Head;
	uint8 bUnitId;
	uint8 bNrInPins;
	uint8 baSrcId1;
};

/* AUDIODESC_INTERFACE for AUDIO_SUBCLASS_AUDIOSTREAMING subclas */
struct AudioStreamIfcDsc
{
	struct AudioDscHead Head;
	uint8 bTerminalLink;
	uint8 bmControls;
	uint8 bFormatType;
	uint32 bmFormats;
	uint8 bNrChannels;
	uint32 bmChannelConfig;
	uint8 iChannelNames;
};

/* Class-specific Isochronous Audio Data Endpoint Descriptor */
struct AudioDataEndpointDsc
{
	struct AudioDscHead Head;
	uint8 bmAttributes;
	uint8 bLockDelayUnits;
	uint8 wLockDelay;
};

// Sample frequency descriptor
struct SamFreq
{
	uint8 bSamFreqLow;
	uint8 bSamFreqMid;
	uint8 bSamFreqHi;
};

// A note on the next three audio type descriptors:
//		If 0 == SamFreqType, Two Sam Frequencies will follow,
//		Which describe the Lower Frequency Limit and the
//		Upper frequency limit of the continuous frequency range.
//		If 0 < SamFreqType, SamFreqType will be the count of
//		available discrete frequency entries in the following table.

/* Class-specific Type I Format Type Descriptor */
struct AudioDataTIFormatDsc
{
	struct AudioDscHead Head;
	uint8 bFormatType;	// FORMAT_TYPE_I
	uint8 bSubslotSize;	// 1,2,3,4 bytes per subslot
	uint8 bBitResolution;
};

/* Class-specific Type II Format Type Descriptor */
struct AudioDataTIIFormatDsc
{
	struct AudioDscHead Head;
	uint8 bFormatType;	// FORMAT_TYPE_II
	uint16 wMaxBitRate;
	uint16 wSlotsPerFrame;
};

/* Class-specific Type III Format Type Descriptor */
	// amazingly similar to Type I
struct AudioDataTIIIFormatDsc
{
	struct AudioDscHead Head;
	uint8 bFormatType;		// FORMAT_TYPE_III
	uint8 bSubslotSize;		// 2
	uint8 bBitResolution;	// 16
};

/****************************************************************************/

#ifdef __GNUC__
   #ifdef __PPC__
    #pragma pack()
   #endif
#elif defined(__VBCC__)
   #pragma default-align
#endif

#ifdef __cplusplus
}
#endif

/****************************************************************************/


#endif
