#ifndef _SYSTEMCTRL_H_
#define _SYSTEMCTRL_H_

#include <psploadcore.h>

typedef int (* STMOD_HANDLER)(SceModule *);

STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER handler);

#endif