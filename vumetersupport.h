#ifndef MIDI_SUPPORT_H
#define MIDI_SUPPORT_H

#include "usbsupport.h"

/* 0.25 dB breakpoints, counting down from 0dBFS
 * for limit voltage of 2147483647
 */
int32 dBTable32[128];

/* 0.25 dB breakpoints, counting down from 0dBFS
 * for limit voltage of 8388607
 */
int32 dBTable24[128];

/* 0.25 dB breakpoints, counting down from 0dBFS
 * for limit voltage of 32767
 */
int32 dBTable16[128];

// return value is limited 0-127
int8 midiLimits(int32 value);

// lookup deciBel for peak of given magnitude, transmit as velocity of given note number
int32 senddBDown(struct AudioDevice *aud, int32 peak, int32 magnitude, int8 note, struct MidiLink *link);

// post/repost a repetitive timer.
void postTimer(struct AudioDevice *adev, uint32 delayms);

#endif

