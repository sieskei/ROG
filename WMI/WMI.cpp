//
//  WMI.cpp
//  WMI
//
//  Created by Miroslav Yozov on 13.07.23.
//  Copyright © 2023 vit9696. All rights reserved.
//

#include "WMI.hpp"
#include "KeyImplementations.hpp"
#include <Headers/kern_version.hpp>
#include <sys/types.h>

#define super IOService

OSDefineMetaClassAndStructors(WMI, IOService)

bool ADDPR(debugEnabled) = true;
uint32_t ADDPR(debugPrintDelay) = 0;

bool WMI::init(OSDictionary *dict) {
    if (!super::init(dict)) {
        return false;
    }

    return true;
}

bool WMI::start(IOService *provider) {
    if (!provider || !super::start(provider)) {
        return false;
    }
    
    device = (IOACPIPlatformDevice *)provider;

    lilu_os_strncpy(wmi_method, "WMNB", 5);
    
    initATKDevice();
    initEC0Device();
    initBAT0Device();
    
    setFansMode();
    setPowerLimits();
    
    registerVSMC();
    registerService();

    return true;
}

void WMI::stop(IOService *provider) {
    super::stop(provider);
    
    return;
}

IOService *WMI::probe(IOService *provider, SInt32 *score) {
    if (!super::probe(provider, score)) {
        return NULL;
    }

    IOACPIPlatformDevice *dev = OSDynamicCast(IOACPIPlatformDevice, provider);
    if (!dev) {
        return NULL;
    }

    OSObject *obj;
    dev->evaluateObject("_UID", &obj);

    OSString *name = OSDynamicCast(OSString, obj);
    if (!name) {
        return NULL;
    }

    IOService *ret = NULL;
    if (name->isEqualTo("ATK")) {
        ret = this;
    }
    name->release();

    return ret;
}

void WMI::initATKDevice() {
    int init = wmi_evaluate_method(ASUS_WMI_METHODID_INIT, 0, 0);
    setProperty("WMI [init]", init);
    
    int sfun = wmi_evaluate_method(ASUS_WMI_METHODID_SFUN, 0, 0);
    setProperty("WMI [sfun]", sfun);
}

void WMI::initEC0Device() {
    auto dict = IOService::nameMatching("AppleACPIPlatformExpert");
    if (!dict) {
        return;
    }

    auto acpi = IOService::waitForMatchingService(dict);
    dict->release();

    if (!acpi) {
        return;
    }

    acpi->release();

    dict = IOService::nameMatching("PNP0C09");
    if (!dict) {
        return;
    }

    auto deviceIterator = IOService::getMatchingServices(dict);
    dict->release();

    if (!deviceIterator) {
        return;
    }

    ec0 = OSDynamicCast(IOACPIPlatformDevice, deviceIterator->getNextObject());
    deviceIterator->release();
    
    if (ec0) {
        setProperty("EC0 [init]", true);
    }
}

void WMI::initBAT0Device() {
    // Battery Health was introduced in 10.15.5
    // Check if we're on 10.15.5+
    if (getKernelVersion() < KernelVersion::Catalina || (getKernelVersion() == KernelVersion::Catalina && getKernelMinorVersion() < 5)) {
        return;
    }
    
    if (wmi_dev_is_present(ASUS_WMI_DEVID_RSOC)) {
        int rsoc = wmi_evaluate_method(ASUS_WMI_METHODID_DEVS, ASUS_WMI_DEVID_RSOC, 60);
        setProperty("BAT0 [rsoc]", rsoc);
    }
}

void WMI::setPowerLimits() {
    int pl1 = wmi_evaluate_method(ASUS_WMI_METHODID_DEVS, ASUS_WMI_DEVID_PPT_PL1, 125);
    setProperty("WMI [PPT PL1]", pl1);
    
    int pl2 = wmi_evaluate_method(ASUS_WMI_METHODID_DEVS, ASUS_WMI_DEVID_PPT_PL2, 125);
    setProperty("WMI [PPT PL2]", pl2);
}

void WMI::setFansMode() {
    // 0 - balanced
    // 1 - turbo
    // 2 - silent
    int ttp = wmi_evaluate_method(ASUS_WMI_METHODID_DEVS, ASUS_WMI_DEVID_THROTTLE_THERMAL_POLICY, fans_mode);
    setProperty("WMI [TT policy mode]", fans_mode);
    setProperty("WMI [TT policy result]", ttp);
}

int WMI::togglePerformanceMode() {
    switch (fans_mode) {
        case 0:
            fans_mode = 1;
            break;
        case 1:
            fans_mode = 2;
            break;
        case 2:
            fans_mode = 0;
            break;
    }
    
    setFansMode();
    return fans_mode;
}

