#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <psploadcore.h>

#include <string.h>

#include "libraries.h"

int (* doLinkLibEntries)(SceLibraryStubTable *imports, SceSize size, int user);

static LibraryData *FindLib(const char *lib)
{
	int i;

	if (!lib)
		return NULL;

	for (i = 0; i < N_LIBRARIES; i++)
	{
		if (strcmp(lib, libraries[i].name) == 0)
			return &libraries[i];
	}

	return NULL;
}

static u32 FindNewNid(LibraryData *lib, u32 nid)
{
	int i;

	for (i = 0; i < lib->nnids; i++)
	{
		if (lib->nids[i].oldnid == nid)
			return lib->nids[i].newnid;
	}

	return 0;
}

int doLinkLibEntriesPatched(SceLibraryStubTable *imports, SceSize size, int user)
{
	int i = 0;
	int j;
	u32 stubTab = (u32)imports;

	while (i < (size & 0xfffffffc))
	{
		SceLibraryStubTable *import = (SceLibraryStubTable *)(stubTab + i);
		LibraryData *data = FindLib(import->libname);

		if (data)
		{
			for (j = 0; j < import->stubcount; j++)
			{
				u32 nnid = FindNewNid(data, import->nidtable[j]);
				if (nnid)
				{
					import->nidtable[j] = nnid;
				}
			}
		}

		i += (import->len*4);
	}

	ClearCaches();

	int res = doLinkLibEntries(imports, size, user);

	return res;
}