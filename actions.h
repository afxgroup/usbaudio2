// actions.h
#ifndef ACTIONS_H
#define ACTIONS_H

#include "usbsupport.h"

int32 play(struct AudioDevice *aud, CONST_STRPTR filename);
int32 record(struct AudioDevice *aud, CONST_STRPTR filename);
int32 monitor(struct AudioDevice *aud);

#endif