void WMI::registerVSMC() {
    /* CPU FAN */
    VirtualSMCAPI::addKey(KeyF0Ac(0),
                          vsmcPlugin.data,
                          VirtualSMCAPI::valueWithFp(0, SmcKeyTypeFpe2, new F0Ac(0, &cpuFanSpeed), SMC_KEY_ATTRIBUTE_READ | SMC_KEY_ATTRIBUTE_FUNCTION));
    
    /* GPU FAN */
    VirtualSMCAPI::addKey(KeyF0Ac(1),
                          vsmcPlugin.data,
                          VirtualSMCAPI::valueWithFp(0, SmcKeyTypeFpe2, new F0Ac(1, &gpuFanSpeed), SMC_KEY_ATTRIBUTE_READ | SMC_KEY_ATTRIBUTE_FUNCTION));
    
    uint8_t fanCount = 2;
    VirtualSMCAPI::addKey(KeyFNum,
                          vsmcPlugin.data,
                          VirtualSMCAPI::valueWithUint8(fanCount, nullptr, SMC_KEY_ATTRIBUTE_CONST | SMC_KEY_ATTRIBUTE_READ));
    
    vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);
    
    setProperty("VSMC [register]", true);
}

bool WMI::vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier) {
    if (sensors && vsmc) {
        auto self = static_cast<WMI *>(sensors);
        auto &plugin = static_cast<WMI *>(sensors)->vsmcPlugin;
        auto ret = vsmc->callPlatformFunction(VirtualSMCAPI::SubmitPlugin, true, sensors, &plugin, nullptr, nullptr);
        if (ret == kIOReturnSuccess) {
            
            // atomic_store_explicit(&self->cpuFanSpeed, 1000, memory_order_release);
            // atomic_store_explicit(&self->gpuFanSpeed, 2000, memory_order_release);
            
            self->setProperty("VSMC [submit plugin]", true);
            
            
            // TODO: add poller logic to refresh fans
            /*
            workloop = getWorkLoop();
            poller = IOTimerEventSource::timerEventSource(this, [](OSObject *object, IOTimerEventSource *sender) {
                auto self = OSDynamicCast(WMI, object);
                if (self) {
                    self->setPowerLimits();
                    self->poller->setTimeoutMS(PLTimeoutMS);
                }
            });
            
            workloop->addEventSource(poller);
            poller->setTimeoutMS(PLTimeoutMS);
            */
            
            return true;
        }
    } else {
        SYSLOG("sdell", "got null vsmc notification");
    }
    return false;
}

int WMI::wmi_evaluate_method(uint32_t method_id, uint32_t arg0, uint32_t arg1) {
    OSObject *params[3];
    
    uint32_t arg2 = 0;
    uint32_t arg3 = 0;
    uint32_t arg4 = 0;
    
    struct wmi_args args = {
        .arg0 = arg0,
        .arg1 = arg1,
        .arg2 = arg2,
        .arg3 = arg3,
        .arg4 = arg4,
    };
    
    params[0] = OSNumber::withNumber(static_cast<uint32_t>(0), 32);
    params[1] = OSNumber::withNumber(method_id, 32);
    params[2] = OSData::withBytes(&args, sizeof(wmi_args));

    uint32_t val;
    IOReturn ret = device->evaluateInteger(wmi_method, &val, params, 3);
    params[0]->release();
    params[1]->release();
    params[2]->release();
    
    if (ret != kIOReturnSuccess) {
        return -1;
    }

    if (val == 0xfffffffe) {
        return -1;
    }

    return val;
}

int WMI::wmi_get_devstate(uint32_t dev_id) {
    return wmi_evaluate_method(ASUS_WMI_METHODID_DSTS, dev_id, 0);
}

bool WMI::wmi_dev_is_present(uint32_t dev_id) {
    int status = wmi_evaluate_method(ASUS_WMI_METHODID_DSTS, dev_id, 0);
    return status != -1 && (status & ASUS_WMI_DSTS_PRESENCE_BIT);
}

EXPORT extern "C" kern_return_t ADDPR(kern_start)(kmod_info_t *, void *) {
    // Report success but actually do not start and let I/O Kit unload us.
    // This works better and increases boot speed in some cases.
    lilu_get_boot_args("liludelay", &ADDPR(debugPrintDelay), sizeof(ADDPR(debugPrintDelay)));
    ADDPR(debugEnabled) = checkKernelArgument("-vsmcdbg") || checkKernelArgument("-rogwmidbg") || checkKernelArgument("-liludbgall");
    return KERN_SUCCESS;
}

EXPORT extern "C" kern_return_t ADDPR(kern_stop)(kmod_info_t *, void *) {
    // It is not safe to unload VirtualSMC plugins!
    return KERN_FAILURE;
}
