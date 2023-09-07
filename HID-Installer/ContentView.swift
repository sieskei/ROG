//
//  ContentView.swift
//  HID-Installer
//
//  Created by Miroslav Yozov on 6.09.23.
//

import SwiftUI
import SystemExtensions

struct ContentView : View {
    var body: some View {
        VStack {
            Text("ROG-HID Driver Manager").font(.headline)
            Spacer()
            HStack {
                Button(action: ExtensionManager.shared.activate) {
                    Text("Install")
                }
                Button(action: ExtensionManager.shared.deactivate) {
                    Text("Uninstall")
                }
            }
        }
        .frame(width: 400, height: 150)
        .padding(20)
    }
}


#if DEBUG
struct ContentView_Previews : PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
#endif
