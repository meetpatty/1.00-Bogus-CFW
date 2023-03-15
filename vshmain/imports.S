	.set noreorder

#include "pspimport.s"
	IMPORT_START "sceVshBridge",0x40010000
	IMPORT_FUNC  "sceVshBridge",0x40716012,vshKernelExitVSHVSH
