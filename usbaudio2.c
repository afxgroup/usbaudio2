#include <stdarg.h>
#include <stdlib.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/usbsys.h>
#include "actions.h"
#include "audchild.h"
#include "oca.h"
#include "usbsupport.h"
#include "midisupport.h"
#include "vumetersupport.h"
#include "wavesupport.h"
#include "audio2.h"
#include "usbaudio2.h"

int main(int argc, char **argv)
{
   int32 result = openAll(argc, argv);
   if(result != RETURN_OK)
   {
      closeAll("arg processing failed\n");
      return(result);
   }

	// Play is output, record and monitor are input
	int32 direction = Play?USBEPADR_DIR_OUT:USBEPADR_DIR_IN;

	if(Verbose)
	{
		IDOS->Printf("%s %ld: USB direction is %s\n", __FILE__, __LINE__,
			direction==USBEPADR_DIR_OUT?"Play":"Record or Monitor");
	}

	// allows the caller to specify which USB audio device to use.
	// (untested)
   struct UsbRawInterface *rawifc;
   if(RawFkt)
   {
      rawifc = (struct UsbRawInterface *)RawFkt;
      if(Verbose)
      {
         IDOS->Printf("Using provided RawFkt of %p\n", rawifc);
      }
   }
   else
   {	// this code is for audio 2.0 only
      rawifc = scanForRaw(0, direction, 0x20);
      if(Verbose)
      {
         IDOS->Printf("Using first available RawIFC of %p\n", rawifc);
      }
   }

   if(!rawifc)
   {
      return(closeAll("Failed to lock USB audio interface\n"));
   }

   // we have a rawifc, of USB protocol 2, for play, record, or monitor

   struct AudioDevice *audio = allocAudioDev(rawifc, 
		direction, Alternate);
   if (!audio)
   {
      return(closeAll("Failed to open Audio Device\n"));
   }

   if(Monitor)
   {	// watch audio input levels without recording them
      monitor(audio);
   }

   else if(Record)
   {
      record(audio, FileName);
   }

   else if(Play)
   {
      play(audio, FileName);
   }
   
   audio = freeAudioDev(audio, NULL);

   if((NULL==RawFkt)&&(rawifc))
   {	// if we found it, we will free it.
      IUSBSys->USBUnlockInterface(rawifc);
   }

   closeAll(NULL);
   return(RETURN_OK);
}


struct AudioDevice *freeAudioDev(struct AudioDevice *adev, CONST_STRPTR reason, ...)
{
	if(reason)
	{	// Printf style error message
		uint32 args[8], x;
		va_list ap;
		va_start(ap, reason);
		for(x = 0; x < 8; x++)
		{
			args[x] = va_arg(ap, uint32);
		}
		va_end(ap);
		errMessage(reason, args[0], args[1], args[2], args[3], 
			args[4], args[5], args[6], args[7]);
	}

   if(!adev)
   {
      return(NULL);
   }

	freeRequests(adev, NULL);

	if(adev->ad_Ifc)
	{	// release our bandwidth
	   IUSBSys->USBIntSetAltSettingA(OpenIOReq, adev->ad_Ifc, 0, NULL);
	}

	IExec->FreeVec(adev->ad_VUPeaks);

	if(adev->ad_IntDsc)
   {
      IUSBSys->USBFreeDescriptors(&adev->ad_IntDsc->Head);
   }

   if(adev->ad_Ifc)
   {
      IUSBSys->USBDeclaimInterface(adev->ad_Ifc);
   }

   IExec->FreeVec(adev);
   return(NULL);
}

// allocates an AudioDevice structure, and fills it with lots of details
// about the UsbRawInterface audio device you provided.
//		direction is USBEPADR_DIR_OUT to play, or USBEPADR_DIR_IN to record or monitor
//		alternate is 0 to release USB bandwidth, 1 or higher to allocate bandwidth
struct AudioDevice *allocAudioDev(struct UsbRawInterface *raw, int32 direction, int32 alternate)
{
	if(Verbose)
	{
		IDOS->Printf("%s %ld: allocAudioDev(%p, %s, %ld)\n", __FILE__, __LINE__,
			raw, direction==USBEPADR_DIR_OUT?"Play":"Record", alternate);
	}

