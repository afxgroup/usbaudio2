// usb_audio_test.h
#ifndef UA2_H
#define UA2_H

#include "usbsupport.h"
#include <exec/nodes.h>
#include <intuition/classusr.h>


// this is the minimum.. HCD may require more
//#define MIN_ISO_IOS 4

// always returns NULL
struct usbGadget *freeUsbGadget(struct usbGadget *uGad, CONST_STRPTR why, ...);

// assumes type 2 request (16 bit operands)
struct usbGadget *allocUsbGadgetT2(
	struct UsbAudioInterface *uaud, int32 gadType, Object *gad);


/*****************************************************/

uint32 allocRequests(struct UsbAudioInterface *uaud);
uint32 freeRequests(struct UsbAudioInterface *uaud, CONST_STRPTR why, ...);
void postTimer(struct UsbAudioInterface *uaud, uint32 delayms);

void pause(int32 num);
//static void Flip16(CONST APTR buf, uint32 bytes);
//void recordSound(struct UsbAudioInterface *uaud, char *filename, uint32 size);
//void playSound(struct UsbAudioInterface *uaud, char *filename);
uint8 SetFeature(struct UsbAudioInterface *uaud, uint8 feature, uint8 chan, uint8 unit, uint8 id, uint8 *data, uint8 length);

// always returns FALSE
BOOL releaseAudioInterface(struct UsbAudioInterface *uaud);

// returns transfer_Max, or FALSE on failue
BOOL getAudioInterface(
	struct UsbAudioInterface *uaud,
	int32 inOut,
	int32 alternate,
	int32 index,
	int32 skip);

CONST_STRPTR terminalType(uint16 type);

struct usbRange
{
	struct MinNode ur_Node;
	int32 ur_Min;
	int32 ur_Max;
	int32 ur_Res;
	int32 ur_Steps;
};

struct usbGadget
{
	struct MinNode ug_Node;
	Object *ug_Gadget;
	struct List *ug_RangeList;
	int32 ug_NumRanges;
	int32 ug_totalRange;
	int32 ug_Level;
	int32 ug_Scale;
	int32 ug_Offset;
};
/*
// Play file.. Open and play the given filename
void playFile(
	struct UsbAudioInterface *uaud, 
	struct Play2DeviceWin *dWin,
	char *filename);

// Record file.. Create and record the given filename
// record LEngth seconds, or until Ctrl-C
void recordFile(
	struct UsbAudioInterface *uaud, 
	struct DeviceWin *dWin,
	char *filename);

// convert volume to text form dB, and Printf() with legend
// value is fixed point, bottom 8 bits are fractional part
// top eight bits are signed whole dB.
void printdB(CONST_STRPTR legend, int16 value);


void freeSampleRates(struct UsbAudioInterface *uaud);

// scans the device for a list of rates
// will delete previous results if present
// return TRUE if a rate list is present
// return FALSE on failure
// Fully NULL safe
BOOL getSampleRates(struct UsbAudioInterface *uaud);
int32 getSampleRate(struct UsbAudioInterface *uaud);
int32 setSampleRate(struct UsbAudioInterface *uaud, int32 rate);

// get Mute status
// Fully NULL safe
BOOL getMute(struct UsbAudioInterface *uaud, uint8 chan);

// set Mute status
// Fully NULL safe
BOOL setMute(struct UsbAudioInterface *uaud, uint8 chan, BOOL muted);

int16 getVolume(struct UsbAudioInterface *uaud, uint8 chan);
int16 setVolume(struct UsbAudioInterface *uaud, uint8 chan, int16 volume);

//                                                CUR or RANGE   FEATURE_xx     0=Master     0 10=AudOutId          1 byte true/false
int32 getFeature(struct UsbAudioInterface *uaud, uint8 command, uint8 feature, uint8 chan, uint8 unit, uint8 id, APTR data, uint16 length);
int32 setFeature(struct UsbAudioInterface *uaud, uint8 feature, uint8 chan, uint8 unit, uint8 id, APTR data, uint16 length);

// feed the VU meters
//void sendAudioLevels(struct UsbAudioInterface *uaud, APTR data, uint8 length);
void getPeakLevels(struct UsbAudioInterface *uaud, APTR data, uint32 length, BOOL send);

BOOL makeIcon(CONST_STRPTR name, char **newtooltypes, char *newdeftool);
*/
#endif


