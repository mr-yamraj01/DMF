/*++

    Copyright (c) Microsoft Corporation. All rights reserved.
    Licensed under the MIT license.

Module Name:

    DmfHelpers.c

Abstract:

    DMF Implementation.
    This Module contains helper functions. Some are used by Clients and DMF, others are used
    only by the DMF.

Environment:

    Kernel-mode Driver Framework
    User-mode Driver Framework

--*/

#include "DmfIncludeInternal.h"

#include "DmfHelpers.tmh"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// These are used by both DMF and DMF Clients.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//

_IRQL_requires_max_(DISPATCH_LEVEL)
WDFDEVICE
DMF_AttachedDeviceGet(
    _In_ DMFMODULE DmfModule
    )
/*++

Routine Description:

    Returns the WDFDEVICE that contains the given DmfModule.

Arguments:

    DmfModule: The given DMF Module.

Return Value:

    WDFDEVICE.

--*/
{
    // NOTE: No FuncEntry/Exit logging. It is too much and it is not necessary for this
    // simple function.
    //

    DMF_OBJECT* dmfObject;

    dmfObject = DMF_ModuleToObject(DmfModule);

    DMF_HandleValidate_IsAvailable(dmfObject);

    ASSERT(dmfObject != NULL);
    ASSERT(dmfObject->ParentDevice != NULL);

    return dmfObject->ParentDevice;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
WDFDEVICE
DMF_ParentDeviceGet(
    _In_ DMFMODULE DmfModule
    )
/*++

Routine Description:

    Returns the WDFDEVICE Object that corresponds to the Client Driver's FDO.

Arguments:

    DmfModule: This Module's Instance DMF Module.

Return Value:

    WDFDEVICE that corresponds to the Client Driver's FDO.

--*/
{
    // NOTE: No FuncEntry/Exit logging. It is too much and it is not necessary for this
    // simple function.
    //
    DMF_DEVICE_CONTEXT* dmfDeviceContext;
    DMF_OBJECT* dmfObject;

    dmfObject = DMF_ModuleToObject(DmfModule);

    DMF_HandleValidate_IsAvailable(dmfObject);

    ASSERT(dmfObject != NULL);
    ASSERT(dmfObject->ParentDevice != NULL);

    // ParentDevice can be either Client driver device or the Control device 
    // (in the case when the Client driver is a filter driver and DmfModule is added 
    // to the Control device.)
    // Since Client will need access to Client driver device,
    // return the Client driver device stored in ParentDevice's dmf context.
    //
    dmfDeviceContext = DmfDeviceContextGet(dmfObject->ParentDevice);

    ASSERT(dmfDeviceContext != NULL);
    ASSERT(dmfDeviceContext->WdfClientDriverDevice != NULL);

    return dmfDeviceContext->WdfClientDriverDevice;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
DMFMODULE
DMF_ParentModuleGet(
    _In_ DMFMODULE DmfModule
    )
/*++

Routine Description:

    Given a DMFMODULE, returns its Parent DMFMODULE.

Arguments:

    DmfModule - The given DMFMODULE.

Return Value:

    Parent DMFMODULE of the given DMFMODULE.
    NULL if no parent. (It is in the Module Collection array.)

--*/
{
    // NOTE: No FuncEntry/Exit logging. It is too much and it is not necessary for this
    // simple function.
    //

    DMF_OBJECT* dmfObject;
    DMFMODULE dmfModuleParent;

    dmfObject = DMF_ModuleToObject(DmfModule);

    DMF_HandleValidate_IsAvailable(dmfObject);

    if (dmfObject->DmfObjectParent != NULL)
    {
        dmfModuleParent = (DMFMODULE)(dmfObject->DmfObjectParent->MemoryDmfObject);
    }
    else
    {
        dmfModuleParent = NULL;
    }

    return dmfModuleParent;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID*
DMF_ModuleConfigGet(
    _In_ DMFMODULE DmfModule
    )
/*++

Routine Description:

    Returns the Client Driver's Module Config for use by the Module when the Module is opened.
    The Module Config allows the Module to initialize itself with Module specific parameters that
    are set by the Client Driver.

Arguments:

    DmfModule: This Module's Instance DMF Module.

Return Value:

    The Module's Module Config structure. Each Module casts the pointer to its own known
    structure type.

--*/
{
    // NOTE: No FuncEntry/Exit logging. It is too much and it is not necessary for this
    // simple function.
    //
    DMF_OBJECT* DmfObject;

    DmfObject = DMF_ModuleToObject(DmfModule);

    DMF_HandleValidate_IsAvailable(DmfObject);

    ASSERT(DmfObject != NULL);

    return DmfObject->ModuleConfig;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
LONG
DMF_ModuleReferenceAdd(
    _In_ DMFMODULE DmfModule
    )
/*++

Routine Description:

    Decrement the Module's Reference Count.
    This Routine must always be called in locked state.

Arguments:

    DmfModule - This Module's handle.

Return Value:

    The updated reference count.

--*/
{
    LONG returnValue;
    DMF_OBJECT* DmfObject;

    DmfObject = DMF_ModuleToObject(DmfModule);

    FuncEntryArguments(DMF_TRACE, "DmfObject=0x%p [%s]", DmfObject, DmfObject->ClientModuleInstanceName);

    DMF_HandleValidate_IsAvailable(DmfObject);

    // This routine must always be called in locked state.
    //
    ASSERT(DMF_ModuleIsLocked(DmfModule));

    returnValue = InterlockedIncrement(&DmfObject->ReferenceCount);

    FuncExit(DMF_TRACE, "DmfObject=0x%p [%s] returnValue=%d", DmfObject, DmfObject->ClientModuleInstanceName, returnValue);

    return returnValue;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
LONG
DMF_ModuleReferenceDelete(
    _In_ DMFMODULE DmfModule
    )
/*++

Routine Description:

    Increment the Module's Reference Count.
    This Routine must always be called in locked state.

Arguments:

    DmfModule - This Module's handle.

Return Value:

    The updated reference count.

--*/
{
    LONG returnValue;
    DMF_OBJECT* DmfObject;

    DmfObject = DMF_ModuleToObject(DmfModule);

    FuncEntryArguments(DMF_TRACE, "DmfObject=0x%p [%s]", DmfObject, DmfObject->ClientModuleInstanceName);

    DMF_HandleValidate_IsAvailable(DmfObject);

    ASSERT(DmfObject->ReferenceCount > 0);
    // This routine must always be called in locked state.
    //
    ASSERT(DMF_ModuleIsLocked(DmfModule));

    returnValue = InterlockedDecrement(&DmfObject->ReferenceCount);

    FuncExit(DMF_TRACE, "DmfObject=0x%p [%s] returnValue=%d", DmfObject, DmfObject->ClientModuleInstanceName, returnValue);

    return returnValue;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
BOOLEAN
DMF_ModuleIsInFilterDriver(
    _In_ DMFMODULE DmfModule
    )
/*++

Routine Description:

    Indicates if the given Module is executing in a filter driver.

Arguments:

    DmfModule - The given Module.

Return Value:

    TRUE indicates the Client Driver is a filter driver.
    FALSE indicates the Client Driver is not filter driver.

--*/
{
    DMF_DEVICE_CONTEXT* deviceContext;
    WDFDEVICE device;

    DMF_ObjectValidate(DmfModule);

    device = DMF_ParentDeviceGet(DmfModule);
    deviceContext = DmfDeviceContextGet(device);

    return deviceContext->IsFilterDevice;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// These are used by only by DMF.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//

BOOLEAN
DMF_IsTopParentDynamicModule(
    _In_ DMF_OBJECT* DmfObject
    )
/*++

Routine Description:

    Given a DMF_OBJECT determine if its root parent DMF_OBJECT refers to a Dynamic Module.
    (Determines if the given Module a child, either immediate or not, of a Dynamic Module.)

Arguments:

    DmfObject - The given DMF_OBJECT.

Return Value:

    TRUE - The given Module is a child of a Dynamic Module, immediate or not.
    FALSE - The given Module is not a child of a Dynamic Module, immediate or not.

--*/
{
    BOOLEAN returnValue;

    // Find the root Parent Module. (Its DmfObjectParent is NULL.)
    //
    while (DmfObject->DmfObjectParent != NULL)
    {
        DmfObject = DmfObject->DmfObjectParent;
    }

    // Determine if this root Parent Module is a Dynamic Module.
    //
    if (DmfObject->DynamicModule)
    {
        returnValue = TRUE;
    }
    else
    {
        returnValue = FALSE;
    }

    return returnValue;
}

#pragma code_seg("PAGE")
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
DMF_SynchronizationCreate(
    _In_ DMF_OBJECT* DmfObject,
    _In_ WDFDEVICE ParentDevice,
    _In_ DMF_MODULE_DESCRIPTOR* ModuleDescriptor,
    _In_ BOOLEAN PassiveLevel
    )
/*++

Routine Description:

    Create a set of Locks for a given DMF Module.

Arguments:

    DmfModule - The given DMF Module.
    ParentDevice - The Parent Device for the Module
    ModuleDescriptor - The given DMF Module's descriptor.
    PassiveLevel - TRUE if Client wants the Module options to be set to MODULE_OPTIONS_PASSIVE.
                   NOTE: Module Options must be set to MODULE_OPTIONS_DISPATCH_MAXIMUM. 

Return Value:

    STATUS_SUCCESS - locks are created.
    STATUS_UNSUCCESSFUL - locks were not created.

--*/
{
    NTSTATUS ntStatus;
    WDF_OBJECT_ATTRIBUTES attributes;
    ULONG lockIndex;

    PAGED_CODE();

    FuncEntryArguments(DMF_TRACE, "DmfObject=0x%p [%s]", DmfObject, DmfObject->ClientModuleInstanceName);

    ASSERT(ModuleDescriptor->NumberOfAuxiliaryLocks <= DMF_MAXIMUM_AUXILIARY_LOCKS);

    if (ModuleDescriptor->ModuleOptions & DMF_MODULE_OPTIONS_DISPATCH_MAXIMUM)
    {
        if (PassiveLevel)
        {
            ModuleDescriptor->ModuleOptions |= DMF_MODULE_OPTIONS_PASSIVE;
        }
        else
        {
            ModuleDescriptor->ModuleOptions |= DMF_MODULE_OPTIONS_DISPATCH;
        }
    }
    else
    {
        ASSERT(FALSE == PassiveLevel);
    }

    // Create the locking mechanism based on Module Options.
    //
    if (ModuleDescriptor->ModuleOptions & DMF_MODULE_OPTIONS_PASSIVE)
    {
        TraceInformation(DMF_TRACE, "DMF_MODULE_OPTIONS_PASSIVE");

        ASSERT(! (ModuleDescriptor->ModuleOptions & DMF_MODULE_OPTIONS_DISPATCH));

        // Create the Generic PASSIVE_LEVEL Lock for the Auxiliary Synchronization and one device lock.
        //
        for (lockIndex = 0; lockIndex < ModuleDescriptor->NumberOfAuxiliaryLocks + DMF_NUMBER_OF_DEFAULT_LOCKS; lockIndex++)
        {
            WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
            attributes.ParentObject = ParentDevice;
            ntStatus = WdfWaitLockCreate(&attributes,
                                         &DmfObject->Synchronizations[lockIndex].SynchronizationPassiveWaitLock);
            if (! NT_SUCCESS(ntStatus))
            {
                TraceEvents(TRACE_LEVEL_ERROR, DMF_TRACE, "Unable to create locks");
                goto Exit;
            }
        }
    }
    else
    {
        TraceInformation(DMF_TRACE, "DMF_MODULE_OPTIONS_DISPATCH");

        // Create the Generic DISPATCH_LEVEL Lock for the Auxiliary Synchronization and one device lock.
        //
        for (lockIndex = 0; lockIndex < ModuleDescriptor->NumberOfAuxiliaryLocks + DMF_NUMBER_OF_DEFAULT_LOCKS; lockIndex++)
        {
            WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
            attributes.ParentObject = ParentDevice;
            ntStatus = WdfSpinLockCreate(&attributes,
                                         &DmfObject->Synchronizations[lockIndex].SynchronizationDispatchSpinLock);
            if (! NT_SUCCESS(ntStatus))
            {
                TraceEvents(TRACE_LEVEL_ERROR, DMF_TRACE, "Unable to create locks");
                goto Exit;
            }
        }
    }

    ntStatus = STATUS_SUCCESS;

Exit:

    FuncExit(DMF_TRACE, "DmfObject=0x%p [%s] ntStatus=%!STATUS!", DmfObject, DmfObject->ClientModuleInstanceName, ntStatus);

    return ntStatus;
}
#pragma code_seg()

// TODO: This function should not reference DMF_OBJECT nor DMF_MODULE_COLLECTION.
//       Currently it is an exception because it is an "internal" Module. But, this
//       needs to be fixed.
//
DMF_OBJECT*
DMF_FeatureHandleGetFromModuleCollection(
    _In_ DMFCOLLECTION DmfCollection,
    _In_ DmfFeatureType DmfFeature
    )
/*++

Routine Description:

    Get the DMF Object from the DMF Collection of the specified DMF Feature.

Arguments:

    DmfCollection - The given DMF Collection.
    DmfFeatureType - The required DMF Feature identifier.

Return Value:

    DMF Object of the required DMF Feature.

--*/
{
    DMF_MODULE_COLLECTION* moduleCollectionHandle;
    DMF_OBJECT* dmfObjectFeature;

    ASSERT(DmfCollection != NULL);

    moduleCollectionHandle = DMF_CollectionToHandle(DmfCollection);

    ASSERT(DmfFeature < DmfFeature_NumberOfFeatures);
    dmfObjectFeature = moduleCollectionHandle->DmfObjectFeature[DmfFeature];
    // It can be NULL if this feature is not running.
    //

    return dmfObjectFeature;
}

DMFMODULE
DMF_FeatureModuleGetFromModule(
    _In_ DMFMODULE DmfModule,
    _In_ DmfFeatureType DmfFeature
    )
/*++

Routine Description:

    Given a DMF Module and a feature identifier, return the corresponding Feature Handle.

Arguments:

    DmfModule - The given DMF Module.
    DmfFeature - The required DMF feature identifier.

Return Value:

    The Feature Module that was created automatically for this Module.

--*/
{
    DMF_OBJECT* dmfObjectFeature;
    DMF_OBJECT* DmfObject;
    DMFMODULE dmfModuleFeature;

    DmfObject = DMF_ModuleToObject(DmfModule);
    ASSERT(DmfObject != NULL);

    DMFCOLLECTION dmfCollection = DMF_ObjectToCollection(DmfObject->ModuleCollection);

    dmfObjectFeature = DMF_FeatureHandleGetFromModuleCollection(dmfCollection,
                                                                DmfFeature);
    if (dmfObjectFeature != NULL)
    {
        dmfModuleFeature = DMF_ObjectToModule(dmfObjectFeature);
        DMF_ObjectValidate(dmfModuleFeature);
    }
    else
    {
        // The Client Driver has not enabled this DMF Feature.
        //
        dmfModuleFeature = NULL;
    }

    return dmfModuleFeature;
}

DMFMODULE
DMF_FeatureModuleGetFromDevice(
    _In_ WDFDEVICE Device,
    _In_ DmfFeatureType DmfFeature
    )
/*++

Routine Description:

    Given a WDF Device and a feature identifier, return the corresponding Feature Module.

Arguments:

    Device - The given WDFDEVICE handle.
    DmfFeature - The required DMF feature identifier.

Return Value:

    The Feature Module that was created automatically for the given WDF Device.

--*/
{
    DMF_OBJECT* dmfObjectFeature;
    DMFMODULE dmfModuleFeature;
    DMF_DEVICE_CONTEXT* dmfDeviceContext;

    ASSERT(Device != NULL);

    dmfDeviceContext = DmfDeviceContextGet(Device);
    ASSERT(dmfDeviceContext != NULL);

    dmfObjectFeature = DMF_FeatureHandleGetFromModuleCollection(dmfDeviceContext->DmfCollection,
                                                                DmfFeature);
    if (dmfObjectFeature != NULL)
    {
        dmfModuleFeature = DMF_ObjectToModule(dmfObjectFeature);
        DMF_ObjectValidate(dmfModuleFeature);
    }
    else
    {
        // The Client Driver has not enabled Automatic BranchTrack.
        //
        dmfModuleFeature = NULL;
    }

    return dmfModuleFeature;
}

_Must_inspect_result_
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
DMF_RequestPassthru(
    _In_ WDFDEVICE Device,
    _In_ WDFREQUEST Request
    )
/*++

Routine Description:

    Forward the given request to next lower driver.

Arguments:

    DmfModule - This Module's handle.

    Request - The given request.

Return Value:

    None. If the given request cannot be forwarded, the request is completed with error.

--*/
{
    WDFIOTARGET ioTarget;
    WDF_REQUEST_SEND_OPTIONS sendOptions; 

    ioTarget = WdfDeviceGetIoTarget(Device);

    WdfRequestFormatRequestUsingCurrentType(Request);
    WDF_REQUEST_SEND_OPTIONS_INIT(&sendOptions,
                                  WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);
    if (! WdfRequestSend(Request,
                         ioTarget,
                         &sendOptions))
    {
        // This is an error that generally should not happen.
        //
        TraceEvents(TRACE_LEVEL_ERROR, DMF_TRACE, "Unable to Passthru Request: Request=%p", Request);

        // It could not be passed down, so just complete it with an error.
        //
        WdfRequestComplete(Request,
                           STATUS_INVALID_DEVICE_STATE);
    }
    else
    {
        // Request will be completed by the target.
        //
        TraceEvents(TRACE_LEVEL_INFORMATION, DMF_TRACE, "Passthru Request: Request=%p", Request);
    }
}

_Must_inspect_result_
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
DMF_RequestPassthruWithCompletion(
    _In_ WDFDEVICE Device,
    _In_ WDFREQUEST Request,
    _In_ PFN_WDF_REQUEST_COMPLETION_ROUTINE CompletionRoutine,
    __drv_aliasesMem WDFCONTEXT CompletionContext
    )
/*++

Routine Description:

    Forward the given request to next lower driver. Sets a completion routine so that the WDFREQUEST can
    be post-processed.

Arguments:

    DmfModule - This Module's handle.

    Request - The given request.

Return Value:

    None. If the given request cannot be forwarded, the request is completed with error.

--*/
{
    WDFIOTARGET ioTarget;

    ioTarget = WdfDeviceGetIoTarget(Device);

    WdfRequestFormatRequestUsingCurrentType(Request);
    WdfRequestSetCompletionRoutine(Request,
                                   CompletionRoutine,
                                   CompletionContext);
    if (! WdfRequestSend(Request,
                         ioTarget,
                         NULL))
    {
        // This is an error that generally should not happen.
        //
        TraceEvents(TRACE_LEVEL_ERROR, DMF_TRACE, "Unable to Passthru Request: Request=%p", Request);

        // It could not be passed down, so just complete it with an error.
        //
        WdfRequestComplete(Request,
                           STATUS_INVALID_DEVICE_STATE);
    }
    else
    {
        // Request will be completed by the target.
        //
        TraceEvents(TRACE_LEVEL_INFORMATION, DMF_TRACE, "Passthru Request: Request=%p", Request);
    }
}

// eof: DmfHelpers.c
//