   struct AudioDevice *adev = IExec->AllocVecTags(
      sizeof(struct AudioDevice),
      AVT_ClearWithValue,0,
      TAG_END);
   if(!adev)
   {
      return(freeAudioDev(adev, "%s %ld:Failed to allocate AudioDevice structure\n", 
			__FILE__, __LINE__));
   }
   adev->ad_RawIfc = raw;
	adev->ad_UsbInOut = direction;

   adev->ad_Ifc = IUSBSys->USBClaimInterface(adev->ad_RawIfc, (APTR)1, NULL);
   if(!adev->ad_Ifc)
   {
      return(freeAudioDev(adev, "%s %ld:Failed to claim audio interface\n", 
			__FILE__, __LINE__));
   }

	// "alternate" chooses from (possibly many) different groups of options.
	// This driver just goes where it's told, usually 1 for active.
	// a smarter driver would look over all available alternates
   IUSBSys->USBIntSetAltSettingA(OpenIOReq, adev->ad_Ifc, alternate, NULL);

	// every time you change alternate, you must release and reload descriptors
   adev->ad_IntDsc = (struct USBBusIntDsc *)IUSBSys->USBIntGetAltSettingA(
      OpenIOReq, adev->ad_Ifc, NULL);
   if(!adev->ad_IntDsc)
   {
      return(freeAudioDev(adev, "%s %ld:Failed to get alternate descriptors\n", 
			__FILE__, __LINE__));
   }


	if(!(adev->ad_IsoEpDsc = findEpDesc(&adev->ad_IntDsc->Head, 
									USBEPTT_ISOCHRONOUS, 
									direction, 
									USBEP_ISOUSAGE_DATA, 0)))
	{
		return(freeAudioDev(adev, "%s %ld:allocAudioDev(%p) failed to findEpDesc(%s)\n",
			__FILE__, __LINE__, adev,
			direction==USBEPADR_DIR_OUT?"USBEPADR_DIR_OUT":"USBEPADR_DIR_IN"));
	}

	if(!(adev->ad_TransferMax = LE_WORD(adev->ad_IsoEpDsc->ed_MaxPacketSize)))
	{
		return(freeAudioDev(adev, "%s %ld:Somehow we picked an ISO endpoint with no data capacity\n", 
			__FILE__, __LINE__));
	}

	if (!(adev->ad_DefaultEp = IUSBSys->USBGetEndPoint(NULL, adev->ad_Ifc, 0)))
	{
		return(freeAudioDev(adev, "%s %ld:allocAudioDev() Couldn't find default control pipe endpoint\n",
			__FILE__, __LINE__));
	}

	if (!(adev->ad_IsoEp = IUSBSys->USBGetEndPoint(NULL, adev->ad_Ifc, adev->ad_IsoEpDsc->ed_Address)))
	{
		return(freeAudioDev(adev,"%s %ld:allocAudioDev() USBGetEndPoint(%p,%p,%ld)\n",
			__FILE__, __LINE__, NULL, adev->ad_Ifc, adev->ad_IsoEpDsc->ed_Address));
	}

	struct USBBusDscHead *desc;
	if(!(desc = findDesc(&adev->ad_IntDsc->Head, USBDESC_INTERFACE)))
	{
		return(freeAudioDev(adev,"%s %ld:allocAudioDev() findDesc(%p,%s)\n",
			__FILE__, __LINE__, NULL, adev->ad_IntDsc->Head, "USBDESC_INTERFACE"));
	}
	struct USBBusIntDsc *ifc = (struct USBBusIntDsc *)desc;
	adev->ad_AudioVersion = ifc->id_Protocol;

	struct AudioDscHead *audesc;
	if(!(audesc = findSubdesc(&adev->ad_IntDsc->Head, AUDIODESC_INTERFACE, AUDIOSTREAMSUBSEC_GENERAL)))
	{
		return(freeAudioDev(adev,"%s %ld:allocAudioDev() findSubdesc(%p, %s, %s)\n",
			__FILE__, __LINE__, NULL, 
			adev->ad_IntDsc->Head, "AUDIODESC_INTERFACE", "AUDIOSTREAMSUBSEC_GENERAL"));
	}
	struct AudioStreamIfcDsc *general = (struct AudioStreamIfcDsc *)audesc;
	adev->ad_AudioFormat = general->bFormatType;
	adev->ad_ChannelCount = general->bNrChannels;

