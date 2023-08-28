#ifndef MYARGS_H
#define MYARGS_H

// include file for myArgs

// process all args and tooltypes into globals below
// returns RETURN_OK or RETURN_FAIL
int32 myArgs(int argc, char **argv);

// startup information
BOOL   ShellStart, WorkbenchStart;
STRPTR ProgramName;

STRPTR FileName;
int32   Record;			// Show level meters until Ctrl-C or Break
int32   Play;			// Show level meters until Ctrl-C or Break
int32   Monitor;			// Show level meters until Ctrl-C or Break
int32  Seconds;
int32  Channels;
int32  Rate;
int32  Width;
APTR   RawFkt;
int32   MinIor;
int32   Alternate;
int32   FillOut;			// duplicate source channels on playback
int32   Help;			// short guide
int32   Verbose;			// print lots of details

#endif

