//
//  WMIUC.cpp
//  WMI
//
//  Created by Miroslav Yozov on 15.08.23.
//  Copyright © 2023 vit9696. All rights reserved.
//

#include "WMIUC.hpp"
#include <Headers/kern_util.hpp>
#include <IOKit/IOLib.h>

#define super IOUserClient

OSDefineMetaClassAndStructors(WMIUserClient, IOUserClient);

bool WMIUserClient::initWithTask(task_t owningTask, void *securityToken, UInt32 type, OSDictionary *) {
    if (!owningTask) return false;

    if (!super::initWithTask(owningTask, securityToken, type)) {
        SYSLOG("WMIUC", "super::initWithTask failed\n");
        return false;
    }

    this->owningTask = owningTask;
    this->wmi = nullptr;
    
    setProperty("WMIUC [initWithTask]", true);

    return true;
}

bool WMIUserClient::start(IOService *provider) {
    if (!super::start(provider)) {
        SYSLOG("WMIUC", "super::start failed\n");
        return false;
    }

    this->wmi = OSDynamicCast(WMI, provider);
    if (!this->wmi) {
        SYSLOG("WMIUC", "null mProvider\n");
        return false;
    }

    setProperty("WMIUC [start]", true);
    
    return true;
}

void WMIUserClient::stop(IOService *provider) {
    this->wmi = nullptr;
    super::stop(provider);
    
    setProperty("WMIUC [stop]", true);
}

IOReturn WMIUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *arguments,
    IOExternalMethodDispatch *, OSObject *, void *) {
    static char version[] = xStringify(MODULE_VERSION);
    switch (selector) {
        case WMISelector::GetVersionLength: {
            arguments->scalarOutputCount = 1;
            *arguments->scalarOutput = sizeof(version);
            arguments->structureOutputSize = 0;
            break;
        }
        case WMISelector::GetVersion: {
            arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = sizeof(version);
            memcpy(arguments->structureOutput, version, sizeof(version));
            break;
        }
        case WMISelector::GetCPUTemp: {
            arguments->scalarOutputCount = 1;
            *arguments->scalarOutput = wmi->getCPUTemp();
            arguments->structureOutputSize = 0;
            break;
        }
        case WMISelector::GetGPUTemp: {
            arguments->scalarOutputCount = 1;
            *arguments->scalarOutput = wmi->getGPUTemp();
            arguments->structureOutputSize = 0;
            break;
        }
        case WMISelector::GetCPURpm: {
            arguments->scalarOutputCount = 1;
            *arguments->scalarOutput = wmi->getCPURpm();
            arguments->structureOutputSize = 0;
            break;
        }
        case WMISelector::GetGPURpm: {
            arguments->scalarOutputCount = 1;
            *arguments->scalarOutput = wmi->getGPURpm();
            arguments->structureOutputSize = 0;
            break;
        }
        case WMISelector::GetThrottleThermalPolicy: {
            arguments->scalarOutputCount = 1;
            *arguments->scalarOutput = wmi->getThrottleThermalPolicy();
            arguments->structureOutputSize = 0;
            break;
        }
        case WMISelector::ToggleThrottleThermalPolicy: {
            arguments->scalarOutputCount = 1;
            *arguments->scalarOutput = wmi->toogleThrottleThermalPolicy();
            arguments->structureOutputSize = 0;
            break;
        }
    }

    return kIOReturnSuccess;
}