	if(!(audesc = findSubdesc(&adev->ad_IntDsc->Head, AUDIODESC_INTERFACE, AUDIOSTREAMSUBSEC_FORMATTYPE)))
	{
		return(freeAudioDev(adev,"%s %ld:allocAudioDev() findSubdesc(%p, %s, %s)\n",
			__FILE__, __LINE__, NULL, 
			adev->ad_IntDsc->Head, "AUDIODESC_INTERFACE", "AUDIOSTREAMSUBSEC_FORMATTYPE"));
	}
	struct AudioDataTIFormatDsc *format = (struct AudioDataTIFormatDsc *)audesc;
	adev->ad_SampleWidth = byteWidth(format->bBitResolution) * 8;

	if(!(adev->ad_CfgDsc = (struct USBBusCfgDsc *)
		IUSBSys->USBIntGetConfigurationA(OpenIOReq, adev->ad_Ifc, NULL)))
	{
		return(freeAudioDev(adev, "%s %ld: no Config Descriptor found\n", __FILE__, __LINE__));
	}

	struct AudioDscHead *next;
	if(!(next = findSubdesc(
		(struct USBBusDscHead *)
		adev->ad_CfgDsc, 
		AUDIODESC_INTERFACE, 
		AUDIOSUBDESC_CLOCK_SOURCE)))
	{
		return(freeAudioDev(adev, "%s %ld:allocAudioDev() Couldn't find a clock source\n",__FILE__,__LINE__));
	}
	struct AudioClockSourceDsc *clock = (struct AudioClockSourceDsc *)next;
	adev->ad_ClockId = (int32)clock->bClockId;

	if(!(next = findSubdesc(
		(struct USBBusDscHead *)
		adev->ad_CfgDsc, 
		AUDIODESC_INTERFACE, 
		AUDIOSUBDESC_CLOCK_SELECTOR)))
	{
		return(freeAudioDev(adev, "%s %ld:allocAudioDev() Couldn't find a clock selector\n",__FILE__,__LINE__));
	}
	struct AudioClockSelectorDsc *cksel = (struct AudioClockSelectorDsc *)next;
	adev->ad_ClockSelector = (int32)cksel->bClockId;

	if(Rate)
	{
		setSampleRate(adev, Rate);
	}

	getSampleRate(adev);

	if((next = findSubdesc(
		(struct USBBusDscHead *)
		adev->ad_CfgDsc, 
		AUDIODESC_INTERFACE, 
		AUDIOSUBDESC_FEATURE)))
	{
		adev->ad_FeatureUnit = (struct AudioFeatureDsc *)next;
		adev->ad_FeatureId = (int32)adev->ad_FeatureUnit->bUnitId;
	}
	else if (Verbose)
	{	// not fatal
		IDOS->Printf("Feature unit not found!\n");
	}

	if(!(adev->ad_VUPeaks = IExec->AllocVecTags(
		adev->ad_ChannelCount * sizeof(uint32),
		AVT_ClearWithValue,0,
		TAG_END)))
	{
		return(freeAudioDev(adev, "%s %ld:allocAudioDev() Couldn't allocate peak detector array\n", __FILE__,__LINE__));
	}

   return(adev);
}

// always returns zero
uint32 freeRequests(struct AudioDevice *aud, CONST_STRPTR why, ...)
{
	if(why)
	{	// post any error message provided
		uint32 args[8], x;
		va_list ap;
		va_start(ap, why);
		for(x = 0; x < 8; x++)
		{
			args[x] = va_arg(ap, uint32);
		}
		va_end(ap);
		errMessage(why, args[0], args[1], args[2], args[3], 
			args[4], args[5], args[6], args[7]);
	}

	if(!aud)
	{
		return(0);
	}

	if(aud->ad_UsbIorTable)
	{
		struct USBIOReq *ior;
		uint32 x;
		for(x = 0; x < aud->ad_IorTableLen; x++)
		{
			ior = aud->ad_UsbIorTable[x];
			IExec->AbortIO((struct IORequest *)ior);
			IExec->WaitIO((struct IORequest *)ior);
			IExec->FreeVec(ior->io_Data);
			IUSBSys->USBFreeRequest(ior);
		}

		IExec->FreeVec(aud->ad_UsbIorTable);
		aud->ad_UsbIorTable = NULL;
	}
	aud->ad_IorTableLen = 0;
	return(0);
}

