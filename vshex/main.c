#include <string.h>

#include <psptypes.h>
#include <pspmoduleinfo.h>
#include <pspmodulemgr.h>
#include <pspkernel.h>

#include "../include/macros.h"
#include "../include/systemctrl.h"

PSP_MODULE_INFO("vshex", 0, 1, 0);

char curParamKey[100];

STMOD_HANDLER previous = NULL;

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

void clearCaches(void)
{
    sceKernelDcacheWritebackAll();
    sceKernelIcacheInvalidateAll();
}

char *vsh_filename;
int *vsh_args;
void **vsh_argp;

int (*setupVshParamArgs)(char *filename);

char buf[256];

int setupVshParamArgsPatched()
{
    int len;

    len = sce_paf_private_strlen(vsh_filename);
    *vsh_args = len + 1;
    *vsh_argp = sce_paf_private_malloc(*vsh_args);
    sce_paf_private_memset(*vsh_argp, 0, *vsh_args);
    sce_paf_private_snprintf(*vsh_argp, *vsh_args, vsh_filename);
    
    strncpy(vsh_filename, *vsh_argp, 0x80);

    return 0;
}

void vvvGetVersionPatched(char *filename, u8 *buf, int len, int a3)
{
    strcpy(buf, "release:1.00 Bogus\n");
}

int OnModuleStart(SceModule *mod)
{
	if (strcmp(mod->modname, "game_plugin_module") == 0)
	{
        HIJACK_FUNCTION(mod->text_addr + 0xa614, getParamSfoKeyDataPatched, _getParamSfoKeyData);
        HIJACK_FUNCTION(mod->text_addr + 0xd270, getParamSfoKeyInfoPatched, _getParamSfoKeyInfo);

        clearCaches();
	}
    else if (strcmp(mod->modname, "sysconf_plugin_module") == 0) {
        MAKE_CALL(mod->text_addr + 0x818c, vvvGetVersionPatched);

        clearCaches();
    }
    else if (strcmp(mod->modname, "vsh_module") == 0) {

        setupVshParamArgs = mod->text_addr + 0x2cfc;
        MAKE_CALL(mod->text_addr + 0x3108, setupVshParamArgsPatched);

        vsh_filename = mod->text_addr + 0x1a8c0;
        vsh_args = mod->text_addr + 0x1a948;
        vsh_argp = mod->text_addr + 0x1a94c;

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
