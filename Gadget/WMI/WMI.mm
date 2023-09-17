#import "WMI.h"

@interface WMI ()

@end

@implementation WMI

+ (instancetype)shared {
    static WMI *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (id)init {
    self = [super init];
    if (self) {
        self.connect = IO_OBJECT_NULL;
        [self initDriver];
    }
    return self;
}

- (int)initDriver {
    io_service_t service = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceNameMatching("WMI"));
    if (service != IO_OBJECT_NULL) {
        io_connect_t connect;
        if (IOServiceOpen(service, mach_task_self_, 0, &connect) == kIOReturnSuccess) {
            self.connect = connect;
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

- (int)getScalar:(WMISelector)selector {
    uint64 scalarOut = 0;
    uint32 outputCount = 1;
    IOConnectCallScalarMethod(self.connect, selector, nil, 0, &scalarOut, &outputCount);
    return (uint) scalarOut;
}

- (uint)getCPUTemp {
    return [self getScalar:getCPUTemp];
}

- (uint)getGPUTemp {
    return [self getScalar:getGPUTemp];
}

- (uint)getCPURpm {
    return [self getScalar:getGPURpm];
}

- (uint)getGPURpm {
    return [self getScalar:getGPURpm];
}

- (int)getThrottleThermalPolicy {
    return [self getScalar:toggleThrottleThermalPolicy];
}

- (int)toggleThrottleThermalPolicy {
    return [self getScalar:toggleThrottleThermalPolicy];
}

@end
