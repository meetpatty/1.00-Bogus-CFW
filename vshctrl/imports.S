	.set noreorder

#include "pspstub.s"

	STUB_START	"SystemCtrlForKernel",0,0x00020005
	STUB_FUNC	0x1C90BECB,sctrlHENSetStartModuleHandler
	STUB_FUNC	0x826668E9,PatchSyscall
	STUB_END

	STUB_START	"SysMemForKernel",0x00010000,0x00020005
	STUB_FUNC	0x237DBD4F,sceKernelAllocPartitionMemory
	STUB_FUNC	0x9D9A5BA1,sceKernelGetBlockHeadAddr
	STUB_END