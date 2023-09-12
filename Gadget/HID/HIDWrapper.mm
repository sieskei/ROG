#import "HIDWrapper.h"

@interface HIDWrapper ()

@property (nonatomic, assign) CFRunLoopRef globalRunLoop;

@end

@implementation HIDWrapper

constexpr uint32_t MessageType_RegisterAsyncCallback = 0;

typedef struct
{
    uint64_t id;
} DataStruct;

- (instancetype)init {
    self = [super init];
    if (self) {
        self.globalRunLoop = CFRunLoopGetCurrent();
    }
    return self;
}

- (int)initDriver {
    io_async_ref64_t refrence = { };
    refrence[kIOAsyncCalloutFuncIndex] = (io_user_reference_t)dispatchKeyboardEvent;
    refrence[kIOAsyncCalloutRefconIndex] = (io_user_reference_t)self;

    IONotificationPortRef notificationPort = IONotificationPortCreate(kIOMainPortDefault);
    if (notificationPort == nullptr) {
        printf("Failed to create notification port for application.\n");
        return EXIT_FAILURE;
    }
    
    mach_port_t machNotificationPort = IONotificationPortGetMachPort(notificationPort);
    if (machNotificationPort == 0) {
        printf("Failed to get mach notification port for application.\n");
        return EXIT_FAILURE;
    }
    
    CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(notificationPort);
    if (runLoopSource == nullptr) {
        printf("Failed to get run loop for application.\n");
        return EXIT_FAILURE;
    }
    
    // Establish our notifications in the run loop, so we can get callbacks.
    CFRunLoopAddSource(self.globalRunLoop, runLoopSource, kCFRunLoopDefaultMode);
    
    io_iterator_t iterator = IO_OBJECT_NULL;
    io_service_t service = IO_OBJECT_NULL;
    
    IOServiceGetMatchingServices(kIOMainPortDefault, IOServiceNameMatching("HID_Driver"), &iterator);
    while ((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL)
    {
        io_connect_t connect;
        if (IOServiceOpen(service, mach_task_self_, 0, &connect) == kIOReturnSuccess) {
            IOConnectCallAsyncStructMethod(connect,
                                           MessageType_RegisterAsyncCallback,
                                           machNotificationPort,
                                           refrence,
                                           kIOAsyncCalloutCount,
                                           nil,
                                           0,
                                           nil,
                                           nil);
        }
    }
    IOObjectRelease(iterator);
    
    return EXIT_SUCCESS;
}

void dispatchKeyboardEvent(void* refcon, IOReturn result, void** args, uint32_t numArgs) {
    printf("called ...");
    uint64_t* arrArgs = (uint64_t*)args;
    
    uint64_t timeStamp = arrArgs[0];
    uint32_t usagePage = (uint32_t) arrArgs[1];
    uint32_t usage = (uint32_t) arrArgs[2];
    uint32_t value = (uint32_t) arrArgs[3];
    
    if (usage == 0xae) { // FAN
        
    }
    
    printf("[dispatchKeyboardEvent] %llu, %d, %d, %d\n", timeStamp, usagePage, usage, value);
}

@end
