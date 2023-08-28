// actions.c
// the things we can do

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/timer.h>
#include "actions.h"
#include "oca.h"
#include "usbsupport.h"
#include "midisupport.h"
#include "vumetersupport.h"
#include "wavesupport.h"
#include "usbaudio2.h"

int32 dummy;

int32 play(struct AudioDevice *aud, CONST_STRPTR filename)
{
	if(Verbose)
	{
	   IDOS->Printf("%s %ld: calling play(%p, %s);\n", __FILE__, __LINE__, aud, filename);
	}

	struct WaveFile *wave = loadWave(FileName, WAVE_FROM_DISK);
	if(!wave)
	{
		IDOS->Printf("%s %ld: Failed to load wave file %s\n", __FILE__, __LINE__, FileName);
	}

	if(!Channels)
	{
		Channels = wave->wf_ChannelCount;
	}

	if(!Width)
	{
		Width = wave->wf_SampleWidth;
	}

	Rate = wave->wf_SampleRate;

	if(Verbose)
	{
		IDOS->Printf("\t%ld Channels\n", Channels);
		IDOS->Printf("\t%ld bit samples\n", Width);
		IDOS->Printf("\tplaying at %ld samples per second\n", Rate);
	}


	// all the signals we care about
   uint32 signals, signalmask = SIGBREAKF_CTRL_C;
   uint32 usbsignal = 1L << UsbPort->mp_SigBit;
   signalmask |= usbsignal;
   uint32 timersignal = 1L << TimerPort->mp_SigBit;
   signalmask |= timersignal;

   BOOL alive = TRUE;
	uint64 endTime;
	uint32 e_Freq;

	if(Seconds)
	{	// specified duration from command line?
		e_Freq = ITimer->ReadEClock((struct EClockVal *)&endTime);
		endTime += (Seconds * e_Freq);
	}
	postTimer(aud, 50);

   // create our IORequests
	if(!(aud->ad_IoReqCount = allocRequests(aud)))
	{
      IDOS->Printf("%s %ld:failed allocRequests(%p)\n", __FILE__, __LINE__, aud);
		return(-1);
	}

	if(Verbose)
	{
		IDOS->Printf("We are running %ld USBIORequests\n", 
			aud->ad_IoReqCount);
	}

	int32 x;

	for(x = 0; x < aud->ad_IoReqCount; x++)
	{	// setup and launch the first round of IORequests
		aud->ad_UsbIorTable[x]->io_Command = CMD_WRITE;
		aud->ad_UsbIorTable[x]->io_EndPoint = aud->ad_IsoEp;
		aud->ad_UsbIorTable[x]->io_Length = 
			aud->ad_TransferMax * aud->ad_TransfersPerFrame;

		IExec->SendIO((struct IORequest*)aud->ad_UsbIorTable[x]);
	}

	if(Verbose)
	{
		IDOS->Printf("All %ld requests have been sent\n", aud->ad_IoReqCount);
	}

	struct USBIOReq *ureq;
	while(alive)
   {
      signals = IExec->Wait(signalmask);

      if(signals & usbsignal)
		{	// need more audio
			while((ureq = (struct USBIOReq *)IExec->GetMsg(UsbPort)))
			{

				if(ureq->io_Error != USBERR_NOERROR)
				{	// handle errors..
					IDOS->Printf("IO Error %ld",ureq->io_Error);
					if(ureq->io_Error==-37)
					{
						IDOS->Printf(" Missed Frame");
					}
					if(ureq->io_Error==-38)
					{
						IDOS->Printf(" Check Isochronous Frames");
					}
					IDOS->Printf("\n");
					ureq->io_Error = USBERR_NOERROR;
				}


				uint8 *dta = (uint8 *)ureq->io_Data;
				struct USBTransferResult *ures;
				for(x = 0; x < ureq->io_Actual; x++)
				{
					ures = IUSBSys->USBGetIsoTransferResult(
						ureq, x, NULL);

					if(CamdVULink)
					{	// calc peak levels
						getPeakLevels(aud, 
							&dta[x * aud->ad_TransferMax],
							ures->Actual, 
							FALSE);
					}

				}

					// we need a channel slice and dice routine..
					alive = readAudioChannels(wave->wf_FileHandle,		// from
						ureq->io_Data,									// to
						aud->ad_TransferMax * aud->ad_TransfersPerFrame,	// to size in bytes
						aud->ad_ChannelCount, 						// to channels
						aud->ad_SampleWidth,							// to sample width
						Channels, 										// from channels
						Width);											// from channel width

				// send the request back out for more
				ureq->io_Command = CMD_WRITE;
				ureq->io_EndPoint = aud->ad_IsoEp;
				ureq->io_Length = alive;
				ureq->io_Actual = aud->ad_TransfersPerFrame;
				IExec->SendIO((struct IORequest*)ureq);

			}
		}

		if(signals & timersignal)
		{
			IExec->WaitIO(&TimerIoReq->Request);
			postTimer(aud, 50);

			if(Seconds)
			{	// if waiting for a timeout
				uint64 timeNow;
				ITimer->ReadEClock((struct EClockVal *)&timeNow);
				if(timeNow > endTime)
				{
					alive = FALSE;
					IDOS->Printf("%s %ld: %s Timeout\n", __FILE__, __LINE__, ProgramName);
				}
			}

			if(CamdVULink)
			{	// update the meters from PeakData
				getPeakLevels(aud, 
					NULL, 0, TRUE);
			}
		
		}

      if(signals & SIGBREAKF_CTRL_C)
      {	// User or System request for us to quit.
         IDOS->Printf("%s %ld: %s BREAK\n", __FILE__, __LINE__, ProgramName);
         alive = FALSE;
      }
   }

	wave = freeWave(wave, NULL);

	freeRequests(aud, NULL);
   return(0);
}

