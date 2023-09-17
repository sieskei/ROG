//
//  MMIOTools.h
//  ROG
//
//  Created by Miroslav Yozov on 14.09.23.
//

#ifndef MMIOTools_h
#define MMIOTools_h

#import <Foundation/Foundation.h>
#import <IOKit/IOKitLib.h>
// #import <IOKit/IOMemoryDescriptor.h>


@interface MMIOTools : NSObject

+ (uint32_t)readMMIORegister:(io_connect_t)connect atAddress:(mach_vm_address_t)address;
+ (io_connect_t)getConnectForMMIOAddress:(uint32_t)address;
+ (int)test;

@end

#endif /* MMIOTools_h */
