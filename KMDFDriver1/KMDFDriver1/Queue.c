/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
#include "Thread.h"
#include "queue.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, KMDFDriver1QueueInitialize)
#endif

NTSTATUS
KMDFDriver1QueueInitialize(
    _In_ WDFDEVICE Device
    )
/*++

Routine Description:

     The I/O dispatch callbacks for the frameworks device object
     are configured in this function.

     A single default I/O Queue is configured for parallel request
     processing, and a driver context memory allocation is created
     to hold our structure QUEUE_CONTEXT.

Arguments:

    Device - Handle to a framework device object.

Return Value:

    VOID

--*/
{
	//DbgBreakPoint();

    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG queueConfig;

    PAGED_CODE();

    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
         &queueConfig,
        WdfIoQueueDispatchParallel
        );

    queueConfig.EvtIoDeviceControl = KMDFDriver1EvtIoDeviceControl;
    queueConfig.EvtIoStop = KMDFDriver1EvtIoStop;

    status = WdfIoQueueCreate(
                 Device,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
                 &queue
                 );

    if(!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate failed %!STATUS!", status);
        return status;
    }

	HANDLE thread;

	PsCreateSystemThread(&thread, GENERIC_ALL, NULL, NULL, NULL, ThreadStart, NULL);

    return status;
}

VOID
KMDFDriver1EvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
    )
/*++

Routine Description:

    This event is invoked when the framework receives IRP_MJ_DEVICE_CONTROL request.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    OutputBufferLength - Size of the output buffer in bytes

    InputBufferLength - Size of the input buffer in bytes

    IoControlCode - I/O control code.

Return Value:

    VOID

--*/
{
	DbgBreakPoint();

    TraceEvents(TRACE_LEVEL_INFORMATION, 
                TRACE_QUEUE, 
                "%!FUNC! Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode %d", 
                Queue, Request, (int) OutputBufferLength, (int) InputBufferLength, IoControlCode);
	WDFMEMORY input;
	WdfRequestRetrieveInputMemory(Request, &input);
	WDFMEMORY output;
	WdfRequestRetrieveOutputMemory(Request, &output);

	ULONG loc;
	WdfMemoryCopyToBuffer(input, 0, &loc, 4);

	LPWSTR outp = L"Hello World";
	WdfMemoryCopyFromBuffer(output, 0, outp, 24);

	HANDLE process;

	OBJECT_ATTRIBUTES pidAttr;
	InitializeObjectAttributes(&pidAttr, NULL, 0, NULL, NULL);
	CLIENT_ID pid;
	pid.UniqueProcess = (HANDLE)loc;
	pid.UniqueThread = 0;

	NTSTATUS status;

	status = ZwOpenProcess(&process, PROCESS_ALL_ACCESS, &pidAttr, &pid);

	HANDLE token;

	status = ZwOpenProcessTokenEx(process, GENERIC_ALL, 0, &token);

	BYTE label[28];

	ULONG len;

	status = ZwQueryInformationToken(token, TokenIntegrityLevel, &label, sizeof(label), &len);
	
	SID_IDENTIFIER_AUTHORITY sa = SECURITY_MANDATORY_LABEL_AUTHORITY;
	SID outm;
	RtlInitializeSidEx(&outm, &sa, 1, SECURITY_MANDATORY_PROTECTED_PROCESS_RID);
	SID **p = (PSID)label;
	**p = outm;

	KeEnterCriticalRegion();

	/*
	nt!NtSetInformationToken+0x850 . set to . ff ff ff ff 	. +0x84d is now . cmp r12d, -1 	[orig: 00 40 00 00]
	nt!NtSetInformationToken+0x8c5 . set to . 83 f8 ff 		. +0x8c5 is now . cmp eax, -1 	[orig: 44 3b e0]
	*/

	BYTE *x = (BYTE *)NtSetInformationToken;

	x += 0x850;

	x[0] = x[1] = x[2] = x[3] = 0xff;

	x += 0x75;

	x[0] = 0x83; x[1] = 0xf8; x[2] = 0xff;

	status = ZwSetInformationToken(token, TokenIntegrityLevel, &label, sizeof(label));

	x[0] = 0x44; x[1] = 0x3b; x[2] = 0xe0;

	x -= 0x75;

	x[0] = x[2] = x[3] = 0x00; x[1] = 0x40;

	KeLeaveCriticalRegion();

	ObCloseHandle(process, UserMode);

	ObCloseHandle(token, UserMode);

    WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, 24);

	UNREFERENCED_PARAMETER(status);

    return;
}

VOID
KMDFDriver1EvtIoStop(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ ULONG ActionFlags
)
/*++

Routine Description:

    This event is invoked for a power-managed queue before the device leaves the working state (D0).

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    ActionFlags - A bitwise OR of one or more WDF_REQUEST_STOP_ACTION_FLAGS-typed flags
                  that identify the reason that the callback function is being called
                  and whether the request is cancelable.

Return Value:

    VOID

--*/
{
	//DbgBreakPoint();

    TraceEvents(TRACE_LEVEL_INFORMATION, 
                TRACE_QUEUE, 
                "%!FUNC! Queue 0x%p, Request 0x%p ActionFlags %d", 
                Queue, Request, ActionFlags);

    //
    // In most cases, the EvtIoStop callback function completes, cancels, or postpones
    // further processing of the I/O request.
    //
    // Typically, the driver uses the following rules:
    //
    // - If the driver owns the I/O request, it calls WdfRequestUnmarkCancelable
    //   (if the request is cancelable) and either calls WdfRequestStopAcknowledge
    //   with a Requeue value of TRUE, or it calls WdfRequestComplete with a
    //   completion status value of STATUS_SUCCESS or STATUS_CANCELLED.
    //
    //   Before it can call these methods safely, the driver must make sure that
    //   its implementation of EvtIoStop has exclusive access to the request.
    //
    //   In order to do that, the driver must synchronize access to the request
    //   to prevent other threads from manipulating the request concurrently.
    //   The synchronization method you choose will depend on your driver's design.
    //
    //   For example, if the request is held in a shared context, the EvtIoStop callback
    //   might acquire an internal driver lock, take the request from the shared context,
    //   and then release the lock. At this point, the EvtIoStop callback owns the request
    //   and can safely complete or requeue the request.
    //
    // - If the driver has forwarded the I/O request to an I/O target, it either calls
    //   WdfRequestCancelSentRequest to attempt to cancel the request, or it postpones
    //   further processing of the request and calls WdfRequestStopAcknowledge with
    //   a Requeue value of FALSE.
    //
    // A driver might choose to take no action in EvtIoStop for requests that are
    // guaranteed to complete in a small amount of time.
    //
    // In this case, the framework waits until the specified request is complete
    // before moving the device (or system) to a lower power state or removing the device.
    // Potentially, this inaction can prevent a system from entering its hibernation state
    // or another low system power state. In extreme cases, it can cause the system
    // to crash with bugcheck code 9F.
    //

    return;
}
