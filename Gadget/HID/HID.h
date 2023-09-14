//
//  MyWrapper.h
//  Gadget
//
//  Created by Miroslav Yozov on 9.09.23.
//

#ifndef HID_h
#define HID_h

#import <Foundation/Foundation.h>

@interface HID : NSObject

+ (instancetype)shared;

- (instancetype)init;
- (int)initDriver;

@end

#endif /* HID_h */
