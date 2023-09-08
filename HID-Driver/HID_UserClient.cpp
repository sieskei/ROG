//
//  HIDUserClient.cpp
//  HID-Driver
//
//  Created by Nick on 2/16/21.
//  Copyright © 2021 Nick. All rights reserved.
//

#include <DriverKit/IOLib.h>
#include "HID_UserClient.h"
#include "HID_Driver.h"

/// - Tag: HIDUserClient_IVars
struct HID_UserClient_IVars {
    HID_Driver *driver { nullptr };
    OSAction* callbackAction { nullptr };
};

// This struct should ideally be shared across all applications using the driver, as the integer values need to be the same across all programs.
typedef enum
{
    ExternalMethodType_RegisterAsyncCallback = 0,
} ExternalMethodType;

const IOUserClientMethodDispatch externalMethodChecks[1] = {
    // The async methods follow the same flow as the checked methods, so the checks and function implementations are similar.
    [ExternalMethodType_RegisterAsyncCallback] =
    {
        .function = (IOUserClientMethodFunction) &HID_UserClient::StaticRegisterAsyncCallback,
        .checkCompletionExists = true,
        .checkScalarInputCount = 0,
        .checkStructureInputSize = 0,
        .checkScalarOutputCount = 0,
        .checkStructureOutputSize = 0,
    },
};

kern_return_t HID_UserClient::StaticRegisterAsyncCallback(OSObject* target, void* reference, IOUserClientMethodArguments* arguments)
{
    if (target == nullptr)
    {
        return kIOReturnError;
    }

    return ((HID_UserClient*)target)->RegisterAsyncCallback(reference, arguments);
}

kern_return_t HID_UserClient::RegisterAsyncCallback(void* reference, IOUserClientMethodArguments* arguments)
{
    
    DBGLOG("Got new async callback");

    /// - Tag: RegisterAsyncCallback_StoreCompletion
    if (arguments->completion == nullptr)
    {
        DBGLOG("Got a null completion.");
        return kIOReturnBadArgument;
    }

    // Save the completion for later.
    // If not saved, then it might be freed before the asychronous return.
    ivars->callbackAction = arguments->completion;
    ivars->callbackAction->retain();

    return kIOReturnSuccess;
}

bool HID_UserClient::init() {
    DBGLOG("Userclient init");
    
    bool result = false;
    
    result = super::init();
    if (result != true)
    {
        DBGLOG("init() - super::init failed.");
        return false;
    }
    
    ivars = IONewZero(HID_UserClient_IVars, 1);
    if (ivars == nullptr)
    {
        DBGLOG("init() - Failed to allocate memory for ivars.");
        return false;
    }
    
    return result;
}

void HID_UserClient::free() {
    DBGLOG("Userclient free");
    
    OSSafeReleaseNULL(ivars->driver);
    OSSafeReleaseNULL(ivars->callbackAction);
    IOSafeDeleteNULL(ivars, HID_UserClient_IVars, 1);
    
    super::free();
}

kern_return_t IMPL(HID_UserClient, Start) {
    DBGLOG("Userclient start");
    
    if (!super::Start(provider)) {
        DBGLOG("Userclient super::start failed");
        return kIOReturnError;
    }
    
    ivars->driver = OSDynamicCast(HID_Driver, provider);
    ivars->driver->retain();
    if (!ivars->driver) {
        DBGLOG("Userclient null mProvider");
        return kIOReturnError;
    }
    
    return kIOReturnSuccess;
}

kern_return_t IMPL(HID_UserClient, Stop) {
    DBGLOG("Userclient stop");
    return kIOReturnSuccess;
}

kern_return_t HID_UserClient::ExternalMethod(uint64_t selector, IOUserClientMethodArguments* arguments, const IOUserClientMethodDispatch* dispatch, OSObject* target, void* reference) {
    DBGLOG("Userclient external method");
    
    // Check to make sure that the call doesn't interfere with the minimum of the un-checked methods, for the sake of this example.
    // Always check to make sure that the selector is not greater than the number of options in the IOUserClientMethodDispatch.
    if (selector == ExternalMethodType_RegisterAsyncCallback)
    {
        dispatch = &externalMethodChecks[selector];
        if (!target)
        {
            target = this;
        }

        // This will call the functions as defined in the IOUserClientMethodDispatch.
        return super::ExternalMethod(selector, arguments, dispatch, target, reference);
    }

    return kIOReturnSuccess;
}
