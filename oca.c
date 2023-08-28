// oca.c
// Open Close All
// Inits globals, opens classes and devices
// reads command arguments and tooltypes
// Closes everything when needed.

#include <stdarg.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/icon.h>
#include <proto/utility.h>
#include <proto/application.h>

#include "oca.h"
#include "myargs.h"
#include "usbsupport.h"
#include "midisupport.h"
#include "storestring.h"

	// here's all the global stuff
	// all inits as NULL so we can tell if we got there yet.

struct IconIFace      *IIcon      = NULL;
struct IntuitionIFace *IIntuition = NULL;
struct UtilityIFace   *IUtility   = NULL;
struct ExpansionIFace *IExpansion = NULL;

struct MsgPort        *UsbPort    = NULL;
struct MsgPort        *UsbNotifyPort = NULL;
struct Library         *USBSysBase = NULL;
struct USBSysIFace    *IUSBSys    = NULL;
struct IORequest      *OpenIOReq  = NULL; 

struct TimeRequest    *TimerIoReq = NULL;
struct MsgPort        *TimerPort  = NULL;
struct Device         *TimerBase  = NULL;
struct TimerIFace     *ITimer     = NULL;
BOOL TimerPosted = FALSE;

/*
 *	Opens all libraries, classes, devices
 *   reads all args and tooltypes
 *   returns RETURN_OK, else RETURN_FAIL
 */
BOOL openAll(int argc, char **argv)
{

	if(	// big list of stuff we require
		
		(!(IIcon = openLibInterface("icon.library", 
			MAJ_REV, "main", 1L))) 									||

		(!(IUtility = openLibInterface("utility.library", 
			MAJ_REV, "main", 1L)))									||

		(!(IExpansion = openLibInterface("expansion.library", 
			MAJ_REV, "main", 1L)))									||

		(!(IIntuition = openLibInterface("intuition.library", 
			MAJ_REV, "main", 1L)))									||

 		((!(UsbPort = IExec->AllocSysObjectTags(
 			ASOT_PORT, TAG_END)))
 			&&(errMessage("Failed to allocate UsbPort")))	||

		((!(OpenIOReq = (struct IORequest*)
			IExec->AllocSysObjectTags(
			ASOT_IOREQUEST, 
    		ASOIOR_ReplyPort, UsbPort, 
    		ASOIOR_Size, sizeof(struct IORequest),
    		TAG_END)))&&
    		(errMessage("Failed to allocate usb_ioreq")))	||

		(!(IUSBSys = openDevInterface(
			"usbsys.device", 0, 
			OpenIOReq, 0, "main", 1L)))				         ||

    	(!(USBSysBase = (struct Library *)
    		OpenIOReq->io_Device)) 									||

		((!(UsbNotifyPort = 
			IExec->AllocSysObjectTags(
			ASOT_PORT, TAG_END)))&&
			(errMessage("Failed to allocate usb_notify_port"))) ||

		((!(TimerPort = 
			IExec->AllocSysObjectTags(
			ASOT_PORT, TAG_END)))&&
			(errMessage("Failed to allocate timer_port")))	||

		((!(TimerIoReq = (struct TimeRequest*)
			IExec->AllocSysObjectTags(
			ASOT_IOREQUEST, 
    		ASOIOR_ReplyPort, TimerPort, 
    		ASOIOR_Size, sizeof(struct TimeRequest),
    		TAG_END)))&&
    		(errMessage("Failed to allocate timer_ioreq")))	||

		(!(ITimer = openDevInterface(
			"timer.device", UNIT_WAITECLOCK, 
			&TimerIoReq->Request, 0, 
			"main", 1L)))												||

		(!(TimerBase = 
			TimerIoReq->Request.io_Device))  	)
	{
		return(closeAll("oca.c Gang Failure\n"));
	}

	if(myArgs(argc, argv))
	{
		closeAll(NULL);
		return(RETURN_FAIL);
	}

	if((haveString(&CamdVULinkName)) 
	 && (RETURN_OK != openMidi(ProgramName)) )
	{
		return(closeAll("openMidi() failure\n"));
	}

	if(Verbose)
	{
		IDOS->Printf("openAll() returns successfully\n");
	}

	return(RETURN_OK);	// success!
}

