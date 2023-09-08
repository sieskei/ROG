//
//  HID_Driver.cpp
//  HID-Driver
//
//  Created by Nick on 7/20/20.
//  Copyright © 2020 Nick. All rights reserved.
//

#include <HIDDriverKit/HIDDriverKit.h>
#include <DriverKit/IOUserClient.h>

#include "HID_Driver.h"
#include "HIDUsageTables.h"
#include "IOBufferMemoryDescriptorUtility.h"

#undef super
#define super IOUserHIDEventDriver

struct HID_Driver_IVars
{
    IOHIDInterface* hid_interface           { nullptr };
    OSArray* customKeyboardElements         { nullptr };
    uint8_t kbdLux                          { 0 };
    bool fixCapsLockLED                     { false };
    bool bkltAutoTurnOff                    { false };
};

#define _hid_interface              ivars->hid_interface
#define _custom_keyboard_elements   ivars->customKeyboardElements
#define _current_lux                ivars->kbdLux

bool HID_Driver::init()
{
    DBGLOG("Init");

    if (!super::init())
        return false;

    ivars = IONewZero(HID_Driver_IVars, 1);

    if (ivars == nullptr)
        return false;

    ivars->fixCapsLockLED = false;
    ivars->bkltAutoTurnOff = false;
    _current_lux = 0;

    return true;
}

kern_return_t IMPL(HID_Driver, Start)
{
    DBGLOG("Start");
    kern_return_t ret;
    ret = Start(provider, SUPERDISPATCH);

    if (ret != kIOReturnSuccess)
    {
        OSLOG("Start failed %d", ret);
        return ret;
    }

    IOHIDInterface* interface = OSDynamicCast(IOHIDInterface, provider);
    if (!interface)
    {
        OSLOG("Failure in casting provider to IOHIDInterface");
        return kIOReturnError;
    }

    _hid_interface = interface;

    OSArray *elements = _hid_interface->getElements();
    elements->retain();
    if (elements)
        parseKeyboardElementsHook(elements);
    elements->release();

    // Now we init the keyboard
    rogNkeyLedInit();

    // Parse Info.plist
    parseInfoPlist();

    // Reflect the intital value on lux
    DBGLOG("Trying to set initial kbd lux to: 0x%x", _current_lux);
    setBacklight(_current_lux);

    // And register ourselves with the system
    DBGLOG("Register service");
    RegisterService();
    
    
    
    

    return ret;
}

void IMPL(HID_Driver, parseKeyboardElementsHook)
{
    DBGLOG("Parse keyboard element hook called");
    _custom_keyboard_elements = OSArray::withCapacity(4);
    uint32_t count, index;
    for (index = 0, count = elements->getCount(); index < count; index++)
    {
        IOHIDElement* element = OSDynamicCast(IOHIDElement, elements->getObject(index));
        if (!element || element->getUsage() == 0)
            continue;
        if (element->getType() == kIOHIDElementTypeCollection)
            continue;

        uint32_t usage, usagePage;
        usage = element->getUsage();
        usagePage = element->getUsagePage();
        bool store = false;

        switch (usagePage)
        {
            case kHIDPage_AsusVendor:
                switch (usage)
                {
                    case kHIDUsage_AsusVendor_BrightnessUp:
                    case kHIDUsage_AsusVendor_BrightnessDown:
                    case kHIDUsage_AsusVendor_DisplayOff:
                    case kHIDUsage_AsusVendor_ROG:
                    case kHIDUsage_AsusVendor_Power4Gear:
                    case kHIDUsage_AsusVendor_TouchpadToggle:
                    case kHIDUsage_AsusVendor_Sleep:
                    case kHIDUsage_AsusVendor_MicMute:
                    case kHIDUsage_AsusVendor_Camera:
                    case kHIDUsage_AsusVendor_RFKill:
                    case kHIDUsage_AsusVendor_Fan:
                    case kHIDUsage_AsusVendor_Calc:
                    case kHIDUsage_AsusVendor_Splendid:
                    case kHIDUsage_AsusVendor_IlluminationUp:
                    case kHIDUsage_AsusVendor_IlluminationDown:
                        store = true;
                        break;
                }
                break;
            case kHIDPage_MicrosoftVendor:
                switch (usage)
                {
                    case kHIDUsage_MicrosoftVendor_WLAN:
                    case kHIDUsage_MicrosoftVendor_BrightnessDown:
                    case kHIDUsage_MicrosoftVendor_BrightnessUp:
                    case kHIDUsage_MicrosoftVendor_DisplayOff:
                    case kHIDUsage_MicrosoftVendor_Camera:
                    case kHIDUsage_MicrosoftVendor_ROG:
                        store = true;
                        break;
                }
                break;
        }
        if (store)
            _custom_keyboard_elements->setObject(element);
    }
}