// returns number of allocated requests
uint32 allocRequests(struct AudioDevice *aud)
{
	if(!aud)
	{
		return(0);
	}

	freeRequests(aud, NULL);	// free any existing requests

	struct UsbEndPoint *ep = aud->ad_IsoEp;
	if(!aud->ad_IsoEp)
	{
		return(freeRequests(aud, "No ISO endpoint aud->ad_IsoEp\n"));
	}

	// request the number of minimum USBIORequests
	uint32 cachedIsochronousFrames, 
			maxTransferSize, transfersPerFrame;
	IUSBSys->USBGetEndPointAttrs((struct UsbEndPoint *)ep,
		USBA_HCD_CachedIsochronousFrames, &cachedIsochronousFrames,
		USBA_EP_MaxTransferSize, &maxTransferSize,
		USBA_EP_TransfersPerFrame, &transfersPerFrame, 
		TAG_END);

	if(Verbose)
	{
		IDOS->Printf("At %s:%ld, cachedIsoFrames = %ld\n",__FILE__,__LINE__,cachedIsochronousFrames);
		IDOS->Printf("maxTransferSize = %ld\n", maxTransferSize);
		IDOS->Printf("transfersPerFrame = %ld\n", transfersPerFrame);
	}


	if(aud->ad_UsbInOut == USBEPADR_DIR_OUT)
	{
		uint32 newMax = aud->ad_SampleWidth > 16 ? 4:2;
		newMax *= aud->ad_SampleRate * aud->ad_ChannelCount;
		newMax /= transfersPerFrame * 1000;
		aud->ad_TransferMax = maxTransferSize = newMax;
	}

	if(Verbose)
	{
		IDOS->Printf("MaxTransfer %ld, transfersPerFrame %ld\n",
			maxTransferSize, transfersPerFrame);
	}

	if(MIN_ISO_IOS > cachedIsochronousFrames)
	{	// increase it to MIN if needed
		cachedIsochronousFrames = MIN_ISO_IOS;
	}

	if(MinIor > cachedIsochronousFrames)
	{	// increase it if user requested more
		cachedIsochronousFrames = MinIor;
	}

	aud->ad_TransfersPerFrame = transfersPerFrame;

	// count + 1 of NULL pointers
	aud->ad_UsbIorTable = IExec->AllocVecTags(
		sizeof(struct USBIOReq *) * (cachedIsochronousFrames + 1),
		AVT_ClearWithValue,0,
		TAG_END);

	if(!aud->ad_UsbIorTable)
	{
		return(freeRequests(aud, "Table allocation failure of size %ld\n",
				(sizeof(struct USBIOReq *) * (cachedIsochronousFrames + 1))));
	}

	uint32 x;
	for(x = 0; x < cachedIsochronousFrames; x++)
	{	// for each request...

			// allocate the USBIOReq
		aud->ad_UsbIorTable[x] =
			IUSBSys->USBAllocRequest(OpenIOReq,
				TAG_END);

		if(!aud->ad_UsbIorTable[x])
		{
			return(freeRequests(aud, "USBIOReq allocation failure\n"));
		}

			// allocate the data buffer
		aud->ad_UsbIorTable[x]->io_Data = 
			IExec->AllocVecTags(
				maxTransferSize * transfersPerFrame,
				AVT_Type, MEMF_SHARED,
				AVT_ClearWithValue, 0,
				TAG_END );
		if(!aud->ad_UsbIorTable[x]->io_Data)
		{
			return(freeRequests(aud, 
					"USBIOReq buffer allocation failure size %ld\n",
					maxTransferSize * transfersPerFrame));
		}

			// setup the frame count and frames
		IUSBSys->USBSetIsoTransferCount(aud->ad_UsbIorTable[x], transfersPerFrame);
		int32 y, max;
		max=(aud->ad_UsbInOut==USBEPADR_DIR_OUT)?aud->ad_TransferMax:maxTransferSize;
		for(y = 0; y < transfersPerFrame; y++)
		{
			IUSBSys->USBSetIsoTransferSetup(aud->ad_UsbIorTable[x], 
				y, y * max, max);
		}
	}
	aud->ad_IorTableLen = cachedIsochronousFrames;

	return(cachedIsochronousFrames);
}

