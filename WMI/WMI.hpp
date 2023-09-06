//
//  WMI.hpp
//  WMI
//
//  Created by Miroslav Yozov on 13.07.23.
//  Copyright © 2023 vit9696. All rights reserved.
//

#ifndef WMI_hpp
#define WMI_hpp

#include <IOKit/IOTimerEventSource.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <VirtualSMCSDK/kern_vsmcapi.hpp>

#define ASUS_WMI_METHODID_INIT         0x54494E49
#define ASUS_WMI_METHODID_SFUN         0x4E554653 /* FUNCtionalities */
#define ASUS_WMI_METHODID_DSTS         0x53545344 /* Device status (DSTS) */
#define ASUS_WMI_METHODID_DEVS         0x53564544
#define ASUS_WMI_DSTS_PRESENCE_BIT     0x00010000
#define ASUS_WMI_DEVID_RSOC            0x00120057

#define ASUS_WMI_DEVID_THROTTLE_THERMAL_POLICY 0x00120075
#define ASUS_WMI_DEVID_PPT_PL1                   0x001200A0 // PL1
#define ASUS_WMI_DEVID_PPT_PL2                   0x001200A3 // PL2

class WMI : public IOService {
    OSDeclareDefaultStructors(WMI)
    
    /**
     *  Registered plugin instance
     */
    VirtualSMCAPI::Plugin vsmcPlugin {
        xStringify(PRODUCT_NAME),
        parseModuleVersion(xStringify(MODULE_VERSION)),
        VirtualSMCAPI::Version,
    };
    
    static constexpr uint32_t PLTimeoutMS {1000};
    
public:
    bool init(OSDictionary *dictionary) override;
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
    IOService *probe(IOService *provider, SInt32 *score) override;
    IOReturn message(uint32_t type, IOService *provider, void *argument) override;
    
    IOACPIPlatformDevice *device {nullptr};
    IOACPIPlatformDevice *ec0 {nullptr};
    
    /**
     *  VirtualSMC service registration notifier
     */
    IONotifier *vsmcNotifier {nullptr};
    
private:
    IOWorkLoop *workloop {nullptr};
    IOTimerEventSource *poller {nullptr};
    
    _Atomic(uint16_t) cpuFanSpeed = ATOMIC_VAR_INIT(100);
    _Atomic(uint16_t) gpuFanSpeed = ATOMIC_VAR_INIT(200);
    
    struct wmi_args {
        uint32_t arg0;
        uint32_t arg1;
        uint32_t arg2;
        uint32_t arg3;
        uint32_t arg4;
    } __packed;
    
    char wmi_method[5];
    int wmi_evaluate_method(uint32_t method_id, uint32_t arg0, uint32_t arg1);
    int wmi_get_devstate(uint32_t dev_id);
    bool wmi_dev_is_present(uint32_t dev_id);
    
    int fans_mode {0};
    
    void initATKDevice();
    void initEC0Device();
    void initBAT0Device();
    
    void setPowerLimits();
    void setFansMode();
    
    void registerVSMC();
    
    /**
     *  Submit the keys to received VirtualSMC service.
     *
     *  @param sensors   SMCBatteryManager service
     *  @param refCon    reference
     *  @param vsmc      VirtualSMC service
     *  @param notifier  created notifier
     */
    static bool vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier);
    
public:
    
    int togglePerformanceMode();
};

#endif /* WMI_hpp */
