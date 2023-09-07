//  ContentView.swift
//  HID-Installer
//
//  Created by Miroslav Yozov on 6.09.23.
//


import Foundation
import SystemExtensions
import os.log

class ExtensionManager: NSObject, OSSystemExtensionRequestDelegate {
    
    private let extensionBundleID = "com.sieskei.ROG-HID-Driver"
    
    // Private init
    override private init() {
        super.init()
    }
    
    // Shared singleton
    static let shared = ExtensionManager()
    
    // Activate
    func activate() -> Void {
        let req = OSSystemExtensionRequest.activationRequest(forExtensionWithIdentifier: extensionBundleID, queue: .main)
        req.delegate = self
        OSSystemExtensionManager.shared.submitRequest(req)
    }
    
    // Deactivate
    func deactivate() -> Void {
        let req = OSSystemExtensionRequest.deactivationRequest(forExtensionWithIdentifier: extensionBundleID, queue: .main)
        req.delegate = self
        OSSystemExtensionManager.shared.submitRequest(req)
    }
    
}

// MARK:- OSSystemExtensionRequest Delegate Methods
extension ExtensionManager {
    func request(_ request: OSSystemExtensionRequest, actionForReplacingExtension existing: OSSystemExtensionProperties, withExtension ext: OSSystemExtensionProperties) -> OSSystemExtensionRequest.ReplacementAction {
        return .cancel
    }
    
    func requestNeedsUserApproval(_ request: OSSystemExtensionRequest) {
        os_log("ROG-HID-Driver :: Dext loading requires user approval")
    }
    
    func request(_ request: OSSystemExtensionRequest, didFinishWithResult result: OSSystemExtensionRequest.Result) {
        os_log("ROG-HID-Driver :: Dext loading finished with result: %@", result.rawValue)
    }
    
    func request(_ request: OSSystemExtensionRequest, didFailWithError error: Error) {
        os_log("ROG-HID-Driver :: Dext loading failed with error: %@", error.localizedDescription)
    }
}
