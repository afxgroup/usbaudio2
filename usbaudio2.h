//uachild.h
#ifndef USBAUDIO2_H
#define USBAUDIO2_H

struct AudioDevice *freeAudioDev(struct AudioDevice *adev, CONST_STRPTR reason, ...);

struct AudioDevice *allocAudioDev(struct UsbRawInterface *raw, 
	int32 direction, int32 alternate);

// returns number of allocated requests
uint32 allocRequests(struct AudioDevice *aud);

// always returns zero
uint32 freeRequests(struct AudioDevice *aud, CONST_STRPTR why, ...);

// watch the audio, record the highest magnitude in each channel
// when "send" transmit and reset peaks.
void getPeakLevels(struct AudioDevice *aud, APTR data, uint32 length, BOOL send);

/**
 * Find a given type of descriptor.
 *
 * @param dsc
 * @param type
 * @return
 */
struct USBBusDscHead *findDesc(struct USBBusDscHead *dsc, int type);

/**
 * Find a given type of descriptor.
 *	by type and subtype
 * @param dsc
 * @param type
 * @return
 */
struct AudioDscHead *findSubdesc(struct USBBusDscHead *dsc, uint8 type, uint8 subtype);

/**
 * Locates an EndPoint based on EndPoint requirements.
 *
 * @param dsclist Pointer to first USB Descriptor to check for matching EndPoint
  * @param ttype TransferType to search for (a USBEPTT_xxx constant)
 * @param dir Direction of EndPoint (a USBEPADR_DIR_xxx constant)
 * @param etype endpoiont usage type (a USBEP_ISOUSAGE_xxxx constant)
 * @return
 */
struct USBBusEPDsc *findEpDesc(
	struct USBBusDscHead *dsc, 
	uint8 ttype, 
	uint8 dir, 
	uint8 etype, 
	uint8 skip);

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

// Convert incoming USB audio data into the user requested 
//		number of channels and sample width.
//	write the result to given file handle
int32 writeAudioChannels(BPTR file,					// to
			uint8 *data,									// from
			uint32 srcSize, 								// from size in bytes
			uint32 srcChans,		 						// from channels
			uint32 srcWidth,								// from sample width, in bits
			uint32 destChans, 							// to channels
			uint32 destWidth);							// to sample width, in bits


// zero a number of bytes, not optimized
uint32 zeroMem(uint8 *data, uint32 destSize);

// Convert outgoing audio data to USB audio data 
//	given number of channels and sample width.
//	write the result to given data buffer
	// for each dest frame, 
	// we want to copy sourceframe into destframe as many times as will fit.
int32 readAudioChannels(BPTR file,					// from
			uint8 *data,									// to
			uint32 destSize, 								// to size in bytes
			uint32 destChans,		 						// to channels
			uint32 destWidth,								// to sample width, in bits
			uint32 srcChans, 								// from channels
			uint32 srcWidth);								// from sample width, in bits

// request current sample rate
// will store in aud->ad_SampleRate also
// return SampleRate or Zero
// Fully NULL safe
int32 getSampleRate(struct AudioDevice *aud);

// set desired sample rate
// device may adjust to closest available value
// will store in uaud->uai_SampleRate also
// return actual SampleRate or negative for error
// Fully NULL safe
int32 setSampleRate(struct AudioDevice *aud, int32 desired);

#endif

