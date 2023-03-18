#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspctrl.h>

#include <stdio.h>
#include <string.h>

#include "../include/macros.h"
#include "rebootex.h"
#include "../include/systemctrl.h"

PSP_MODULE_INFO("SystemControl", 0x3007, 1, 0);

extern int sceKernelCreateThreadPatched(char *name, void *entry, int priority, int stackSize, int attr, SceKernelThreadOptParam *param);
extern int sceKernelStartThreadPatched(SceUID thid, SceSize arglen, void *argp);
extern int doLinkLibEntriesPatched(SceLibraryStubTable *imports, SceSize size, int user);
extern int (* doLinkLibEntries)(SceLibraryStubTable *imports, SceSize size, int user);

static u32 sceRebootTextAddr;

void ClearCaches()
{
	sceKernelDcacheWBinvAll();
	sceKernelIcacheClearAll();
}

int sceKernelRebootBeforeForUserPatched(int a0)
{
	memcpy(0x88FC0000, rebootex_buffer, size_rebootex_buffer);

	return sceKernelRebootBeforeForUser(a0);
}

int sceKernelRebootPatched(int *a0, int *a1, int* a2)
{
	void (*rebootex_entry)(int *a0, int *a1, int* a2, u32 sceRebootTextAddr) = (void *)0x88fc0000;

	//Jump to rebootex
	rebootex_entry(a0, a1, a2, sceRebootTextAddr);
}

void PatchSceLoaderCoreI(void)
{
	SceModule* mod = sceKernelFindModuleByName("sceLoaderCoreInternal");

	MAKE_CALL(mod->text_addr+0x1cec, doLinkLibEntriesPatched);
	MAKE_CALL(mod->text_addr+0x1d08, doLinkLibEntriesPatched);
	doLinkLibEntries = mod->text_addr+0x1b6c;
}

void PatchSceLoadExec()
{
	SceModule* mod = sceKernelFindModuleByName("sceLoadExec");

	MAKE_CALL(mod->text_addr + 0xef4, sceKernelRebootBeforeForUserPatched);
	MAKE_CALL(mod->text_addr + 0x109c, sceKernelRebootPatched);

	_sw(LI_V0(4), mod->text_addr + 0x251c);
}

void PatchSceModuleManager(void)
{
	SceModule *mod = sceKernelFindModuleByName("sceModuleManager");

	MAKE_JUMP(mod->text_addr + 0x46b4, sceKernelCreateThreadPatched);
	MAKE_JUMP(mod->text_addr + 0x46c4, sceKernelStartThreadPatched);
}

int OnModuleStart(SceModule *mod)
{
	char *moduleName = mod->modname;
	u32 text_addr = mod->text_addr;

	if (strcmp(moduleName, "sceReboot") == 0) {
		sceRebootTextAddr = text_addr;
	}

	return 0;
}

int module_start(SceSize args, void *argp)
{
	PatchSceLoaderCoreI();
	PatchSceLoadExec();
	PatchSceModuleManager();
	sctrlHENSetStartModuleHandler(OnModuleStart);

	ClearCaches();

	return 0;
}