/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_KMDFDriver1,
    0x39c3adfa,0x57cd,0x4fc1,0x8b,0x75,0xea,0x30,0xca,0xfd,0xa3,0xe0);
// {39c3adfa-57cd-4fc1-8b75-ea30cafda3e0}
