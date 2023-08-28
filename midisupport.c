// midisupport.c
// open, close and manage CAMD messages

#include <proto/exec.h>
#include <proto/camd.h>
#include <proto/dos.h>
#include <midi/mididefs.h>

#include "midisupport.h"
#include "storestring.h"
#include "oca.h"

struct CamdIFace *ICamd      = NULL;

struct MidiNode *CamdNode    = NULL;
STRPTR CamdVULinkName        = NULL;
struct MidiLink *CamdVULink  = NULL;
//struct MidiLink *CamdPlayLink = NULL;
int8             MidiSig     = -1;

// name is for link comments and node
// also references global strings:
// "CamdInLinkName", "CamdOutLinkName"
// RETURN_OK or RETURN_FAIL
int32 openMidi(CONST_STRPTR name)
{
	if(!(ICamd = openLibInterface("camd.library", 
		MAJ_REV, "main", 1L)))
	{
		return(closeMidi(NULL));
	}

	MidiSig = IExec->AllocSignal(-1);
	if (-1 == MidiSig)
	{
		return (closeMidi("Could not alloc a signal for CAMD"));
	}

	if(!(CamdNode = ICamd->CreateMidi(
		MIDI_Name, name,
		MIDI_RecvSignal, MidiSig,
		MIDI_MsgQueue, 2048,
		MIDI_SysExSize,1024,
		MIDI_ClientType, CCType_EventProcessor,
		TAG_END)))
	{
		return (closeMidi("Could not Create CamdNode"));
	}

	if(haveString(&CamdVULinkName))
	{
		if(!(CamdVULink = ICamd->AddMidiLink(
			CamdNode, MLTYPE_Sender,
			MLINK_Name, name,
			MLINK_Location, CamdVULinkName, 
			TAG_END)))
		{
			return (closeMidi("Could not Create CamdVULink"));
		}
	}

	return(RETURN_OK);
}

// always RETURN_FAIL
int32 closeMidi(CONST_STRPTR why)
{
	errMessage(why);	// optional, NULL safe

	if (ICamd)
	{
		ICamd->RemoveMidiLink(CamdVULink);
		ICamd->DeleteMidi(CamdNode);
		closeInterface(ICamd);
	}
	IExec->FreeSignal(MidiSig);

	CamdVULink = NULL;
	CamdNode = NULL;
	ICamd = NULL;
	MidiSig = -1;

	return(RETURN_FAIL);	// always
}

// return 0 if MIDI Reset received
/*
int32 handleMidi(struct MidiNode *mNode)
{
	MidiMsg msg;
	int8 channel, status;

	while(ICamd->GetMidi(mNode, &msg))
	{
		channel = msg.mm_Status & 0x0F;	// these only apply to channel messages
		status  = msg.mm_Status & 0xF0;

		switch(ICamd->MidiMsgType(&msg))
		{
			case CMB_Note:			// MS_NoteOn, MS_NoteOff
				break;
			case CMB_Prog:			// MS_Prog
				break;
			case CMB_PitchBend:	// MS_PitchBend
				break;
			case CMB_CtrlMSB:		// CC #0   to 31
				break;
			case CMB_CtrlLSB:		// CC #32  to 63
				break;
			case CMB_CtrlSwitch:	// CC #64  to 79
				break;
			case CMB_CtrlByte:	// CC #81  to 95
				break;
			case CMB_CtrlParam:	// CC #96  to 101
				break;
			case CMB_CtrlUndef:	// CC #103 to 119
				break;
			case CMB_Mode:			// CC #104 to 127
				break;
			case CMB_ChanPress:
				break;
			case CMB_PolyPress:
				break;
			case CMB_RealTime:	// MS_Clock, MS_Tick, MS_Start, 
										// MS_Stop, MS_Continue
										// MS_ActvSense, MS_Reset
				if(MS_Reset == msg.mm_Status)
				{	// normally a "die" message
					return(0);
				}
				break;
			case CMB_SysCom:		// MS_QtrFrams, MS_SongPos,
										// MS_SongSelect, MS_TuneReq,
										// 0xF4, 0xF5, 0xF7
				break;
			case CMB_SysEx:
				break;
		}
	}
	return(1);
}
*/


