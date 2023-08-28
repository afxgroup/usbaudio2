// audChild.h

#ifndef AUDCHILD_H
#define AUDCHILD_H

#include <exec/ports.h>
#include <exec/types.h>
#include <usb/system.h>

/*
Parent shall:  USBFindFunction()
               USBLockFunction(rawfkt)
               select the appropriate child 
               start the child, sending the USB Raw Function, and any command details
            Child Shall:
                        usfkt = USBClaimFunction(fkt,ref,msgport)               
                        set the requested parameters
                        do the deed
                        USBDeclaimFunction(usfkt)
                        exit and return to parent.
               on return, the parent will USBUnlockFunction(fkt)

this should allow various children to be added and edited 
independent of the parent program.


On to the gritty details:
record, playback are obvious primary tasks.
Parent specifies max time for either, but MAY break with Ctrl-C
other parameters may include rate, width, channels, volume/gain,

child programs, without any GUI or command line, are well insulated 
from intuition delays and wait states.

command line arguments for the child..
   rawfkt as a hex string        // child will open forst suitable if not provided
   ar_Function text RECORD | PLAY  // must supply exactly ONE
   CHANNELS/N
   RATE/N
   WIDTH/N
   FILE/A/K
   METER/K
*/

// AudRequest.ar_Function defines
#define ARF_UNKNOWN  0
#define ARF_RECORD   1
#define ARF_PLAYBACK 2

struct AudRequest
{
   struct Node *ar_Node;
   // the following are provided by parent (some optional)
   struct USBRawFunction *ar_Rawfkt;   // the target device
   struct AudRequest     *ar_Self;        // required
   uint32 ar_Function;                    // required
   uint32 ar_Channels;                    // default 2 for recording
   uint32 ar_SampleRate;                  // default 48000
   uint32 ar_SampleWidth;                 // default 16
   CONST_STRPTR ar_FileName;              // required
   CONST_STRPTR ar_MeterName;             // optional
   // the following are for child usage// parent may ignore
   struct USBFunction *ar_UsbFkt;         // claimed function
   struct MsgPort *ar_MsgPort;
};


#endif
