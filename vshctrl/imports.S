	.set noreorder

#include "pspstub.s"

	STUB_START	"SystemCtrlForKernel",0,0x00020005
	STUB_FUNC	0x1C90BECB,sctrlHENSetStartModuleHandler
	STUB_FUNC	0x826668E9,PatchSyscall
	STUB_END
