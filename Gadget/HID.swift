//
//  HID.swift
//  Gadget
//
//  Created by Miroslav Yozov on 8.09.23.
//

import Foundation

class HID {
    static let shared: HID = .init()
    
    private var connect: io_connect_t = 0
    private var port: mach_port_t = 0
    var asyncRef: io_async_ref64_t = (0, 0, 0, 0, 0, 0, 0, 0)
    
    init() {
        if !initDriver() {
            print("Unable to init driver.")
        }
    }
    
    private func initDriver() -> Bool {
        let serviceObject = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceNameMatching("HID_Driver"))
        if serviceObject == 0 {
            return false
        }
        
        if IOServiceOpen(serviceObject, mach_task_self_, 0, &connect) != KERN_SUCCESS {
            return false
        }
        
        let notificationPort = IONotificationPortCreate(kIOMainPortDefault)
        if notificationPort == nil {
            return false
        }

        port = IONotificationPortGetMachPort(notificationPort);
        if port == 0 {
            return false
        }
        
        // asyncRef.1 =
        // asyncRef.2 = unsafeBitCast(nil, to: io_user_reference_t.self)
       
        
        
//        let _ = IOConnectCallAsyncMethod(connect, 0, port, &a, 1, nil, 0, nil, 0, nil, nil, nil, nil)
//
//        IOConnectCallAsyncScalarMethod(<#T##connection: mach_port_t##mach_port_t#>, <#T##selector: UInt32##UInt32#>, <#T##wake_port: mach_port_t##mach_port_t#>, <#T##reference: UnsafeMutablePointer<UInt64>!##UnsafeMutablePointer<UInt64>!#>, <#T##referenceCnt: UInt32##UInt32#>, <#T##input: UnsafePointer<UInt64>!##UnsafePointer<UInt64>!#>, <#T##inputCnt: UInt32##UInt32#>, <#T##output: UnsafeMutablePointer<UInt64>!##UnsafeMutablePointer<UInt64>!#>, <#T##outputCnt: UnsafeMutablePointer<UInt32>!##UnsafeMutablePointer<UInt32>!#>)
//
//        IOConnectCallAsyncScalarMethod(<#T##connection: mach_port_t##mach_port_t#>, <#T##selector: UInt32##UInt32#>, <#T##wake_port: mach_port_t##mach_port_t#>, <#T##reference: UnsafeMutablePointer<UInt64>!##UnsafeMutablePointer<UInt64>!#>, <#T##referenceCnt: UInt32##UInt32#>, <#T##input: UnsafePointer<UInt64>!##UnsafePointer<UInt64>!#>, <#T##inputCnt: UInt32##UInt32#>, <#T##output: UnsafeMutablePointer<UInt64>!##UnsafeMutablePointer<UInt64>!#>, <#T##outputCnt: UnsafeMutablePointer<UInt32>!##UnsafeMutablePointer<UInt32>!#>)
        
        return true
    }
    
    static func AsyncCallback(refcon: UnsafeMutableRawPointer?, result: IOReturn, args: UnsafeMutablePointer<UnsafeMutableRawPointer?>?, numArgs: UInt32) {
        // Your implementation here
    }
}


/*
IOConnectCallMethod(<#T##connection: mach_port_t##mach_port_t#>, <#T##selector: UInt32##UInt32#>, <#T##input: UnsafePointer<UInt64>!##UnsafePointer<UInt64>!#>, <#T##inputCnt: UInt32##UInt32#>, <#T##inputStruct: UnsafeRawPointer!##UnsafeRawPointer!#>, <#T##inputStructCnt: Int##Int#>, <#T##output: UnsafeMutablePointer<UInt64>!##UnsafeMutablePointer<UInt64>!#>, <#T##outputCnt: UnsafeMutablePointer<UInt32>!##UnsafeMutablePointer<UInt32>!#>, <#T##outputStruct: UnsafeMutableRawPointer!##UnsafeMutableRawPointer!#>, <#T##outputStructCnt: UnsafeMutablePointer<Int>!##UnsafeMutablePointer<Int>!#>)

IOConnectCallAsyncStructMethod(<#T##connection: mach_port_t##mach_port_t#>, <#T##selector: UInt32##UInt32#>, <#T##wake_port: mach_port_t##mach_port_t#>, <#T##reference: UnsafeMutablePointer<UInt64>!##UnsafeMutablePointer<UInt64>!#>, <#T##referenceCnt: UInt32##UInt32#>, <#T##inputStruct: UnsafeRawPointer!##UnsafeRawPointer!#>, <#T##inputStructCnt: Int##Int#>, <#T##outputStruct: UnsafeMutableRawPointer!##UnsafeMutableRawPointer!#>, <#T##outputStructCnt: UnsafeMutablePointer<Int>!##UnsafeMutablePointer<Int>!#>)
*/
