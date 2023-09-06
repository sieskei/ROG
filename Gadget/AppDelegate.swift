//
//  AppDelegate.swift
//  Gadget
//
//  Created by Miroslav Yozov on 6.09.23.
//

import Cocoa

class AppDelegate: NSObject, NSApplicationDelegate {
    var statusBar: StatusBarController?

    static var shared: AppDelegate {
        guard let delegate = NSApp.delegate as? AppDelegate else { fatalError("Invalid delegate") }
        return delegate
    }

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        NSApp.setActivationPolicy(NSApplication.ActivationPolicy.accessory)
        statusBar = .init()
        
        let _ = WMI.shared
    }

    func applicationWillTerminate(_ aNotification: Notification) { }

    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool { true }
}