// gets default parameters from recording device, then overrides with
// command line options if present
int32 record(struct AudioDevice *aud, CONST_STRPTR filename)
{
	if(Verbose)
	{
	   IDOS->Printf("%s %ld: calling record(%p, %s);\n", __FILE__, __LINE__, aud, filename);
	}

	struct WaveFile *wave = newWave(FileName);
	if(!wave)
	{
		IDOS->Printf("failed to create wave file at %s\n", __FILE__, __LINE__, FileName);
	}

	if(!Channels)
	{
		Channels = aud->ad_ChannelCount;
	}

	if(!Width)
	{
		Width = aud->ad_SampleWidth;
	}

	Rate = aud->ad_SampleRate;	// should already be as close as possible


	// all the signals we care about
   uint32 signals, signalmask = SIGBREAKF_CTRL_C;
   uint32 usbsignal = 1L << UsbPort->mp_SigBit;
   signalmask |= usbsignal;
   uint32 timersignal = 1L << TimerPort->mp_SigBit;
   signalmask |= timersignal;

   BOOL alive = TRUE;
	uint64 endTime;
	uint32 e_Freq;

	if(Seconds)
	{	// specified duration from command line?
		e_Freq = ITimer->ReadEClock((struct EClockVal *)&endTime);
		endTime += (Seconds * e_Freq);
	}
	postTimer(aud, 50);

   // create our IORequests
	if(!(aud->ad_IoReqCount = allocRequests(aud)))
	{
      IDOS->Printf("%s %ld:failed allocRequests(%p)\n", __FILE__, __LINE__, aud);
		return(-1);
	}

	if(Verbose)
	{
		IDOS->Printf("We are running %ld USBIORequests\n", 
			aud->ad_IoReqCount);
	}

	int32 x;
	for(x = 0; x < aud->ad_IoReqCount; x++)
	{	// setup and launch the first round of IORequests
		aud->ad_UsbIorTable[x]->io_Command = CMD_READ;
		aud->ad_UsbIorTable[x]->io_EndPoint = aud->ad_IsoEp;
		aud->ad_UsbIorTable[x]->io_Length = 
			aud->ad_TransferMax * aud->ad_TransfersPerFrame;

		IExec->SendIO((struct IORequest*)aud->ad_UsbIorTable[x]);
	}

	if(Verbose)
	{
		IDOS->Printf("All %ld requests have been sent\n", aud->ad_IoReqCount);
	}


	struct USBIOReq *ureq;
	while(alive)
   {
      signals = IExec->Wait(signalmask);

      if(signals & usbsignal)
		{	// incoming audio
			while((ureq = (struct USBIOReq *)IExec->GetMsg(UsbPort)))
			{

				if(ureq->io_Error != USBERR_NOERROR)
				{	// handle errors..
					IDOS->Printf("IO Error %ld",ureq->io_Error);
					if(ureq->io_Error==-37)
					{
						IDOS->Printf(" Missed Frame");
					}
					if(ureq->io_Error==-38)
					{
						IDOS->Printf(" Check Isochronous Frames");
					}
					IDOS->Printf("\n");
					ureq->io_Error = USBERR_NOERROR;
				}

				uint8 *dta = (uint8 *)ureq->io_Data;
				struct USBTransferResult *ures;
				for(x = 0; x < ureq->io_Actual; x++)
				{
					ures = IUSBSys->USBGetIsoTransferResult(
						ureq, x, NULL);
					if(CamdVULink)
					{	// calc peak levels
						getPeakLevels(aud, 
							&dta[x * aud->ad_TransferMax],
							ures->Actual, 
							FALSE);
					}

					// we need a channel slice and dice routine..
					writeAudioChannels(wave->wf_FileHandle,	// to
						&dta[x * aud->ad_TransferMax],			// from
						ures->Actual, 									// from size in bytes
						aud->ad_ChannelCount, 						// from channels
						aud->ad_SampleWidth,							// from sample width
						Channels, 										// to channels
						Width);											// to channel width
				}

				// send the request back out for more
				ureq->io_Command = CMD_READ;
				ureq->io_EndPoint = aud->ad_IsoEp;
				ureq->io_Length =
					aud->ad_TransferMax * aud->ad_TransfersPerFrame;
				IExec->SendIO((struct IORequest*)ureq);

			}
		}

		if(signals & timersignal)
		{
			IExec->WaitIO(&TimerIoReq->Request);
			postTimer(aud, 50);

			if(Seconds)
			{	// if waiting for a timeout
				uint64 timeNow;
				ITimer->ReadEClock((struct EClockVal *)&timeNow);
				if(timeNow > endTime)
				{
					alive = FALSE;
					IDOS->Printf("%s %ld: %s Timeout\n", __FILE__, __LINE__, ProgramName);
				}
			}

			if(CamdVULink)
			{	// update the meters from PeakData
				getPeakLevels(aud, 
					NULL, 0, TRUE);
			}
		
		}

      if(signals & SIGBREAKF_CTRL_C)
      {	// User or System request for us to quit.
         IDOS->Printf("%s %ld: %s BREAK\n", __FILE__, __LINE__, ProgramName);
         alive = FALSE;
      }
   }

	if(!aud->ad_SampleRate)
	{
		IDOS->Printf("No sample rate provided\n");
	}

	wave->wf_AudioFormat  = 1;
	wave->wf_SampleRate   = Rate;
	wave->wf_ChannelCount = Channels;
	wave->wf_SampleWidth  = Width;
	finishWave(wave);
	wave = freeWave(wave, NULL);

	freeRequests(aud, NULL);

   return(0);
}

