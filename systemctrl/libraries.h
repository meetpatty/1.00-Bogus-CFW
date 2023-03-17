#ifndef __LIBRARIES_H__
#define __LIBRARIES_H__

#define N_LIBRARIES	1

typedef struct
{
	u32 oldnid;
	u32 newnid;
} Nids;

typedef struct
{
	const char *name;
	Nids *nids;
	int  nnids;
} LibraryData;

Nids SysMemUserForUser[] =
{
	{ 0xA291F107, 0xE6581468 } //sceKernelMaxFreeMemSize
};

LibraryData libraries[N_LIBRARIES] =
{
	{
		"SysMemUserForUser",
		SysMemUserForUser,
		1
	}
};

#endif

