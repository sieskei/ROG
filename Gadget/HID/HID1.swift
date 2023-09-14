//
//  HID.swift
//  Gadget
//
//  Created by Miroslav Yozov on 9.09.23.
//

import Foundation

@objc class HID1: NSObject {
    static let shared: HID1 = .init()
    
    private let wrapper: HID = .init()
    
    private override init() {
        super.init()
    }
    
    @discardableResult
    func initDriver() -> Bool {
        wrapper.initDriver() == .zero
    }
}
