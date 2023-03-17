	.set noreorder

#include "pspstub.s"

	STUB_START	"SystemCtrlForUser",0x40010000,0x00010005
	STUB_FUNC	0x1C90BECB,sctrlHENSetStartModuleHandler
	STUB_END

	STUB_START	"scePaf",0x00010000,0x00040005
	STUB_FUNC	0xCA79D58B,sce_paf_private_strlen
	STUB_FUNC	0xFCB4E053,sce_paf_private_malloc
	STUB_FUNC	0x6829D7AF,sce_paf_private_memset
	STUB_FUNC	0x809A4F83,sce_paf_private_snprintf
	STUB_END
