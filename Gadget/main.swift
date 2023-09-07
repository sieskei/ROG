//
//  main.swift
//  Gadget
//
//  Created by Miroslav Yozov on 6.09.23.
//

import AppKit

let delegate = AppDelegate()
let app = NSApplication.shared
app.delegate = delegate
_ = NSApplicationMain(CommandLine.argc, CommandLine.unsafeArgv)
