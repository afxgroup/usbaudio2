// oca.h
#ifndef OCA_H
#define OCA_H

#include <proto/intuition.h>
#include "myargs.h"

#define MAJ_REV 53

#define MIN_ISO_IOS 16

struct ExpansionIFace *IExpansion;
struct IconIFace *IIcon;
struct UtilityIFace *IUtility;

struct MsgPort *UsbPort;
struct MsgPort *UsbNotifyPort;

struct Library  *USBSysBase;
struct USBSysIFace *IUSBSys;
struct IORequest *OpenIOReq; 

struct TimeRequest *TimerIoReq;
struct MsgPort *TimerPort;
struct Device  *TimerBase;
struct TimerIFace *ITimer;
BOOL TimerPosted;

//
 //	Opens all libraries, classes, devices
 //   reads all args or tooltypes
 //   returns UsbAudioInterface *, else NULL
//
BOOL openAll(int argc, char **argv);

// always safe to call, handles any intermediate states
// displays any given reason by errMessage()
// ALWAYS returns FALSE
BOOL closeAll(CONST_STRPTR reason, ...);

// use any available output, or create one if needed
// always returns TRUE
BOOL errMessage(CONST_STRPTR reason, ...);

// easy OpenDevice & GetInterface in one
// no need to store DevBase, as long as closeInterface is used to cleanup
// displays error details by errMessage if needed
APTR openDevInterface (CONST_STRPTR dev_name, int32 dev_unit,
	struct IORequest *ior, uint32 dev_flags,
	CONST_STRPTR int_name, int32 int_vers);

// easy OpenLibrary & GetInterface in one
// no need to store LibraryBase, as long as closeInterface is used to cleanup
// displays error details by errMessage if needed
APTR openLibInterface (CONST_STRPTR lib_name, int32 lib_vers,
   CONST_STRPTR int_name, int32 int_vers);
// ALWAYS returns NULL
APTR closeInterface (APTR interface);

// IIntuition->OpenClass() with error text output
APTR openClass(CONST_STRPTR className, uint32 classVers, Class **classPtr);


#endif
