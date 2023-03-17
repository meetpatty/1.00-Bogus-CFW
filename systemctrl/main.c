#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>

#include <stdio.h>
#include <string.h>

#include "../include/macros.h"

PSP_MODULE_INFO("SystemControl", 0x3007, 1, 0);

extern int sceKernelCreateThreadPatched(char *name, void *entry, int priority, int stackSize, int attr, SceKernelThreadOptParam *param);
extern int sceKernelStartThreadPatched(SceUID thid, SceSize arglen, void *argp);
extern int doLinkLibEntriesPatched(SceLibraryStubTable *imports, SceSize size, int user);
extern int (* doLinkLibEntries)(SceLibraryStubTable *imports, SceSize size, int user);

void ClearCaches()
{
	sceKernelDcacheWBinvAll();
	sceKernelIcacheClearAll();
}

void PatchSceLoaderCoreI(void)
{
	SceModule* module = sceKernelFindModuleByName("sceLoaderCoreInternal");

	MAKE_CALL(module->text_addr+0x1cec, doLinkLibEntriesPatched);
	MAKE_CALL(module->text_addr+0x1d08, doLinkLibEntriesPatched);
	doLinkLibEntries = module->text_addr+0x1b6c;
}

void PatchSceModuleManager(void)
{
	SceModule *mod = sceKernelFindModuleByName("sceModuleManager");

	MAKE_JUMP(mod->text_addr + 0x46b4, sceKernelCreateThreadPatched);
	MAKE_JUMP(mod->text_addr + 0x46c4, sceKernelStartThreadPatched);
}

int module_start(SceSize args, void *argp)
{
	PatchSceLoaderCoreI();
	PatchSceModuleManager();

	ClearCaches();

	return 0;
}