int32 repcount = 0;
// watch the audio, record the highest magnitude in each channel
// when "send", transmit and reset peaks.
void getPeakLevels(struct AudioDevice *aud, APTR data, uint32 length, BOOL send)
{
	uint32 *peaks = aud->ad_VUPeaks;
	uint32 index = 0, sampleCount, x;
	struct MidiLink *outLink = CamdVULink;

	if(!peaks)
	{
		IDOS->Printf("NULL uaud->uai_VUPeaks\n");
		return;
	}

	if(send)
	{// send off the peaks, then reset them all to zero
		for(x = 0; x < aud->ad_ChannelCount; x++)
		{	// for every channel
								// high, sample width,                      note,  link
			senddBDown(aud, peaks[x], aud->ad_SampleWidth > 16?32:16, x + 1, outLink);
			peaks[x] = 0;	// then reset high
		}
	}

	// check every sample, record the peak absolute levels
	if(aud->ad_SampleWidth > 16)
	{	// 32 bit container width
		sampleCount = length / sizeof(int32);
		int32 value, *sample = (int32 *)data;
		int32 channel;

		while(index < sampleCount)
		{
			channel = index % aud->ad_ChannelCount;
			value = *sample++;
			value = LE_LONG(value);
			value = abs(value);
			if(value > peaks[channel])
			{
				peaks[channel] = value;
			}
			index++;
		}
	}
}

/**
 * Find a given type of descriptor.
 *
 * @param dsc
 * @param type
 * @return
 */
struct USBBusDscHead *findDesc(struct USBBusDscHead *dsc, int type)
{
	while (dsc)
	{
        if (dsc->dh_Type == type)
        	return dsc;

        dsc = IUSBSys->USBNextDescriptor( dsc );
	}
	return NULL;
}

/**
 * Find a given type of descriptor.
 *	by type and subtype
 * @param dsc
 * @param type
 * @return
 */
struct AudioDscHead *findSubdesc(struct USBBusDscHead *udsc, uint8 type, uint8 subtype)
{
	struct AudioDscHead *dsc = (struct AudioDscHead *)udsc;
	while (udsc)
	{
		dsc = (struct AudioDscHead *)
			IUSBSys->USBNextDescriptor((struct USBBusDscHead *) dsc );

		if(!dsc)
		{
			return(NULL);
		}

		if((dsc->Head.dh_Type == type)&&(dsc->subType == subtype))
		{
			return dsc;
		}
	}
	return NULL;
}

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
	uint8 skip)
{

    struct USBBusEPDsc   *epd;

    while (dsc)
    {

        if (dsc->dh_Type == USBDESC_ENDPOINT)
        {
            epd = (struct USBBusEPDsc *)dsc;

            if (((epd->ed_Address & USBEPADRM_DIRECTION) == dir) &&
                ((epd->ed_Attributes & USBEPATRM_TRANSFERTYPE) == ttype ) &&
					 ((epd->ed_Attributes & USBEPATRM_USAGETYPE) == etype) ) // Properties match
            {
					if(0 == skip)
					{
	            	break; // We've got a matching EndPoint
					}
					skip--;
            }
        }

        dsc = IUSBSys->USBNextDescriptor(dsc);
    }

    return (struct USBBusEPDsc *)dsc;
}

// find nth unclaimed USB audio interfaces
// of specified direction and protocol
// return USBRawInterface
// parameters:
	// skip: how many unclaimed matching interfaces to skip, normally 0
	// direction: either USBEPADR_DIR_IN to record audio 
	//						or USBEPADR_DIR_OUT to play audio
	// protocol: The USB Audio revision level of this driver, 20
