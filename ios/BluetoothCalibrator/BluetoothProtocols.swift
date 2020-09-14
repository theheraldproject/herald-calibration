//
//  BluetoothProtocols.swift
//  BluetoothCalibrator
//
//  Created by Adam Fowler on 06/Sep/2020.
//  Copyright © 2020 VMware, Inc.
//  SPDX-License-Identifier: MIT
//

import Foundation
import CoreBluetooth
import UIKit

extension UIDevice {
    
    // Cribbed from https://stackoverflow.com/questions/11197509/how-to-get-device-make-and-model-on-ios
    
    var modelName: String {
        var systemInfo = utsname()
        uname(&systemInfo)
        let machineMirror = Mirror(reflecting: systemInfo.machine)
        let identifier = machineMirror.children.reduce("") { identifier, element in
            guard let value = element.value as? Int8, value != 0 else { return identifier }
            return identifier + String(UnicodeScalar(UInt8(value)))
        }
        return identifier
    }

}

// There's a TONNE of information that may be useful:-
//   https://developer.apple.com/documentation/uikit/uidevice

// Stringly typed. Naughty Adam. Needs must when the devil drives.
struct CalibrationMetadata {
    var deviceName: String = ""
    // Identifier for this time running the app (helps with mac rotation)
    var persistentIdentifier: String = ""
    var model: String = ""
    var operatingSystemName: String = ""
    var operatingSystemVersion: String = ""
    // The power settings of the OTHER device
    var transmitPower: String = ""
    var receivePower: String = ""
    // The power settings of THIS device, as observed by the OTHER device
    var yourTransmitPower: String = ""
    
    var outputString: String {
        get {
            return "pid%\(persistentIdentifier)::deviceName%\(deviceName)::model%\(model)::osname%\(operatingSystemName)::osver%\(operatingSystemVersion)::txpower%\(transmitPower)::rxpower%\(receivePower)::yourtxpower%\(yourTransmitPower)"
        }
    }
    
    var csvQuotedString: String {
        get {
            // Note: No need for PID or deviceName
            return "\"\(model)\",\"\(operatingSystemName)\",\"\(operatingSystemVersion)\",\"\(transmitPower)\",\"\(receivePower)\",\"\(yourTransmitPower)\""
        }
    }
    
    static var csvHeaders: String {
        get {
            // Note: No need for PID or deviceName
            return "model,osname,osver,txpower,rxpower,yourtxpower"
        }
    }
    
    static func fromString(data:String) -> CalibrationMetadata {
        let parts = data.components(separatedBy: "::")
        var m = CalibrationMetadata()
        for part in parts {
            let kvpair = part.components(separatedBy:"%")
            if (kvpair.count > 1) {
                //sanity check
                let key = kvpair[0]
                let value = kvpair[1]
                switch key {
                case "pid":
                    m.persistentIdentifier = value
                    break
                case "deviceName":
                    m.deviceName = value
                    break
                case "model":
                    m.model = value
                    break
                case "osname":
                    m.operatingSystemName = value
                    break
                case "osver":
                    m.operatingSystemVersion = value
                    break
                case "txpower":
                    m.transmitPower = value
                    break
                case "rxpower":
                    m.receivePower = value
                    break
                case "yourtxpower":
                    m.yourTransmitPower = value
                    break
                default:
                    // do nothing
                    break
                }
            }
        }
        return m
    }
}

protocol RSSIDelegate {
    func discovered(peripheralID: UUID, name: String?) -> Void
    func readMetadata(peripheralID: UUID, metadata: CalibrationMetadata) -> Void
    func readRSSI(peripheralID: UUID, rssi: Double) -> Void
}
//
//  CoreBluetoothExtensions.swift
//  BLeDebugProject
//
//  Created by Adam Fowler on 06/09/2020.
//  Copyright © 2020 Adam Fowler. All rights reserved.
//

import Foundation
import CoreBluetooth

extension CBPeripheral {
    
    public var identifierWithName: String {
        return "\(identifier) (\(name ?? "unknown"))"
    }
    
}

extension Sequence where Iterator.Element == CBService {
    
    func calibrationService() -> CBService? {
        return first(where: {$0.uuid == BTLEQuickConnectListener.calibrationServiceUUID})
    }
    
}

extension Sequence where Iterator.Element == CBCharacteristic {
    
    func calibrationCharacteristic() -> CBCharacteristic? {
        return first(where: {$0.uuid == BTLEQuickConnectListener.calibrationCharUUID})
    }

}

