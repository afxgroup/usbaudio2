#ifndef USB_SUPPORT_H
#define USB_SUPPORT_H

#include <proto/usbsys.h>
#include <dos/dos.h>
#include <usb/usb.h>

struct IoReqNode
{
	struct Node node;
	struct IORequest *ior;
	uint32 flags;
};

#define IORN_InUse        1L << 0
#define IORN_Opener       1L << 1


struct AudioDevice
{
	struct Node ad_Node;
	struct UsbRawInterface *ad_RawIfc;
	struct UsbInterface *ad_Ifc;
	struct USBBusIntDsc *ad_IntDsc;
	struct USBBusDevDsc *ad_DevDsc;
	struct USBBusCfgDsc *ad_CfgDsc;
	uint32 ad_AudioVersion;
	uint32 ad_TransferMax;
	uint32 ad_TransfersPerFrame;
	uint32 ad_ClockId;
	uint32 ad_ClockSelector;
	uint32 ad_FeatureId;
	uint32 ad_ChannelCount;	// of USB device
	uint32 ad_AudioFormat;	// 1 == LPCM
	uint32 ad_SampleRate;
	uint32 ad_SampleWidth;	// container in bits
	uint32 ad_IoReqCount;
	uint32 ad_IorTableLen;
	uint32 ad_UsbInOut;		// USBEPADR_DIR_IN : USBEPADR_DIR_OUT
	uint32 *ad_VUPeaks;
	struct UsbEndPoint  *ad_DefaultEp;
	struct USBBusEPDsc  *ad_IsoEpDsc;
	struct UsbEndPoint  *ad_IsoEp;
	struct USBIOReq	 **ad_UsbIorTable;
	struct List         *ad_SampleRates;
   struct AudioFeatureDsc *ad_FeatureUnit;
};

struct SampleRate
{
	struct Node node;
	uint32 min;
	uint32 max;
	uint32 res;
};

struct USBBusClockSourceDesc
{
	struct USBBusDscHead	Head;// len 8, type CS_INTERFACE
	uint8 bDescriptorSubtype;	//CLOCK_SOURCE
	uint8 bClockID;				// unique Identifier
	uint8 bmAttributes;
	uint8 bmControls;				
	uint8 bAssocTerminal;		// terminal associated with this
	uint8 iClockSource;			// string index
};

struct USBBusClockSelectorDesc
{
	struct USBBusDscHead	Head;// len 7+p, type CS_INTERFACE
	uint8 bDescriptorSubtype;	//CLOCK_SELECTOR
	uint8 bClockID;				// unique Identifier		offset 3
	uint8 bNrInPins;				//	(p)						offset 4
	uint8 baCSourceID;			// ID of first input....offset 5+(p-1)
	uint8 bmControls;				// 1:0 = clock selector	offset 5+p	
	uint8 iClockSelector;			// string index		offset 6+p
};

#define CLOCK_ATTR_TYPE_EXTERNAL             	0x00
#define CLOCK_ATTR_TYPE_INTERNAL_FIXED 			0x01
#define CLOCK_ATTR_TYPE_INTERNAL_VARIABLE 		0x02
#define CLOCK_ATTR_TYPE_INTERNAL_PROGRAMMABLE 	0x03
#define CLOCK_ATTR_SYNC_TO_SOF 						0x04

#define CLOCK_CONTROLS_FREQUENCY						0x03
#define CLOCK_CONTROLS_VALIDITY						0x0C

struct USBBusStringDesc
{
	struct USBBusDscHead	Head;
	uint16				string[];
};

struct FtrUnitDsc
{
	struct USBBusDscHead	Head;// len 6+(chan+1)*4, type CS_INTERFACE
	uint8  bDescriptorSubtype;	// FEATURE_UNIT
	uint8  bUnitId;
	uint8  bSourceId;
	uint32 bmaControls0;			// MASTER, all channels control
//	uint32 bmaControls1;			// first channel
// ... additional uint32 for each channel
	uint8  iFeature;				// string index
};

//void scanData(struct UsbAudioInterface *uaud);
void scanData(struct AudioDevice *audio);

struct USBBusDscHead *find_desc(struct USBBusDscHead *dsc, int type);
struct AudioDscHead *find_subdesc(struct AudioDscHead *dsc, uint8 type, uint8 subtype);

/**
 * Locates an EndPoint based on EndPoint requirements.
 *
 * @param dsclist Pointer to first USB Descriptor to check for matching EndPoint
 * @param ttype TransferType to search for (a USBEPTT_xxx constant)
 * @param dir Direction of EndPoint (a USBEPADR_DIR_xxx constant)
 * @param etype endpoint usage type (a USBEP_ISOUSAGE_xxxx constant)
 * @param skip skip this number of matching interfaces
 * @return
 */
struct USBBusEPDsc *find_ep_desc(
	struct USBBusDscHead *dsc, 
	uint8 ttype, 
	uint8 dir, 
	uint8 etype,
	uint8 skip);


int32 freeAudio(struct List *lst);

int32 scanForAudio(struct List *play, struct List *record);

// find nth unclaimed USB audio interfaces
// of specified direction and protocol
// return USBRawInterface
// parameters:
	// skip: how many unclaimed matching interfaces to skip, normally 0
	// direction: either USBEPADR_DIR_IN to record audio 
	//						or USBEPADR_DIR_OUT to play audio
	// protocol: The USB Audio revision level of this driver, often 1 or 2
// NOTE: Caller must eventually call IUSBSys->USBUnlockInterface()
// on the returned RAW interface
struct UsbRawInterface *scanForRaw(int32 skip, int32 direction, int32 protocol);

void dump_desc(struct USBBusDscHead *dsc);
STRPTR getProductStr(struct UsbInterface *ifc, struct IORequest *ior);
struct UsbAudioInterface *freeUsbAudioInterface(struct UsbAudioInterface *uaud);
// retrieve struct DevInfo, copy stuff from it
struct UsbAudioInterface *getUsbAudioInterface(
	struct UsbInterface *ifc, 
	struct IORequest *ior);

struct IoReqNode *freeIoNode(
	struct IoReqNode *irn, 
	CONST_STRPTR why);
struct IoReqNode *allocIoNode(struct MsgPort *replyPort);

BOOL getStringData(
	struct UsbAudioInterface *uaud);

void printToolTypes(struct UsbAudioInterface *uaud);

int32 storeStringDesc(
	STRPTR *dest,
	STRPTR header,
	uint32 index,
	struct UsbInterface *ifc, 
	struct IORequest *ior);


// always returns NULL
struct UsbAudioInterface *freeUaud(struct UsbAudioInterface *uaud, CONST_STRPTR reason, ...);

// ifc should already be locked
// inOut = USBEPADR_DIR_IN/OUT
// alternate=0 for no bacndwidth (off) 1+ for usage
struct UsbAudioInterface *buildUaud(struct UsbInterface *ifc, int32 inOut, int32 alternate);


#endif

