#include <pspkerneltypes.h>

#include "../include/macros.h"

void (* rebootEntry)(s32 a0, s32 a1, s32 a2);
int (* sceBootLfatOpen)(char *filename);
int (* sceBootLfatRead)(void *buffer, int buffer_size);
int (* sceBootLfatClose)(void);
int (* _sceKernelCheckPspConfig)(char *buffer, int a1);
int (* DcacheClear)(void);
int (* IcacheClear)(void);

int game = 0;

void clearCaches()
{
	DcacheClear();
	IcacheClear();
}

int sceKernelCheckPspConfigPatched(char *buffer, int a1)
{
    int len = _sceKernelCheckPspConfig(buffer, a1);

    if (len == 0x3fa && game)
    {
        char *p = strstr(buffer, "%");

        if (p)
            strcpy(p, "/kd/systemctrl.prx\n%/vsh/module/common_util.prx\n%%/vsh/module/vshmain.prx\n");

        len += 19;
    }

    return len;
}

int sceBootLfatOpenPatched(char *filename) {

    if (strcmp(filename, "/kd/pspbtcnf_game.txt") == 0)
        game = 1;

    int res = sceBootLfatOpen(filename);

    return res;
}

int sceBootLfatReadPatched(void *buffer, int buffer_size) {

    int res = sceBootLfatRead(buffer, buffer_size);

    return res;
}

int sceBootLfatClosePatched() {

    int res = sceBootLfatClose();

    return res;
}

int loadCoreIModuleStartPatched(int (*module_start)(u32, void *), SceSize args, void *argp) {

    u32 const text_addr = module_start - 0x0cf4;

    // NoPlainModuleCheckPatch (mfc0 $v0, $22 -> li $v0, 1)
    _sw(0x24020001, text_addr + 0x4160);

    clearCaches();

    return module_start(8, argp);
}

void _reboot(s32 const a0, s32 const a1, s32 const a2,u32 reboot_text_addr) {

	rebootEntry = reboot_text_addr + 0x170;
	
	MAKE_JUMP(reboot_text_addr + 0xfdc, loadCoreIModuleStartPatched);

    DcacheClear = reboot_text_addr + 0x2b6c;
    IcacheClear = reboot_text_addr + 0x2f3c;

    _sceKernelCheckPspConfig = reboot_text_addr + 0x3ad8;
    MAKE_CALL(reboot_text_addr + 0x7c4, sceKernelCheckPspConfigPatched);

	/* File open redirection */
    sceBootLfatOpen = reboot_text_addr + 0x6078;
    sceBootLfatRead = reboot_text_addr + 0x6290;
    sceBootLfatClose = reboot_text_addr + 0x6230;

	MAKE_CALL(reboot_text_addr + 0x9c, sceBootLfatOpenPatched);
	MAKE_CALL(reboot_text_addr + 0xcc, sceBootLfatReadPatched);
	MAKE_CALL(reboot_text_addr + 0xf8, sceBootLfatClosePatched);

	clearCaches();
	
	rebootEntry(a0, a1, a2);
}