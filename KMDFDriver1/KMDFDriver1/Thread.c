
#include "Driver.h"
#include "Thread.h"
#include "Thread.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, ThreadStart)
#endif

void ThreadStart(
	_In_ PVOID StartContext
	)
{
	//DbgBreakPoint();

	UNREFERENCED_PARAMETER(StartContext);

	KTIMER tim;
	KeInitializeTimer(&tim);
	LARGE_INTEGER li;
	li.QuadPart = -50000000;
	KeSetTimer(&tim, li, NULL);
	KeWaitForSingleObject(&tim, Executive, KernelMode, TRUE, NULL);

	//DbgBreakPoint();
}