int32 monitor(struct AudioDevice *aud)
{
	if(Verbose)
	{
	   IDOS->Printf("%s %ld: calling monitor(%p);\n", __FILE__, __LINE__, aud);
	}

	// all the signals we care about
   uint32 signals, signalmask = SIGBREAKF_CTRL_C;
   uint32 usbsignal = 1L << UsbPort->mp_SigBit;
   signalmask |= usbsignal;
   uint32 timersignal = 1L << TimerPort->mp_SigBit;
   signalmask |= timersignal;

   BOOL alive = TRUE;
	uint64 endTime;
	uint32 e_Freq;

	if(Seconds)
	{	// specified duration from command line?
		e_Freq = ITimer->ReadEClock((struct EClockVal *)&endTime);
		endTime += (Seconds * e_Freq);
	}
	postTimer(aud, 50);

   // create our IORequests
	if(!(aud->ad_IoReqCount = allocRequests(aud)))
	{
      IDOS->Printf("%s %ld:failed allocRequests(%p)\n", __FILE__, __LINE__, aud);
		return(-1);
	}

	if(Verbose)
	{
		IDOS->Printf("We are running %ld USBIORequests\n", 
			aud->ad_IoReqCount);
	}

	int32 x;
	for(x = 0; x < aud->ad_IoReqCount; x++)
	{	// setup and launch the first round of IORequests
		aud->ad_UsbIorTable[x]->io_Command = CMD_READ;
		aud->ad_UsbIorTable[x]->io_EndPoint = aud->ad_IsoEp;
		aud->ad_UsbIorTable[x]->io_Length = 
			aud->ad_TransferMax * aud->ad_TransfersPerFrame;

		IExec->SendIO((struct IORequest*)aud->ad_UsbIorTable[x]);
	}

	if(Verbose)
	{
		IDOS->Printf("All %ld requests have been sent\n", aud->ad_IoReqCount);
	}


	struct USBIOReq *ureq;
	while(alive)
   {
      signals = IExec->Wait(signalmask);

      if(signals & usbsignal)
		{	// incoming audio
			while((ureq = (struct USBIOReq *)IExec->GetMsg(UsbPort)))
			{

				if(ureq->io_Error != USBERR_NOERROR)
				{	// handle errors..
					IDOS->Printf("IO Error %ld",ureq->io_Error);
					if(ureq->io_Error==-37)
					{
						IDOS->Printf(" Missed Frame");
					}
					if(ureq->io_Error==-38)
					{
						IDOS->Printf(" Check Isochronous Frames");
					}
					IDOS->Printf("\n");
					ureq->io_Error = USBERR_NOERROR;
				}

				uint8 *dta = (uint8 *)ureq->io_Data;
				struct USBTransferResult *ures;
				for(x = 0; x < ureq->io_Actual; x++)
				{
					ures = IUSBSys->USBGetIsoTransferResult(
						ureq, x, NULL);
					if(CamdVULink)
					{	// calc peak levels
						getPeakLevels(aud, 
							&dta[x * aud->ad_TransferMax],
							ures->Actual, 
							FALSE);
					}
				}

				// send the request back out for more
				ureq->io_Command = CMD_READ;
				ureq->io_EndPoint = aud->ad_IsoEp;
				ureq->io_Length =
					aud->ad_TransferMax * aud->ad_TransfersPerFrame;
				IExec->SendIO((struct IORequest*)ureq);

			}
		}

		if(signals & timersignal)
		{
			IExec->WaitIO(&TimerIoReq->Request);
			postTimer(aud, 50);

			if(Seconds)
			{	// if waiting for a timeout
				uint64 timeNow;
				ITimer->ReadEClock((struct EClockVal *)&timeNow);
				if(timeNow > endTime)
				{
					alive = FALSE;
					IDOS->Printf("%s %ld: %s Timeout\n", __FILE__, __LINE__, ProgramName);
				}
			}

			if(CamdVULink)
			{	// update the meters from PeakData
				getPeakLevels(aud, 
					NULL, 0, TRUE);
			}
		
		}

      if(signals & SIGBREAKF_CTRL_C)
      {	// User or System request for us to quit.
         IDOS->Printf("%s %ld: %s BREAK\n", __FILE__, __LINE__, ProgramName);
         alive = FALSE;
      }
   }

	freeRequests(aud, NULL);

   return(0);
}

