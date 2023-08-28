// midisupport.h
// basic CAMD open close and receive

#ifndef MIDISUPPORT_H
#define MIDISUPPORT_H

#include <proto/camd.h>

struct CamdIFace *ICamd;

struct MidiNode *CamdNode;
STRPTR CamdVULinkName;
//STRPTR CamdRecLinkName;
struct MidiLink *CamdVULink;
//extern struct MidiLink *CamdRecLink;
int8 MidiSig;

// RETURN_OK or RETURN_FAIL
int32 openMidi(CONST_STRPTR name);

// always safe to call
int32 closeMidi(CONST_STRPTR why);

// return 0 if MIDI Reset received
int32 handleMidi(struct MidiNode *mNode);

#endif

