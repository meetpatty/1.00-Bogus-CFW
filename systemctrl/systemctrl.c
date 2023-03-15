#include <pspthreadman_kernel.h>

#include "../include/systemctrl.h"

STMOD_HANDLER stmod_handler = NULL;

static int start_thread;
static SceModule *last_module;

STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER handler)
{
	unsigned int k1 = pspSdkSetK1(0);

	STMOD_HANDLER res = stmod_handler;

	stmod_handler = (STMOD_HANDLER)(0x80000000 | (u32)handler);

	pspSdkSetK1(k1);
	return res;
}

int sceKernelStartThreadPatched(SceUID thid, SceSize arglen, void *argp)
{
	if (thid == start_thread)
	{
		start_thread = -1;

		if (last_module && stmod_handler)
			stmod_handler(last_module);
	}

	return sceKernelStartThread(thid, arglen, argp);
}

int sceKernelCreateThreadPatched(char *name, void *entry, int priority, int stackSize, int attr, SceKernelThreadOptParam *param)
{
	int thid = sceKernelCreateThread(name, entry, priority, stackSize, attr, param);
	if (thid >= 0 && strncmp(name, "SceModmgrStart", 14) == 0)
	{
		start_thread = thid;
		last_module = (SceModule *)sceKernelFindModuleByAddress((u32)entry);
	}

	return thid;
}