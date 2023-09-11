#import "HIDWrapper.h"

@interface HIDWrapper ()

@property io_connect_t connect;
@property (nonatomic, assign) io_async_ref64_t *refrence;

@property (nonatomic, assign) CFRunLoopRef globalRunLoop;
@property (nonatomic, assign) CFRunLoopSourceRef runLoopSource;

@end

@implementation HIDWrapper

constexpr uint32_t MessageType_RegisterAsyncCallback = 0;

- (instancetype)init {
    self = [super init];
    if (self) {
        self.connect = IO_OBJECT_NULL;
        self.refrence = nullptr;
        self.globalRunLoop = nullptr;
        self.runLoopSource = nullptr;
    }
    return self;
}

- (int)initDriver {
    io_service_t service = IO_OBJECT_NULL;
    service = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceNameMatching("HID_Driver"));
    if (service == IO_OBJECT_NULL) {
        printf("Failed to match to device.\n");
        return EXIT_FAILURE;
    }
    
    io_connect_t connect = IO_OBJECT_NULL;
    if (IOServiceOpen(service, mach_task_self_, 0, &connect) != KERN_SUCCESS) {
        return EXIT_FAILURE;
    }
    self.connect = connect;
    
    io_async_ref64_t refrence = { };
    refrence[kIOAsyncCalloutFuncIndex] = (io_user_reference_t)AsyncCallback;
    refrence[kIOAsyncCalloutRefconIndex] = (io_user_reference_t)nullptr;
    self.refrence = &refrence;
    
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
    self.runLoopSource = runLoopSource;
    
    // Async initialization
    self.globalRunLoop = CFRunLoopGetCurrent();
    CFRetain(self.globalRunLoop);
    
    // Establish our notifications in the run loop, so we can get callbacks.
    CFRunLoopAddSource(self.globalRunLoop, runLoopSource, kCFRunLoopDefaultMode);
    
    
    kern_return_t ret = IOConnectCallAsyncStructMethod(connect,
                                                       MessageType_RegisterAsyncCallback,
                                                       machNotificationPort,
                                                       refrence,
                                                       kIOAsyncCalloutCount,
                                                       nil,
                                                       0,
                                                       nil,
                                                       nil);
    if (ret != kIOReturnSuccess) {
        printf("IOConnectCallStructMethod failed with error: 0x%08x.\n", ret);
    }
    
    return EXIT_SUCCESS;
}

- (void)dealloc {
    
}

static void AsyncCallback(void* refcon, IOReturn result, void** args, uint32_t numArgs) {
    printf("called ...");
    
    
}

@end
