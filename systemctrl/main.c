#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>

#include <stdio.h>
#include <string.h>

#include "../include/macros.h"

PSP_MODULE_INFO("SystemControl", 0x3007, 1, 0);

extern int sceKernelCreateThreadPatched(char *name, void *entry, int priority, int stackSize, int attr, SceKernelThreadOptParam *param);
extern int sceKernelStartThreadPatched(SceUID thid, SceSize arglen, void *argp);

void ClearCaches()
{
	sceKernelDcacheWBinvAll();
	sceKernelIcacheClearAll();
}

void PatchSceModuleManager()
{
	SceModule *mod = (SceModule *)sceKernelFindModuleByName("sceModuleManager");

	MAKE_JUMP(mod->text_addr + 0x46b4, sceKernelCreateThreadPatched);
	MAKE_JUMP(mod->text_addr + 0x46c4, sceKernelStartThreadPatched);

	ClearCaches();
}

int module_start(SceSize args, void *argp)
{
	PatchSceModuleManager();

	return 0;
}