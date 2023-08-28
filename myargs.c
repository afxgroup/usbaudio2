// myargs.c
// process all command line arguments and tooltypes
// into global variables

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <workbench/startup.h>

#include "oca.h"
#include "storestring.h"
#include "midisupport.h"
#include "usbaudio2_rev.h"

#include <string.h>

// globals
BOOL ShellStart = FALSE;
BOOL WorkbenchStart = FALSE;
STRPTR ProgramName = NULL;

#define TEMPLATE "FILE,\
RECORD/S,\
PLAY/S,\
MONITOR/S,\
SECONDS/N,\
CHANNELS/N,\
RATE/N,\
WIDTH/N,\
RAWFKT/K,\
METER/K,\
MINIOR/K,\
ALTERNATE/N,\
FILLOUT/S,\
HELP/S,\
V=VERBOSE/S"

STRPTR FileName;					// required for record or playback
int32  Record;		// must		// create a new audio recording
int32  Play;		// choose	// play back an audio recording
int32  Monitor;	// ONE		// see input audio levels without recording
int32  Seconds;					// default = forever
int32  Channels;					// default from file or 2
int32  Rate;						// default = 48000
int32  Width;						// default = 16
APTR   RawFkt;						// default = first USB device found, format = HEX
//STRPTR CamdVULinkName;		// from midi_support.c
int32  MinIor;						// minimum USB IORequests to queue
int32  Alternate;					// which USB alternate setting to use, default 1
int32  FillOut;					// on playback, copy source to all output channels
int32  Help;						// Short help text
int32  Verbose;					// lots of details to stdout

int32 readIcons(struct WBStartup *arg_msg);
static int32 readToolTypes(struct WBArg *wbarg);
int32 readCommandLine(int32 argc, STRPTR argv[]);
int32 argCount(CONST_STRPTR templ);
int32 intToolType(char **toolarray, CONST_STRPTR tool, APTR arg);
int32 hexToolType(char **toolarray, CONST_STRPTR tool, APTR arg);
int32 stringToolType(char **toolarray, CONST_STRPTR tool, APTR arg);
int32 boolToolType(char **toolarray, CONST_STRPTR tool, APTR arg);
int32 intArg(int32 *arg, int32 *var);
int32 hexArg(int32 *arg, uint32 *var);
STRPTR stringArg(int32 *arg, STRPTR *var);
int32 boolArg(int32 *arg, int32 *var);

// process args and tooltypes into globals
// RETURN_OK or RETURN_FAIL
int32 myArgs(int argc, char **argv)
{
	int32 retval = RETURN_OK;

	// establish defaults

	FileName = NULL;
	Record = Play = Monitor = FALSE;
	Seconds = 0;
	Channels = 0;
	Rate = 0;
	Width = 0;
	RawFkt = NULL;
//	CamdVuLinkName = NULL;
	MinIor = 8;
	Alternate = 1;
	FillOut = FALSE;
	Help = FALSE;
	Verbose = FALSE;

//	storeString(&CamdVULinkName, (STRPTR)"HD404");

	if(0 == argc)
	{			// started from Workbench
		WorkbenchStart = TRUE;
		retval = readIcons((struct WBStartup *)argv);
	}
	else
	{			// started from command line
		ShellStart = TRUE;
		retval = readCommandLine(argc, argv);
	}

	if(Record + Play + Monitor != 1)
	{
		IDOS->Printf("Must select ONE of \"Record\", \"Play\", or \"Monitor\"\n");
		retval = RETURN_FAIL;
	}
	else if((!Monitor)&&(!haveString(&FileName)))
	{
		IDOS->Printf("\"Record\" and \"Play\" require a FILE name\n");
		retval = RETURN_FAIL;
	}
	else if(RETURN_OK != retval)
	{
		IDOS->Printf("argument parsing failed\n");
	}


	if(Help)
	{
		IDOS->Printf("%s\n", VSTRING);
		IDOS->Printf("%s Help\n", ProgramName);
		IDOS->Printf("Used to record, play back or monitor USB Audio devices.\n");
		IDOS->Printf("FILE: The file name, required for PLAY or RECORD options.\n");
		IDOS->Printf("one of RECORD, PLAY, or MONITOR must be specified.\n");
		IDOS->Printf("\tMONITOR will drive the VU meters with the current audio input.\n");
		IDOS->Printf("Seconds: Time to play/record/monitor, if not specified it will\n");
		IDOS->Printf("\t PLAY until finished, it will PLAY, RECORD or MONITOR until Ctrl-C or BREAK.\n");
		IDOS->Printf("Channels: How many channels to record or monitor. PLAY defaults to the file contents.\n");
		IDOS->Printf("RATE: recording/playback rate, in frames per second. default to 48000 or whatever the playback file says.\n");
		IDOS->Printf("WIDTH: width of samples in bits, default is 16 or whatever the playback file says.\n");
		IDOS->Printf("RawFkt: a hex string pointing to the desired USBRawFunction. This allows parent to specify USB devices.\n");
		IDOS->Printf("VULink: the named Camd Link for your VU Meters, if desired.\n");
		IDOS->Printf("MinIor: The minimum number of USBIORequests to use\n");
		IDOS->Printf("Alternate: Which Alternate USB setting to use, default 1\n");
		IDOS->Printf("FillOut: duplicate playback into multiple output channels.\n");
		IDOS->Printf("HELP: displays this text.\n");
		IDOS->Printf("Verbose: displays details about operation.\n\n");
	}


	if(Verbose)
	{
		IDOS->Printf("%s:run from %s\n%s\n",
			ProgramName, WorkbenchStart?"Workbench":"Command Line", TEMPLATE);
		IDOS->Printf("\tFile: %s\n", haveString(&FileName)?FileName:"<NONE>");
		IDOS->Printf("\tMode:%s\n",Record?"Record":Play?"Play":Monitor?"Monitor":"None");
		IDOS->Printf("\t%ld Seconds\n", Seconds);
		IDOS->Printf("\t%ld Channels\n", Channels);
		IDOS->Printf("\tat %ld samples per second\n", Rate);
		IDOS->Printf("\tusing %ld bit samples\n", Width);
		if(RawFkt)
		{
			IDOS->Printf("\tRawFkt %p\n", RawFkt);
		}
		else
		{
			IDOS->Printf("\tRawFkt [NULL]\n");
		}
		IDOS->Printf("\tVULink %s\n", CamdVULinkName);
		IDOS->Printf("\tMinIor %ld\n", MinIor);
		IDOS->Printf("\tAlternate %ld\n", Alternate);
		IDOS->Printf("\tFillOut = %s\n", FillOut?"TRUE":"FALSE");
		IDOS->Printf("\tHelp = %s\n", Help?"TRUE":"FALSE");
		IDOS->Printf("\tVerbose = %s\n", Verbose?"TRUE":"FALSE");
	}
	
	return(retval);
}