void HID_Driver::handleKeyboardReport(uint64_t timestamp, uint32_t reportID)
{
    for (unsigned int i = 0; i < _custom_keyboard_elements->getCount(); i++)
    {
        IOHIDElement* element { nullptr };
        uint64_t elementTimeStamp;
        uint32_t usagePage, usage, value, preValue;

        element = OSDynamicCast(IOHIDElement, _custom_keyboard_elements->getObject(i));

        if (!element || element->getReportID() != reportID)
            continue;

        elementTimeStamp = element->getTimeStamp();
        if (timestamp != elementTimeStamp)
            continue;

        usagePage = element->getUsagePage();
        usage = element->getUsage();

        preValue = element->getValue(kIOHIDValueOptionsFlagPrevious) != 0;

        // Fix for double reports of KBD illumination
        if (usagePage == kHIDPage_AsusVendor && (usage == kHIDUsage_AsusVendor_IlluminationUp || usage == kHIDUsage_AsusVendor_IlluminationDown))
        {
            value = element->getValue(kIOHIDValueOptionsFlagRelativeSimple) != 0;
        }
        else
        {
            value = element->getValue(0) != 0;
        }

        if (value == preValue)
            continue;

        DBGLOG("Handle Key Report - usage: 0x%x, page: 0x%x, val: 0x%x", usage, usagePage, value);
        dispatchKeyboardEvent(timestamp, usagePage, usage, value, 0, false);
        return;
    }

    // Pass the rest of the reports to super for handling
    super::handleKeyboardReport(timestamp, reportID);
}

kern_return_t HID_Driver::dispatchKeyboardEvent(uint64_t timeStamp, uint32_t usagePage, uint32_t usage, uint32_t value, IOOptionBits options, bool repeat)
{
    if (usagePage == kHIDPage_AsusVendor)
    {
        switch (usage) {
            case kHIDUsage_AsusVendor_BrightnessDown:
                usagePage = kHIDPage_AppleVendorTopCase;
                usage = kHIDUsage_AV_TopCase_BrightnessDown;
                break;
            case kHIDUsage_AsusVendor_BrightnessUp:
                usagePage = kHIDPage_AppleVendorTopCase;
                usage = kHIDUsage_AV_TopCase_BrightnessUp;
                break;
            case kHIDUsage_AsusVendor_IlluminationDown:
                setKbdLux(luxDown);
                break;
            case kHIDUsage_AsusVendor_IlluminationUp:
                setKbdLux(luxUp);
                break;
        }
    }
    if (usagePage == kHIDPage_MicrosoftVendor)
    {
        switch (usage) {
            case kHIDUsage_MicrosoftVendor_BrightnessDown:
                usagePage = kHIDPage_AppleVendorTopCase;
                usage = kHIDUsage_AV_TopCase_BrightnessDown;
                break;
            case kHIDUsage_MicrosoftVendor_BrightnessUp:
                usagePage = kHIDPage_AppleVendorTopCase;
                usage = kHIDUsage_AV_TopCase_BrightnessUp;
                break;
        }
    }

    // Fix erratic caps lock key
    if (usage == kHIDUsage_KeyboardCapsLock && ivars->fixCapsLockLED)
        IOSleep(80);

    DBGLOG("Dispatch Event - usage: 0x%x, page: 0x%x, val: 0x%x", usage, usagePage, value);
    return super::dispatchKeyboardEvent(timeStamp, usagePage, usage, value, options, repeat);
}

void IMPL(HID_Driver, setKbdLux)
{
    if (luxEvent == luxUp)
    {
        if (_current_lux == 3)
            return;
        setBacklight(++_current_lux);
        return;
    }

    if (luxEvent == luxDown)
    {
        if (_current_lux == 0)
            return;
        setBacklight(--_current_lux);
        return;
    }
}

void IMPL(HID_Driver, parseInfoPlist)
{
    DBGLOG("Parse custom Info.plist properties");
    OSContainer* propContainer{nullptr};
    OSNumber* containerVal{nullptr};

    SearchProperty("FixCapsLockLED", "IOService", kIOServiceSearchPropertyParents, &propContainer);
    if (!propContainer)
        goto exit;

    containerVal = OSDynamicCast(OSNumber, propContainer);
    if (containerVal == nullptr)
        goto exit;

    ivars->fixCapsLockLED = containerVal->unsigned8BitValue() == kBooleanTrue;
    DBGLOG("Fix caps lock led: %s", ivars->fixCapsLockLED ? "True" : "False");

    SearchProperty("BacklightAutoTurnOff", "IOService", kIOServiceSearchPropertyParents, &propContainer);
    if (!propContainer)
        goto exit;

    containerVal = OSDynamicCast(OSNumber, propContainer);
    if (containerVal == nullptr)
        goto exit;

    ivars->bkltAutoTurnOff = containerVal->unsigned8BitValue() == kBooleanTrue;
    DBGLOG("Backlight auto turn off: %s", ivars->bkltAutoTurnOff ? "True" : "False");

exit:
    OSSafeReleaseNULL(propContainer);
    DBGLOG("Done parsing custom Info.plist properties");
}

