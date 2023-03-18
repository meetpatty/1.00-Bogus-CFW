#include <pspsdk.h>
#include <pspkernel.h>
#include <psputils.h>
#include <pspctrl.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "systemctrlmod.h"
#include "vshexmod.h"
#include "vshmainmod.h"

PSP_MODULE_INFO("bogusFlasher_app", 0x0800, 1, 0);

PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);

#define FLASHER_PATH "ms0:/PSP/GAME/BogusFlasher/"
#define printf pspDebugScreenPrintf

void ErrorExit(int milisecs, char *fmt, ...)
{
	va_list list;
	char msg[256];	

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	pspDebugScreenSetTextColor(0x000000FF);

	printf(msg);

	sceKernelDelayThread(milisecs*1000);
	sceKernelExitGame();
}

#define VSHMAIN_REAL_SIZE	57216

char vshmain_real_buffer[VSHMAIN_REAL_SIZE];

u8 vshmain_real_md5[16] =
{
	0x1E, 0x8B, 0x03, 0xFE, 0x7B, 0x19, 0x10, 0xD3,
	0x80, 0x80, 0x48, 0x6F, 0x0A, 0x6F, 0x09, 0xD4
};

char buf[16536];

void copy_vshmain()
{
	SceUID i = sceIoOpen("flash0:/vsh/module/vshmain_real.prx", PSP_O_RDONLY, 0777);

	if (i < 0)
	{
		i = sceIoOpen("flash0:/vsh/module/vshmain.prx", PSP_O_RDONLY, 0777);

		if (i < 0)
		{
			ErrorExit(4000, "Cannot copy vshmain.prx: input error.\n");
		}
	}

	SceUID o = sceIoOpen(FLASHER_PATH "vshmain_real.prx", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (o < 0)
	{
		ErrorExit(4000, "Cannot copy vshmain.prx: output error.\n");
	}

	int read;

	while ((read = sceIoRead(i, buf, 16536)) > 0)
	{
		sceIoWrite(o, buf, read);
	}

	sceIoClose(i);
	sceIoClose(o);
}

void read_file(char *file, u8 *md5, char *buf, int size)
{
	SceUID fd;
	int read;
	u8 digest[16];
	
	fd= sceIoOpen(file, PSP_O_RDONLY, 0777);

	if (fd < 0)
	{
		ErrorExit(4000, "Cannot read file %s.\n", file);
	}
	
	read = sceIoRead(fd, buf, size);

	if (read != size)
	{
		ErrorExit(4000, "File size of %s doesn't match.\n", file);
	}

	if (sceKernelUtilsMd5Digest((u8 *)buf, size, digest) < 0)
	{
		ErrorExit(4000, "Error calculating md5.\n");
	}

	if (memcmp(digest, md5, 16) != 0)
	{
		ErrorExit(4000, "MD5 of %s doesn't match.\n", file);
	}
}

void flash_file(char *file, char *buf, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (fd >= 0)
	{
		sceIoWrite(fd, buf, size);
		sceIoClose(fd);
	}
	// else { better not to think we are going end here :P }
}

int is_bogus(void)
{
	SceIoStat stat;
	int ins;

	ins = *(int*)sceKernelDevkitVersion;

	//Vanilla 1.00 Bogus sysmem doesn't implement sceKernelDevkitVersion
	if (ins != 0x3E00008)
	{
		if (sceKernelDevkitVersion() != 0x00100000)
			return 0;
	}

	memset(&stat, 0, sizeof(stat));
	
	if (sceIoGetstat("flash0:/kd/loadcorei.prx", &stat) < 0)
		return 0;

	return 1;
}

int main()
{
	pspDebugScreenInit();
	pspDebugScreenClear();
	pspDebugScreenSetTextColor(0x0000FF00);

	if (!is_bogus())
		ErrorExit(4000, "This installer can only be run on 1.00 Bogus.\n");

	printf("Copying vshmain.prx as vshmain_real.prx.\n");
	copy_vshmain();

	printf("Copying files to buffers...\n");
	read_file(FLASHER_PATH "vshmain_real.prx", vshmain_real_md5, vshmain_real_buffer, VSHMAIN_REAL_SIZE);

	if (sceIoUnassign("flash0:") < 0)
		ErrorExit(4000, "Cannot unassign flash0.");

	if (sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0) < 0)
	{
		ErrorExit(4000, "Error assigning flash0 in write mode.");
	}

	pspDebugScreenSetTextColor(0x000000FF);
	printf("Preparation done. Press X to flash the proof of concept at your own risk.\n");

	while (1)
	{
		SceCtrlData pad;

		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS)
			break;

		sceKernelDelayThread(20000);
	}

	printf("Flashing files...\n");

	sceIoRemove("flash0:/vsh/module/vshmain.prx");

	flash_file("flash0:/vsh/module/vshmain.prx", vshmain_buffer, size_vshmain_buffer);
	flash_file("flash0:/vsh/module/vshmain_real.prx", vshmain_real_buffer, VSHMAIN_REAL_SIZE);
	flash_file("flash0:/vsh/module/vshex.prx", vshex_buffer, size_vshex_buffer);
	flash_file("flash0:/kd/systemctrl.prx", systemctrl_buffer, size_systemctrl_buffer);

	ErrorExit(4000, "Done.\n");

	return 0;
}