// NOTE: Caller must eventually call IUSBSys->USBUnlockInterface()
// on the returned RAW interface
struct UsbRawInterface *scanForRaw(int32 skip, int32 direction, int32 protocol)
{
	struct UsbRawInterface *rawifc, *oldrawifc = NULL, *match = NULL;
	struct UsbInterface *ifc = NULL;
	struct USBBusIntDsc *intDsc;

	while((rawifc = (struct UsbRawInterface *)IUSBSys->USBFindInterface(
		oldrawifc,
		USBA_Class, USBCLASS_AUDIO,
		USBA_Subclass, AUDIO_SUBCLASS_AUDIOSTREAMING,
		TAG_END)))
	{
		if(oldrawifc)
		{
			IUSBSys->USBUnlockInterface(oldrawifc);
		}
		oldrawifc = rawifc;

		// we need to verify direction and protocol here..
		// we must claim it to see the details..
		if((ifc = IUSBSys->USBClaimInterface(rawifc, (APTR)1, NULL)))
		{
			IUSBSys->USBIntSetAltSettingA(OpenIOReq, ifc, 1, NULL);
			if((intDsc = (struct USBBusIntDsc *)IUSBSys->USBIntGetAltSettingA(OpenIOReq, ifc, NULL)))
			{
				if(protocol == intDsc->id_Protocol)
				{
					// we are the correct protocol level
					// now we check the direction
					if(findEpDesc(&intDsc->Head, USBEPTT_ISOCHRONOUS,
						direction, USBEP_ISOUSAGE_DATA,0))
					{
						match = rawifc;
						if(Verbose)
						{
							IDOS->Printf("Found a matching USBRawInterface\n");
						}
					}

				}

				IUSBSys->USBFreeDescriptors((struct USBBusDscHead *)intDsc);
			}

			IUSBSys->USBDeclaimInterface(ifc);
			ifc = NULL;
		}


		if(!match)
		{	// keep looking..
			continue;
		}

		// we found an unclaimed raw interface of the suitable Class, 
		//		Subclass, Direction and protocol
		// do we skip it or take it?
		if(0 == skip)
		{
			return(match);
		}

		skip--;

	}

	// if we get this far the search has failed, so clean up the last lock
	if(oldrawifc)
	{
		IUSBSys->USBUnlockInterface(oldrawifc);
		oldrawifc = NULL;
	}

	return(NULL);
}

// Convert incoming USB audio data into the user requested 
//		number of channels and sample width.
//	write the result to given file handle
int32 writeAudioChannels(BPTR file,					// to
			uint8 *data,									// from
			uint32 srcSize, 								// from size in bytes
			uint32 srcChans,		 						// from channels
			uint32 srcWidth,								// from sample width, in bits
			uint32 destChans, 							// to channels
			uint32 destWidth)								// to sample width, in bits
{
	if((srcChans == destChans)&&(srcWidth == destWidth))
	{	// speed copy for optimized saves
		return(IDOS->FWrite(file, data, 1, srcSize));
	}

	uint8 *endOfData = data + srcSize;

	int32 written = 0;
	int32 srcFrame = srcChans * srcWidth / 8;	// one source frame
	int32 destFrame = destChans * destWidth / 8; // one dest frame
	while(data < endOfData)
	{
		written += IDOS->FWrite(file, data, 1, destFrame);
		data += srcFrame;
	}

	return(written);
}

// zero a number of bytes, not optimized
uint32 zeroMem(uint8 *data, uint32 destSize)
{
	uint32 size = destSize;
	while(size--)
	{
		*data++ = 0;
	}
	return(destSize);
}


// Convert outgoing audio data to USB audio data 
//	given number of channels and sample width.
//	write the result to given data buffer
	// for each dest frame, 
	// we want to copy sourceframe into destframe as many times as will fit.
