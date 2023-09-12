//
//  WMIUC.hpp
//  WMI
//
//  Created by Miroslav Yozov on 15.08.23.
//  Copyright © 2023 vit9696. All rights reserved.
//

#ifndef WMIUC_hpp
#define WMIUC_hpp

#pragma once
#include "WMI.hpp"
#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>

struct WMISelector {
    enum : uint32_t {
        GetVersionLength = 0,
        GetVersion,
        GetCPUTemp,
        GetGPUTemp,
        GetCPURpm,
        GetGPURpm,
        ToggleThrottleThermalPolicy,
    };
};

class WMIUserClient : public IOUserClient {
    OSDeclareDefaultStructors(WMIUserClient);

    private:
    WMI *wmi;
    task_t owningTask;

    public:
    bool initWithTask(task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties) override;
    void stop(IOService *provider) override;
    bool start(IOService *provider) override;
    IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments *arguments, IOExternalMethodDispatch *dispatch,
        OSObject *target, void *reference) override;
};

#endif /* WMIUC_hpp */
