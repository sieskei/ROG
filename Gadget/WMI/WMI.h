//
//  WMIWrapper.h
//  ROG
//
//  Created by Miroslav Yozov on 13.09.23.
//

#ifndef WMIWrapper_h
#define WMIWrapper_h

#import <Foundation/Foundation.h>

typedef enum {
    getVersionLength = 0,
    getVersion,
    getCPUTemp,
    getGPUTemp,
    getCPURpm,
    getGPURpm,
    getThrottleThermalPolicy,
    toggleThrottleThermalPolicy
} WMISelector;

@interface WMI : NSObject

@property (nonatomic, assign) io_connect_t connect;

+ (instancetype)shared;

- (instancetype)init;
- (int)initDriver;

- (uint)getCPUTemp;
- (uint)getGPUTemp;
- (uint)getCPURpm;
- (uint)getGPURpm;
- (int)getThrottleThermalPolicy;
- (int)toggleThrottleThermalPolicy;

@end

#endif /* WMIWrapper_h */