kern_return_t IMPL(HID_Driver, Stop)
{
    DBGLOG("Stop");
    return Stop(provider, SUPERDISPATCH);
}

void HID_Driver::free()
{
    DBGLOG("Free");
    if (ivars != nullptr)
    {
        OSSafeReleaseNULL(_hid_interface);
        OSSafeReleaseNULL(_custom_keyboard_elements);
    }

    IOSafeDeleteNULL(ivars, HID_Driver_IVars, 1);
    super::free();
}

#pragma mark
#pragma mark Ported from hid-asus.c
#pragma mark

void IMPL(HID_Driver, rogNkeyLedInit)
{
    DBGLOG("initialise N-KEY Device");

    uint8_t buf_init_start[] = { KBD_FEATURE_REPORT_ID1, 0xb9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t buf_init2[]      = { KBD_FEATURE_REPORT_ID1, 0x41, 0x53, 0x55, 0x53, 0x20, 0x54, 0x65, 0x63, 0x68, 0x2e, 0x49, 0x6e, 0x63, 0x2e, 0x00 };
    uint8_t buf_init3[]      = { KBD_FEATURE_REPORT_ID1, 0x05, 0x20, 0x31, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    //                                                                           0x1a // G-Helper

    OSData* data = nullptr;

    /* The first message is an init start */
    data = OSData::withBytes(buf_init_start, KBD_FEATURE_REPORT_SIZE);
    if (setReport(KBD_FEATURE_REPORT_ID1, data) != kIOReturnSuccess) {
        OSLOG("failed to send init start command");
        goto exit;
    }
    data->release();

    /* Followed by a string */
    data = OSData::withBytes(buf_init2, KBD_FEATURE_REPORT_SIZE);
    if (setReport(KBD_FEATURE_REPORT_ID1, data) != kIOReturnSuccess) {
        OSLOG("failed to send init command 1.0");
        goto exit;
    }
    data->release();

    /* Followed by a string */
    data = OSData::withBytes(buf_init3, KBD_FEATURE_REPORT_SIZE);
    if (setReport(KBD_FEATURE_REPORT_ID1, data) != kIOReturnSuccess) {
        OSLOG("failed to send init command 1.1");
        goto exit;
    }
    data->release();

    /* begin second report ID with same data */
    buf_init2[0] = KBD_FEATURE_REPORT_ID2;
    buf_init3[0] = KBD_FEATURE_REPORT_ID2;

    data = OSData::withBytes(buf_init2, KBD_FEATURE_REPORT_SIZE);
    if (setReport(KBD_FEATURE_REPORT_ID2, data) != kIOReturnSuccess) {
        OSLOG("failed to send init command 2.0");
        goto exit;
    }
    data->release();

    data = OSData::withBytes(buf_init3, KBD_FEATURE_REPORT_SIZE);
    if (setReport(KBD_FEATURE_REPORT_ID2, data) != kIOReturnSuccess) {
        OSLOG("failed to send init command 2.1");
        goto exit;
    }
    data->release();

    return;

exit:
    data->release();
    return;
}

void IMPL(HID_Driver, setBacklight)
{
    DBGLOG("Setting keyboard Lux to: %d", val);

    uint8_t buf[] = { KBD_FEATURE_REPORT_ID, 0xba, 0xc5, 0xc4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    buf[4] = val;
    OSData* data = OSData::withBytes(buf, KBD_FEATURE_REPORT_SIZE);
    setReport(KBD_FEATURE_REPORT_ID, data);
    data->release();
}

kern_return_t IMPL(HID_Driver, setReport)
{
    IOMemoryDescriptor* report = nullptr;
    auto ret = IOBufferMemoryDescriptorUtility::createWithBytes(data->getBytesNoCopy(0, KBD_FEATURE_REPORT_SIZE), data->getLength(), &report);
    if (ret == kIOReturnSuccess)
    {
        _hid_interface->SetReport(report, kIOHIDReportTypeFeature, id, 0);
    } else {
        OSLOG("Error allocating memory for report");
    }

    report->release();

    return ret;
}

kern_return_t IMPL(HID_Driver, NewUserClient) {
    IOService *client {nullptr};

    auto ret = Create(this, "UserClientProperties", &client);
    if (ret != kIOReturnSuccess) {
        DBGLOG("Newuserclient create failed");

        return ret;
    }

    DBGLOG("Created user client successfully");

    *userClient = OSDynamicCast(IOUserClient, client);

    if (!*userClient) {
        DBGLOG("Dynamic cast failed");
        client->release();

        return kIOReturnError;
    }

    return kIOReturnSuccess;
}
