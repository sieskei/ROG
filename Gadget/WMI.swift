//
//  WMI.swift
//  ROG-HID
//
//  Created by Miroslav Yozov on 15.08.23.
//  Copyright © 2023 Nick. All rights reserved.
//

import Cocoa

enum WMISelector: UInt32 {
    case getVersionLength = 0
    case getVersion = 1
    case toggleThrottleThermalPolicy = 2
}

class WMI {
    static let shared: WMI = .init()
    private var connect: io_connect_t = 0
    
    init() {
        if !self.initDriver() { self.alert("Please download ROG-WMI from the release page", critical: true) }
        // let mode = togglePerformanceMode()
    }

    func getKextVersion() -> (Int, Int) {
        var scalarOut: UInt64 = 0
        var outputCount: UInt32 = 1
        _ = IOConnectCallMethod(
            connect, WMISelector.getVersionLength.rawValue, nil, 0, nil, 0, &scalarOut, &outputCount, nil, nil)

        var outputStrCount: Int = Int(scalarOut)
        var outputStr: [CChar] = [CChar](repeating: 0, count: outputStrCount)
        _ = IOConnectCallMethod(
            connect, WMISelector.getVersion.rawValue, nil, 0, nil, 0, nil, nil, &outputStr, &outputStrCount)

        let version = String(cString: Array(outputStr[0...outputStrCount - 1])).components(separatedBy: ".")
        if version.count <= 1 { self.alert("Invalid kext version", critical: true) }
        return (Int(version[0]) ?? 0, Int(version[1]) ?? 0)
    }
    
    @discardableResult
    func toggleThrottleThermalPolicy() -> Int {
        var scalarOut: UInt64 = 0
        var outputCount: UInt32 = 1

        _ = IOConnectCallMethod(
            connect, WMISelector.toggleThrottleThermalPolicy.rawValue, nil, 0, nil, 0, &scalarOut, &outputCount, nil, nil)
        
        return Int(scalarOut)
    }

    private func initDriver() -> Bool {
        let serviceObject = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceMatching("WMI"))
        if serviceObject == 0 { return false }
        return IOServiceOpen(serviceObject, mach_task_self_, 0, &connect) == KERN_SUCCESS
    }

    private func alert(_ message: String, critical: Bool = false) {
        let alert = NSAlert()
        alert.messageText = critical ? "ROG-WMI communication failure" : "Update Available"
        alert.informativeText = message
        alert.alertStyle = .critical
        alert.addButton(withTitle: critical ? "Quit" : "Dismiss")
        alert.addButton(withTitle: "\(critical ? "Quit and " : "")Download")
        if alert.runModal() == .alertSecondButtonReturn {
            NSWorkspace.shared.open(URL(string: "https://github.com/sieskei/ROG/releases")!)
        }
        if critical { NSApplication.shared.terminate(self) }
    }
}

class AAAAA: NSObject {
    
}
