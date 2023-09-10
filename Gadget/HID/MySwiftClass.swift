//
//  MySwiftClass.swift
//  Gadget
//
//  Created by Miroslav Yozov on 9.09.23.
//

import Foundation
//import MyWrapper.h

@objc class HID: NSObject {
    let shared: HID = .init()
    
    private let wrapper: HIDWrapper = .init()
    
    private override init() {
        super.init()
    }
}
