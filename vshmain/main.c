#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspctrl.h>

#include <stdio.h>
#include <string.h>

PSP_MODULE_INFO("bogus_vsh", 0x1000, 1, 0);

PSP_MAIN_THREAD_ATTR(0);

// Why do we need this? Well, we need to link against a lib that won't be resolved
// on cold boot as vshmain is started from game mode and pspbtcnf_game.txt does not
// load vsh dependencies. It is expected that the real vshmain will fail to load
// and the psp is warm rebooted using pspbtcnf.txt...
void DummyVshBridgeDep()
{
	vshKernelExitVSHVSH();
}

int main_thread(SceSize args, void *argp)
{
	SceCtrlData pad;
	SceUID mod;

	pspSdkInstallNoDeviceCheckPatch();
	pspSdkInstallNoPlainModuleCheckPatch();

	sceCtrlReadBufferPositive(&pad, 1);

	if (!(pad.Buttons & PSP_CTRL_LTRIGGER))
	{
		mod = sceKernelLoadModule("flash0:/kd/systemctrl.prx", 0, 0);
		if (mod >= 0)
			sceKernelStartModule(mod, 0, NULL, NULL, NULL);

		mod = sceKernelLoadModule("flash0:/kd/vshctrl.prx", 0, NULL);
		if (mod >= 0)
			sceKernelStartModule(mod, 0, NULL, NULL, NULL);

		mod = sceKernelLoadModule("flash0:/vsh/module/vshex.prx", 0, NULL);
		if (mod >= 0)
			sceKernelStartModule(mod, 0, NULL, NULL, NULL);
	}

	mod = sceKernelLoadModule("flash0:/vsh/module/vshmain_real.prx", 0, NULL);
	if (mod > 0)
		sceKernelStartModule(mod, args, argp, NULL, NULL);
	
	sceKernelExitDeleteThread(0);
	
	return 0;
}

int module_start(SceSize args, void *argp) __attribute__((alias("_start")));

int _start(SceSize args, void *argp)
{
	SceUID th;

	u32 func = 0x80000000 | (u32)main_thread;

	th = sceKernelCreateThread("main_thread", (void *)func, 0x20, 0x10000, 0, NULL);

	if (th >= 0)
	{
		sceKernelStartThread(th, args, argp);
	}

	return 0;
}
