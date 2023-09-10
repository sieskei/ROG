//
//  HID.swift
//  Gadget
//
//  Created by Miroslav Yozov on 9.09.23.
//

import Foundation

@objc class HID: NSObject {
    static let shared: HID = .init()
    
    private let wrapper: HIDWrapper = .init()
    
    private override init() {
        super.init()
    }
    
    @discardableResult
    func initDriver() -> Bool {
        wrapper.initDriver()
    }
}
