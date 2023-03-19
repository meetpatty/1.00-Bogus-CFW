#include <stdio.h>
#include <string.h>

#include <pspsdk.h>
#include <psptypes.h>
#include <pspmoduleinfo.h>
#include <pspmodulemgr.h>
#include <pspkernel.h>

#include "../include/macros.h"
#include "../include/systemctrl.h"

PSP_MODULE_INFO("VshControl", 0x1007, 1, 0);

void PatchSyscall(u32 funcaddr, void *newfunc);

STMOD_HANDLER previous = NULL;
SceUID gamedfd = -1;

void clearCaches(void)
{
    sceKernelDcacheWBinvAll();
	sceKernelIcacheClearAll();
}

u32 MakeSyscallStub(void *function) {
	SceUID block_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "", PSP_SMEM_High, 2 * sizeof(u32), NULL);
	u32 stub = (u32)sceKernelGetBlockHeadAddr(block_id);
	_sw(0x03E00008, stub);
	_sw(0x0000000C | (sceKernelQuerySystemCall(function) << 6), stub + 4);
	return stub;
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

char curParamKey[100];

int (*_getParamSfoKeyData)(u8 * buf, int a1, u8 *paramData);

int getParamSfoKeyDataPatched(u8 * buf, int a1, u8 *paramData)
{
    int res = _getParamSfoKeyData(buf, a1, paramData);

    if (res < 0)
    {
        if (strcmp(curParamKey, "DRIVER_PATH") == 0)  {
            return 0;
        }
        else if(strcmp(curParamKey, "DISC_ID") == 0) {
            strcpy((char *)paramData, "UCJS10041");
            return 0;
        }
        else if(strcmp(curParamKey, "DISC_VERSION") == 0 || strcmp(curParamKey, "PSP_SYSTEM_VER") == 0) {
            strcpy((char *)paramData, "1.00");
            return 0;
        }
        else if(strcmp(curParamKey, "PARENTAL_LEVEL") == 0) {
            paramData[0] = 1;
            return 0;
        }
    }

    return res;
}

int (*_getParamSfoKeyInfo)(u8 * buf, char *key, int* a2);

int getParamSfoKeyInfoPatched(u8 *buf, char *key, int* a2)
{
    strcpy(curParamKey, key);

    return _getParamSfoKeyInfo(buf, key, a2);
}

char *vsh_filename;
int *vsh_args;
void **vsh_argp;

int (*setupVshParamArgs)(char *filename);

int (*sce_paf_private_malloc)(int len);
int (*sce_paf_private_snprintf)(char *s, int n, const char *format, ...);

int setupVshParamArgsPatched()
{
    int len;

    len = strlen(vsh_filename);
    *vsh_args = len + 1;
    *vsh_argp = sce_paf_private_malloc(*vsh_args);
    memset(*vsh_argp, 0, *vsh_args);
    sce_paf_private_snprintf(*vsh_argp, *vsh_args, vsh_filename);
    
    strncpy(vsh_filename, *vsh_argp, 0x80);

    return 0;
}

int vvvGetVersionPatched(char *filename, u8 *buf, int len, int a3)
{
    strcpy(buf, "release:1.00 Bogus\n");

	return 20;
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

void GetPafFuncs(void)
{
    SceModule *mod = sceKernelFindModuleByName("scePaf_Module");

    sce_paf_private_malloc = mod->text_addr + 0x107848;
    sce_paf_private_snprintf = mod->text_addr + 0x10AC18;
}

int OnModuleStart(SceModule *mod)
{
	if (strcmp(mod->modname, "game_plugin_module") == 0)
	{
		HIJACK_FUNCTION(mod->text_addr + 0xa614, MakeSyscallStub(getParamSfoKeyDataPatched), _getParamSfoKeyData);
        HIJACK_FUNCTION(mod->text_addr + 0xd270, MakeSyscallStub(getParamSfoKeyInfoPatched), _getParamSfoKeyInfo);

        clearCaches();
	}
    else if (strcmp(mod->modname, "sysconf_plugin_module") == 0)
    {
		MAKE_CALL(mod->text_addr + 0x818c, MakeSyscallStub(vvvGetVersionPatched));

        clearCaches();
    }
    else if (strcmp(mod->modname, "vsh_module") == 0)
    {
		setupVshParamArgs = mod->text_addr + 0x2cfc;
        MAKE_CALL(mod->text_addr + 0x3108, MakeSyscallStub(setupVshParamArgsPatched));

        vsh_filename = mod->text_addr + 0x1a8c0;
        vsh_args = mod->text_addr + 0x1a948;
        vsh_argp = mod->text_addr + 0x1a94c;

		IoPatches();
        GetPafFuncs();

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
