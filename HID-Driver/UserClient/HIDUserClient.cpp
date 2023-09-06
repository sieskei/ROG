//
//  HIDUserClient.cpp
//  HID-Driver
//
//  Created by Nick on 2/16/21.
//  Copyright © 2021 Nick. All rights reserved.
//

#include "HIDUserClient.h"
#include "HID_Driver.h"

bool HIDUserClient::init() {
    DBGLOG("Userclient init");

    return super::init();
}

void HIDUserClient::free() {
    DBGLOG("Userclient free");
    super::free();
}

kern_return_t IMPL(HIDUserClient, Start) {
    DBGLOG("Userclient start");
    
    return kIOReturnSuccess;
}

kern_return_t IMPL(HIDUserClient, Stop) {
    DBGLOG("Userclient stop");
    return kIOReturnSuccess;
}

kern_return_t HIDUserClient::ExternalMethod(uint64_t selector, IOUserClientMethodArguments* arguments, const IOUserClientMethodDispatch* dispatch, OSObject* target, void* reference) {
    DBGLOG("Userclient external method");
    return kIOReturnSuccess;
}