// note: closeAll calls may be nested, 
//  so NULL what you have returned
// This function ALWAYS returns FALSE.
BOOL closeAll( CONST_STRPTR reason, ...)
{
	if(reason)
	{
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

	if(IIntuition)
	{
		closeInterface(IIntuition);
		IIntuition = NULL;
	}

	closeMidi(NULL);

	if(TimerPosted)
	{
		IExec->AbortIO(&TimerIoReq->Request);
		IExec->WaitIO(&TimerIoReq->Request);
		TimerPosted = FALSE;
	}

	IExec->DropInterface((struct Interface *)ITimer);
	ITimer = NULL;
	if (TimerBase)
	{
		IExec->CloseDevice(&TimerIoReq->Request);
		TimerBase = NULL;
	}

	IExec->FreeSysObject(ASOT_IOREQUEST, &TimerIoReq->Request);
	TimerIoReq = NULL;
	IExec->FreeSysObject(ASOT_PORT, TimerPort);
	TimerPort = NULL;
	IExec->FreeSysObject(ASOT_PORT, UsbNotifyPort);
	UsbNotifyPort = NULL;
	IExec->DropInterface((struct Interface *)IUSBSys);
	IUSBSys = NULL;
	if (USBSysBase)
	{
		IExec->CloseDevice(OpenIOReq);
		USBSysBase = NULL;
	}
	IExec->FreeSysObject(ASOT_IOREQUEST, OpenIOReq);
	OpenIOReq = NULL;
	IExec->FreeSysObject(ASOT_PORT, UsbPort);
	UsbPort = NULL;

	// return dynamic string buffers
	freeStrings();

	IIcon      = closeInterface(IIcon);
	IUtility   = closeInterface(IUtility);
	IExpansion = closeInterface(IExpansion);
	IIntuition = closeInterface(IIntuition);

	return(RETURN_FAIL);
}

// find a non-NIL: output handle, or open a CON:
// Write the given message to it.
BOOL errMessage(CONST_STRPTR reason, ...)
{
	if((!reason)||(0 == reason[0]))
	{
		return(TRUE);
	}

	if(reason)
	{
		uint32 args[4], x;
		va_list ap;
		va_start(ap, reason);
		for(x = 0; x < 4; x++)
		{
			args[x] = va_arg(ap, uint32);
		}
		va_end(ap);
		IDOS->Printf(reason, args[0], args[1], args[2], args[3]);
	}
	return(TRUE);
}

APTR openDevInterface (
	CONST_STRPTR dev_name, 
	int32 dev_unit,
	struct IORequest *ior, 
	uint32 dev_flags,
	CONST_STRPTR int_name, 
	int32 int_vers)
{
   int8 openFail;
   struct Interface *interface;
   STRPTR buffer;

	if(!ior)
	{
		errMessage("OpenDevice(%s, %ld) failed, no IORequest given\n", dev_name, dev_unit);
		return(NULL);
	}

   openFail = IExec->OpenDevice(dev_name, dev_unit, ior, dev_flags);
   if (0 == openFail)
   {
      interface = IExec->GetInterface(
      	(struct Library *)ior->io_Device,
      	int_name, int_vers, NULL);
      if (interface)
      {
         return(interface);
      }
      IExec->CloseDevice(ior);

		if((IUtility)&&((buffer = IUtility->ASPrintf(
			"Failed to get %s interface version %ld from %s\n",
			int_name, int_vers, dev_name))))
		{
			errMessage(buffer);
			IExec->FreeVec(buffer);
			return(NULL);
		}

   }

	if((IUtility)&&((buffer = IUtility->ASPrintf(
		"Failed to open %s unit %ld\n",
		dev_name, dev_unit))))
	{
		errMessage(buffer);
		IExec->FreeVec(buffer);
		return(NULL);
	}

	errMessage("Failed to open a device");

   return NULL;
}

APTR openLibInterface (CONST_STRPTR lib_name, int32 lib_vers,
   CONST_STRPTR int_name, int32 int_vers)
{
   struct Library   *library;
   struct Interface *interface;
   STRPTR buffer;

   library = IExec->OpenLibrary(lib_name, lib_vers);
   if (library)
   {
      interface = IExec->GetInterface(library, 
      	int_name, int_vers, NULL);
      if (interface)
      {
         return interface;
      }
      IExec->CloseLibrary(library);

		if((IUtility)&&((buffer = IUtility->ASPrintf(
			"Failed to get %s interface version %ld from %s\n",
			int_name, int_vers, lib_name))))
		{
			errMessage(buffer);
			IExec->FreeVec(buffer);
			return(NULL);
		}

   }

	if((IUtility)&&((buffer = IUtility->ASPrintf(
		"Failed to open %s version %ld\n",
		lib_name, lib_vers))))
	{
		errMessage(buffer);
		IExec->FreeVec(buffer);
		return(NULL);
	}

	errMessage("Failed to open a library");


   return NULL;
}

APTR closeInterface (APTR interface)
{
   if (interface)
   {
      struct Library *library = ((struct Interface *)
      	interface)->Data.LibBase;
      IExec->DropInterface(interface);
      IExec->CloseLibrary(library);
   }
	return(NULL);
}

APTR openClass(CONST_STRPTR className, uint32 classVers, Class **classPtr)
{
	APTR result = NULL;

	if((result = IIntuition->OpenClass(className, classVers, classPtr)))
	{
		return(result);
	}

	errMessage("Failed to open Class %s version %ld\n", className, classVers);
	return(NULL);
}