void checkLocalVar(STRPTR progName, STRPTR varName, STRPTR *storage)
{
	char buffer[256], argbuffer[256];

	// load any user preference for progname/VULINK
	IUtility->Strlcpy(buffer, progName, 256);
	IUtility->Strlcat(buffer, "/", 256);
	IUtility->Strlcat(buffer, varName, 256);

	if((0 < IDOS->GetVar(buffer, argbuffer, 256, GVF_GLOBAL_ONLY)))
	{
		storeString(storage, argbuffer);
		IDOS->Printf("Meter Link %s at %s\n", argbuffer, buffer);
	}
}

static int32 readToolTypes(struct WBArg *wbarg)
{
	struct DiskObject *dobj; 
	char **toolarray;

	if((wbarg->wa_Name) && (dobj = IIcon->GetDiskObject(wbarg->wa_Name)))
	{
		storeString(&ProgramName, (STRPTR)IDOS->FilePart(wbarg->wa_Name));

		// load any user preference for VULINK
		checkLocalVar(ProgramName, (STRPTR)"VULINK", &CamdVULinkName);

		toolarray = (char **)dobj->do_ToolTypes;

		stringToolType(toolarray, "File", &FileName);
		boolToolType(toolarray, "Record", &Record);
		boolToolType(toolarray, "Play", &Play);
		boolToolType(toolarray, "Monitor", &Monitor);
		intToolType(toolarray, "Seconds", &Seconds);
		intToolType(toolarray, "Channels", &Channels);
		intToolType(toolarray, "Rate", &Rate);
		intToolType(toolarray, "Width", &Width);
		hexToolType(toolarray, "Rawfkt", &RawFkt);
		stringToolType(toolarray, "Meter", &CamdVULinkName);
		intToolType(toolarray, "MinIor", &MinIor);
		intToolType(toolarray, "Alternate", &Alternate);
		boolToolType(toolarray, "FILLOUT", &FillOut);
		boolToolType(toolarray, "HELP", &Help);
		boolToolType(toolarray, "VERBOSE", &Verbose);
	}
	return(RETURN_OK);
}

