#include <stdio.h>
#include <string.h>

#include <pspsdk.h>
#include <psptypes.h>
#include <pspmoduleinfo.h>
#include <pspmodulemgr.h>
#include <pspkernel.h>

#include "../include/macros.h"
#include "../include/systemctrl.h"

PSP_MODULE_INFO("vshctrl", 0x1000, 1, 0);

void PatchSyscall(u32 funcaddr, void *newfunc);

STMOD_HANDLER previous = NULL;
SceUID gamedfd = -1;

void clearCaches(void)
{
    sceKernelDcacheWBinvAll();
	sceKernelIcacheClearAll();
}

int CorruptIconPatch(char *name)
{
	char path[256];
	SceIoStat stat;

	sprintf(path, "ms0:/PSP/GAME/%s%%/EBOOT.PBP", name);

	memset(&stat, 0, sizeof(stat));

	if (sceIoGetstat(path, &stat) >= 0)
	{
		strcpy(name, "__SCE"); // hide icon
		return 1;
	}
	
	return 0;
}

SceUID sceIoDopenPatched(const char *dirname)
{
	int res, k1;
	
	k1 = pspSdkSetK1(0);
	
	res = sceIoDopen(dirname);

	if (strcmp(dirname, "ms0:/PSP/GAME") == 0)
		gamedfd = res;

	pspSdkSetK1(k1);

	return res;
}

int sceIoDreadPatched(SceUID fd, SceIoDirent *dir)
{
    int res, k1;
	
	k1 = pspSdkSetK1(0);
	
	res = sceIoDread(fd, dir);

    if (res > 0)
    {
        if (fd == gamedfd && dir->d_name[0] != '.')
			CorruptIconPatch(dir->d_name);
    }

	pspSdkSetK1(k1);

    return res;
}

int sceIoDclosePatched(SceUID fd)
{
    int res, k1;

	k1 = pspSdkSetK1(0);
    
	res = sceIoDclose(fd);
    
    if (fd == gamedfd)
        gamedfd = -1;

	pspSdkSetK1(k1);

    return res;
}

void IoPatches(void)
{
    SceModule *mod;
    u32 text_addr;

	mod = sceKernelFindModuleByName("sceIOFileManager");

	if (mod)
	{
        int intr = sceKernelCpuSuspendIntr();

		text_addr = mod->text_addr;

		PatchSyscall(text_addr+0x16B4, sceIoDopenPatched);
		PatchSyscall(text_addr+0x1810, sceIoDreadPatched);
		PatchSyscall(text_addr+0x18D4, sceIoDclosePatched);

        sceKernelCpuResumeIntr(intr);
    }
}

int OnModuleStart(SceModule *mod)
{	
    if (strcmp(mod->modname, "vsh_module") == 0) {
        IoPatches();
        clearCaches();
    }

	if (!previous)
		return 0;

	return previous(mod);
}


int module_start(SceSize args, void *argp)
{
	previous = sctrlHENSetStartModuleHandler(OnModuleStart);

	return 0;
}
