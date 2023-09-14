//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

import AppKit
import Cocoa
import Foundation
import SwiftUI

private class StatusbarView: NSView {
    private var normalLabel: [NSAttributedString.Key: NSObject]?
    private var compactLabel: [NSAttributedString.Key: NSObject]?
    private var normalValue: [NSAttributedString.Key: NSObject]?

    func setup() {
        compactLabel = [
            .font: NSFont.monospacedSystemFont(ofSize: 7.2, weight: .regular), .foregroundColor: NSColor.labelColor,
        ]
        normalValue = [
            .font: NSFont.systemFont(ofSize: 11, weight: .regular), .foregroundColor: NSColor.labelColor,
        ]
    }
    
    func drawMultiline(_ label: String, _ value: String, x: CGFloat) {
        NSAttributedString(string: label, attributes: compactLabel)
            .draw(at: NSPoint(x: x, y: 11))
        
        let str = NSAttributedString(string: value, attributes: normalValue)
        print(NSStringFromSize(str.size()))
        str
            .draw(at: NSPoint(x: x, y: 0))
    }
    
    func drawIcon(x: CGFloat) {
        let attach = NSTextAttachment()
        attach.image = NSImage(named: "asus_rog_red_24p")
        
        let str = NSAttributedString(attachment: attach)
        print(NSStringFromSize(str.size()))
        str
            .draw(at: .init(x: x, y: 2.5))
    }

    override func draw(_ dirtyRect: NSRect) {
        guard (NSGraphicsContext.current?.cgContext) != nil,
              let wmi = WMI.shared() else {
            return
        }
        
        drawIcon(x: .zero)
        drawMultiline("CPU", "\(wmi.getCPUTemp())°C (\(wmi.getCPURpm()))", x: 32)
        drawMultiline("GPU", "\(wmi.getGPUTemp())°C (\(wmi.getCPURpm()))", x: 106)
    }
}

class StatusBarController {
    private var statusItem: NSStatusItem!
    private var view: StatusbarView!
    private var popover: NSPopover
    private var updateTimer: Timer?

    init() {
        self.statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        self.statusItem.isVisible = true
        self.statusItem.length = 172

        self.view = StatusbarView()
        self.view.setup()

        self.popover = NSPopover()
        self.popover.contentViewController = NSHostingController(rootView: PopupView())

        if let statusBarButton = self.statusItem.button {
            self.view.frame = statusBarButton.bounds
            statusBarButton.wantsLayer = true
            statusBarButton.addSubview(view)
            statusBarButton.action = #selector(togglePopover(sender:))
            statusBarButton.target = self
        }

        self.updateTimer = Timer.scheduledTimer(withTimeInterval: 2, repeats: true, block: { _ in self.update() })
    }

    func update() {
        self.view.setNeedsDisplay(self.view.frame)
    }

    func dismiss() {
        self.updateTimer?.invalidate()
        NSStatusBar.system.removeStatusItem(self.statusItem!)
        self.statusItem = nil
    }

    @objc func togglePopover(sender: AnyObject) {
        if self.popover.isShown {
            self.popover.performClose(sender)
        } else if let statusBarButton = self.statusItem.button {
            self.popover.show(relativeTo: statusBarButton.bounds, of: statusBarButton, preferredEdge: NSRectEdge.minY)
        }
    }
}

struct PopupView: View {
    var body: some View {
        VStack {
            HStack {
                Image(nsImage: NSImage(named: "AppIcon")!)
                    .resizable()
                    .frame(width: 32.0, height: 32.0)
                    .cornerRadius(8)
                Text("ROG Gadget").font(.title)
            }
            Spacer().frame(height: 16)
            Button(action: {
                WMI1.shared.toggleThrottleThermalPolicy()
            }) {
                HStack {
                    Image("fan")
                        .renderingMode(.template)
                        .resizable()
                        .scaledToFit()
                        .frame(width: 24, height: 24)
                    Text("Fan Mode")
                        .fontWeight(.bold)
                }
                .padding(6)
                .overlay(RoundedRectangle(cornerRadius: 8)
                    .stroke(lineWidth: 1))
            }
            .buttonStyle(.plain)
            Spacer().frame(height: 32)
            Button("Exit", action: { exit(0) })
        }
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0).padding(12.0)
    }
}