int32 readCommandLine(int32 argc, STRPTR argv[])
{
	struct RDArgs *read_args;
	CONST_STRPTR template = TEMPLATE;
	int32 retval = RETURN_FAIL;

	storeString(&ProgramName, (STRPTR)IDOS->FilePart(argv[0]));
	checkLocalVar(ProgramName, (STRPTR)"VULINK", &CamdVULinkName);

	int32 numArg = argCount(TEMPLATE);

	int32 **arguments = IExec->AllocVecTags(
		sizeof(int32 *)*numArg,
		AVT_ClearWithValue,0,
		TAG_END);

	if(!arguments)
	{
		errMessage("Failed to allocate arguments table\n");
		return(RETURN_FAIL);
	}

	if((read_args = IDOS->ReadArgs(template, (int32 *)arguments, NULL)))
	{
		retval = RETURN_OK;

		stringArg(arguments[0], &FileName);
		boolArg(  arguments[1], &Record);
		boolArg(  arguments[2], &Play);
		boolArg(  arguments[3], &Monitor);
		intArg(   arguments[4], &Seconds);
		intArg(   arguments[5], &Channels);
		intArg(   arguments[6], &Rate);
		intArg(   arguments[7], &Width);
		hexArg(   arguments[8], (uint32 *)&RawFkt);
		stringArg(arguments[9], &CamdVULinkName);
		intArg(   arguments[10], &MinIor);
		intArg(   arguments[11], &Alternate);
		boolArg(  arguments[12], &FillOut);
		boolArg(  arguments[13], &Help);
		boolArg(  arguments[14], &Verbose);

		IDOS->FreeArgs(read_args);
	}

	IExec->FreeVec(arguments);

	return(retval);
}

int32 intArg(int32 *arg, int32 *var)
{
	if(arg)
	{
		*var = *arg;
	}
	return(*var);
}

int32 hexArg(int32 *arg, uint32 *var)
{
	if(arg)
	{
		IDOS->HexToLong((CONST_STRPTR)arg, var);
	}
	return(*var);
}

STRPTR stringArg(int32 *arg, STRPTR *var)
{
	storeArg(var, arg);
	return(*var);
}

int32 boolArg(int32 *arg, int32 *var)
{
	if(arg)
	{
		*var = TRUE;
	}
	else
	{
		*var = FALSE;
	}
	return(*var);
}

int32 intToolType(char **toolarray, CONST_STRPTR tool, APTR arg)
{
	int32 retval, *argument = arg;

	if(-1 != IDOS->StrToLong(IIcon->FindToolType(toolarray, tool), &retval))
	{
		*argument = retval;
	}
	return(*argument);
}

int32 hexToolType(char **toolarray, CONST_STRPTR tool, APTR arg)
{
	uint32 retval, *argument = arg;

	if(-1 != IDOS->HexToLong(IIcon->FindToolType(toolarray, tool), &retval))
	{
		*argument = retval;
	}
	return(*argument);
}

int32 stringToolType(char **toolarray, CONST_STRPTR tool, APTR arg)
{
	STRPTR s;

	if((s = IIcon->FindToolType(toolarray, tool)))
	{
		storeString((STRPTR *)arg,s);
	}
	return((int32)arg);
}

int32 boolToolType(char **toolarray, CONST_STRPTR tool, APTR arg)
{
	int32 *barg = (int32 *)arg;
	char *s;
	if((s = IIcon->FindToolType(toolarray, tool)))
	{
		IDOS->Printf("found tooltype %s\n", tool);
		if(   (0 == IUtility->Stricmp(s, "NO"))
			|| (0 == IUtility->Stricmp(s, "OFF"))
			|| (0 == IUtility->Stricmp(s, "FALSE")) )
		{
			*barg = 0;
		}
		else
		{
			*barg = 1;
		}
	}
	return(*barg);
}

int32 readIcons(struct WBStartup *arg_msg)
{
	STRPTR full_name;
	struct WBArg *wb_arg;
	int32 ktr, success;
	int32 maxNameLen = 2048;

	wb_arg = arg_msg->sm_ArgList;

	storeString(&ProgramName, wb_arg->wa_Name);

	full_name = IExec->AllocVecTags(maxNameLen, TAG_END);
	if(NULL == full_name)
	{
		errMessage("Failed to allocate temp filename buffer\n");
		return(RETURN_FAIL);
	}

	for(ktr = 0; ktr < arg_msg->sm_NumArgs; ktr++, wb_arg++)
	{
		tryagain:
		if((BPTR)(NULL) != wb_arg->wa_Lock)
		{
			if(ktr > 0)
			{
				if((success = IDOS->NameFromLock(wb_arg->wa_Lock, full_name, maxNameLen)))
				{
					success = IDOS->AddPart(full_name, wb_arg->wa_Name, maxNameLen);
				}
			}
			if((!success)&&(ERROR_LINE_TOO_LONG == IDOS->IoErr()))
			{
				IExec->FreeVec(full_name);
				maxNameLen *= 2;
				if((full_name = IExec->AllocVecTags(maxNameLen, TAG_END)))
				{
					goto tryagain;
				}
			}

			BPTR old_dir = IDOS->SetCurrentDir(wb_arg->wa_Lock);
			readToolTypes(wb_arg);
			IDOS->SetCurrentDir(old_dir);
		}
	}

	IExec->FreeVec(full_name);

	return(RETURN_OK);
}

// count the number of template elements
int32 argCount(CONST_STRPTR templ)
{
	int32 counter = 0;
	char *ret = (char *)templ;

	if((ret)&&(strlen(ret)))
	{
		counter++;
		while((ret = strchr(ret,',')))
		{
			ret++;
			counter++;
		}
	}
	return(counter);
}