// return the byte size of returned data buffer (used)
int32 readAudioChannels(BPTR file,					// from
			uint8 *data,									// to
			uint32 destSize, 								// to size in bytes
			uint32 destChans,		 						// to channels
			uint32 destWidth,								// to sample width, in bits
			uint32 srcChans, 								// from channels
			uint32 srcWidth)								// from sample width, in bits
{

	if((srcChans == destChans)&&(srcWidth == destWidth))
	{	// speed copy for optimized loading
		return(IDOS->FRead(file, data, 1, destSize));
	}

	int32 totalRead = 0, lastRead = 1;	// start while() bit, not counted
	int32 srcFrameSize = srcChans * srcWidth / 8;	// one source frame // 8
	int32 destFrameSize = destChans * destWidth / 8; // one dest frame // 16
	int32 destFrameCount = destSize / destFrameSize;	// 200
	int32 copied = 0;

	while(lastRead && destFrameCount)
	{	// when FRead() has returned zero, or dest size is filled, we are done
		if((lastRead = IDOS->FRead(file, data, 1, srcFrameSize)))	// get one src frame
		{
			copied = lastRead;
			zeroMem(data + copied, destFrameSize);	// erase extra part of frame

			if(FillOut)
			{
				// copy src into dest as many times as it fits.
				// this makes mono files play in stereo, or multi channel playback
				// of 4 or 8 channel output devices
				while(destFrameSize >= srcFrameSize + copied)
				{
					IExec->CopyMem(data, data + copied, srcFrameSize);
					copied += srcFrameSize;
				}
			}

			data += destFrameSize;
			totalRead += destFrameSize;
			destFrameCount--;
		}
	}

	return(totalRead);
}

// request current sample rate
// will store in aud->ad_SampleRate also
// return SampleRate or Zero
// Fully NULL safe
int32 getSampleRate(struct AudioDevice *aud)
{
	// make sure we already have an audio device
	if(!aud)
	{
		return(-10);
	}

	// a buffer to hold the results for processing
	uint32 *buffer = IExec->AllocVecTags(
		8,
		AVT_Type, MEMF_SHARED,
		AVT_ClearWithValue, 0,
		TAG_END);
	if(!buffer)
	{
		IDOS->Printf("Failed to allocate rate buffer\n");
		return(-11);
	}

	int32 err = IUSBSys->USBEPControlXfer(
		OpenIOReq,		// request used to open USBSys
		aud->ad_DefaultEp,		// endpoint to address
		CUR,							// Command
		  USBSDT_DIR_DEVTOHOST	// 1000 0000
		| USBSDT_TYP_CLASS		// 0010 0000
		| USBSDT_REC_INTERFACE,	// 0000 0001
		(CS_SAM_FREQ_CONTROL << 8) | 0,	// Value
		(aud->ad_ClockId   << 8) | aud->ad_IntDsc->id_InterfaceID,	// Index
		buffer,
		4,			// Length
		TAG_END);
	if(0 == err)
	{
		aud->ad_SampleRate = LE_LONG(buffer[0]);
		IExec->FreeVec(buffer);
		return(aud->ad_SampleRate);
	}
	aud->ad_SampleRate = 0;
	IExec->FreeVec(buffer);
	return(-13);
}

// set desired sample rate
// device may adjust to closest available value
// will store in uaud->uai_SampleRate also
// return actual SampleRate or negative for error
// Fully NULL safe
int32 setSampleRate(struct AudioDevice *aud, int32 desired)
{
	// make sure we already have an audio device
	if(!aud)
	{
		IDOS->Printf("No Audio Device\n");
		return(-1);
	}

	// a buffer to hold the results for processing
	uint32 *buffer = IExec->AllocVecTags(
		8,
		AVT_Type, MEMF_SHARED,
		AVT_ClearWithValue, 0,
		TAG_END);
	if(!buffer)
	{
		IDOS->Printf("Failed to allocate rate buffer\n");
		return(-2);
	}

	buffer[0] = LE_LONG(desired);

	int32 err = IUSBSys->USBEPControlXfer(
		OpenIOReq,		// request used to open USBSys
		aud->ad_DefaultEp,		// endpoint to address
		CUR,							// Command
		  USBSDT_DIR_HOSTTODEV	// 0000 0000
		| USBSDT_TYP_CLASS		// 0010 0000
		| USBSDT_REC_INTERFACE,	// 0000 0001
		(CS_SAM_FREQ_CONTROL << 8) | 0,	// Value
		(aud->ad_ClockId   << 8) | aud->ad_IntDsc->id_InterfaceID,	// Index
		buffer,
		4,			// Length
		TAG_END);

	IExec->FreeVec(buffer);
	if(0 == err)
	{
		getSampleRate(aud);
		return(aud->ad_SampleRate);
	}
	return(-4);
}




