//
//  MMIOTools.m
//  Gadget
//
//  Created by Miroslav Yozov on 14.09.23.
//

#import "MMIOTools.h"

@implementation MMIOTools

+ (io_connect_t)getConnectForMMIOAddress:(uint32_t)address {
    io_iterator_t iterator;
    kern_return_t result = IOServiceGetMatchingServices(kIOMainPortDefault, IOServiceMatching("IOPlatformExpertDevice"), &iterator);
    
    if (result != KERN_SUCCESS) {
        NSLog(@"Error: Unable to get matching services");
        return IO_OBJECT_NULL;
    }
    
    io_service_t service = IO_OBJECT_NULL;
    while ((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
        io_connect_t connect;
        result = IOServiceOpen(service, mach_task_self_, 1, &connect);
        
        if (result != KERN_SUCCESS) {
            NSLog(@"Error: Unable to open service");
            IOObjectRelease(service);
            continue;
        }
        
        return connect;
    }
    
    IOObjectRelease(iterator);
    return IO_OBJECT_NULL;
}

+ (uint32_t)readMMIORegister:(io_connect_t)connect atAddress:(mach_vm_address_t)address {
    mach_vm_size_t size = sizeof(uint32_t);
    mach_vm_address_t mapAddress = address;
    kern_return_t result = IOConnectMapMemory(connect, 0, mach_task_self(), &mapAddress, &size, kIOMapAnywhere);
    
    if (result != KERN_SUCCESS) {
        NSLog(@"Error: Unable to map memory");
        return 0;
    }
    
    uint32_t registerValue = *(uint32_t *)mapAddress;
    
    IOConnectUnmapMemory(connect, 0, mach_task_self(), mapAddress);
    
    return registerValue;
}

+ (int)test {
    uint32_t mmioAddress = 0xFED159A0; // Адресът на MMIO регистъра, който искате да прочетете
    io_connect_t connect = [MMIOTools getConnectForMMIOAddress:mmioAddress];
    
    if (connect != IO_OBJECT_NULL) {
        uint32_t registerValue = [MMIOTools readMMIORegister:connect atAddress:mmioAddress];
        NSLog(@"MMIO Register (0x%08x) Value: 0x%08x", mmioAddress, registerValue);
        IOServiceClose(connect);
    } else {
        NSLog(@"Error: Unable to find the IOPlatformExpertDevice service for MMIO address");
    }
    
    return 0;
}

@end
