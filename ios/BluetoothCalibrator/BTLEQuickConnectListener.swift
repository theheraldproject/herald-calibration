//
//  BTLEQuickConnectListener.swift
//  BluetoothCalibrator
//
//  Created by Adam Fowler on 06/Sep/2020.
//  Copyright © 2020 VMware, Inc.
//  SPDX-License-Identifier: MIT
//

import Foundation
import CoreBluetooth
//import Logging

class BTLEQuickConnectListener: NSObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    
    static let calibrationServiceUUID = CBUUID(nsuuid: UUID(uuidString: "aa3016fe-d290-43dc-a3a8-06a1ce8784ff")!)
    static let calibrationCharUUID = CBUUID(nsuuid: UUID(uuidString: "488627e1-b977-4bce-8da2-c5e46a3336af")!)

    //var logger = Logger(label: "BTLE")
    
    var central: CBCentralManager?
    
    
    var peripherals: [UUID: CBPeripheral] = [:]
    
    var peripheralPower: [UUID: String] = [:]
    
    var rssiDelegate: RSSIDelegate?
    
    // MARK: CBCentralManagerDelegate
    

    func start() {
        //logger.logLevel = Logger.Level.debug
        
        central = CBCentralManager(
            delegate: self,
            queue: nil,
            options: [ : ])
    }
    
    func setRssiDelegate(rssiDelegate: RSSIDelegate) {
        self.rssiDelegate = rssiDelegate
    }
    
    func centralManager(_ central: CBCentralManager, willRestoreState dict: [String : Any]) {
        //logger.debug("willRestoreState")
        guard let restoredPeripherals = dict[CBCentralManagerRestoredStatePeripheralsKey] as? [CBPeripheral] else {
            //logger.info("no peripherals to restore for \(central)")
            return
        }
        
        //logger.info("restoring \(restoredPeripherals.count) peripherals for central \(central)")
        for peripheral in restoredPeripherals {
            peripherals[peripheral.identifier] = peripheral
            peripheral.delegate = self
        }
    }

    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        //logger.debug("centralManagerDidUpdateState")
        //logger.info("state: \(central.state)")
        
        //stateDelegate?.btleListener(self, didUpdateState: central.state)
        
        switch (central.state) {
                
        case .poweredOn:
            self.central = central
            
            // Ensure all "connected" peripherals are properly connected after state restoration
            /*
            for peripheral in peripherals.values {
                guard peripheral.state == .connected else {
                    logger.info("attempting connection to peripheral \(peripheral.identifierWithName) in state \(peripheral.state)")
                    central.connect(peripheral)
                    continue
                }
                guard let idService = peripheral.services?.idService() else {
                    logger.info("discovering services for peripheral \(peripheral.identifierWithName)")
                    peripheral.discoverServices([ConcreteBTLEBroadcaster.serviceUUID])
                    continue
                }
                guard let idCharacteristic = idService.characteristics?.idCharacteristic() else {
                    logger.info("discovering characteristics for peripheral \(peripheral.identifierWithName)")
                    peripheral.discoverCharacteristics([ConcreteBTLEBroadcaster.idCharacteristicUUID], for: idService)
                    continue
                }
                logger.info("reading id from fully-connected peripheral \(peripheral.identifierWithName)")
                peripheral.readValue(for: idCharacteristic)
            }
 */
            let calibrationService =  BTLEQuickConnectListener.calibrationServiceUUID
            
            central.scanForPeripherals(withServices: [calibrationService], options: [CBCentralManagerScanOptionAllowDuplicatesKey: true])

        default:
            break
        }
    }
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        //logger.debug("didDiscover")
        //logger.info("peripheral \(peripheral.identifierWithName) discovered at RSSI = \(RSSI)")
        
        peripherals[peripheral.identifier] = peripheral
        let power:String? = advertisementData[CBAdvertisementDataTxPowerLevelKey] as? String
        if (nil != power) {
            peripheralPower[peripheral.identifier] = power
        }
        rssiDelegate?.discovered(peripheralID: peripheral.identifier, name: peripheral.name)
        rssiDelegate?.readRSSI(peripheralID: peripheral.identifier, rssi: Double(truncating: RSSI))
        
        //delegate?.btleListener(self, didReadRSSI: RSSI.intValue, forPeripheral: peripheral)
        
        // Now connect and read additional metadata
        central.connect(peripheral)
    }
    
    func getAdvertisedTxPower(identifier: UUID) -> String {
        let id: String? = peripheralPower[identifier]
        if (nil == id) {
            return ""
        }
        return id!
    }
    
    
    
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        //logger.debug("didConnect")
        //logger.info("\(peripheral.identifierWithName)")

        peripheral.delegate = self
        peripheral.discoverServices([BTLEQuickConnectListener.calibrationServiceUUID])
    }

    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        //logger.debug("didDisconnect")
        if let error = error {
            //logger.info("\(peripheral.identifierWithName) error: \(error)")
        } else {
            //logger.info("\(peripheral.identifierWithName)")
        }
                
        //peripherals[peripheral.identifierWithName] = nil
        //logger.info("-------------------")
    }
    
    // MARK: CBPeripheralDelegate
    /*
    func peripheral(_ peripheral: CBPeripheral, didModifyServices invalidatedServices: [CBService]) {
        logger.info("\(peripheral.identifierWithName) invalidatedServices:")
        for service in invalidatedServices {
            logger.info("\t\(service)\n")
        }
    }
 */
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        //logger.debug("didDiscoverServices")
        guard error == nil else {
            //logger.info("error: \(error!)")
            return
        }
        
        guard let services = peripheral.services, services.count > 0 else {
            //logger.info("No services discovered for peripheral \(peripheral.identifierWithName)")
            return
        }
        
        guard let idService = services.calibrationService() else {
            //logger.info("Service not discovered for \(peripheral.identifierWithName)")
            return
        }

        //logger.info("id service found: \(idService)")
        peripheral.discoverCharacteristics([BTLEQuickConnectListener.calibrationCharUUID], for: idService)
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        //logger.debug("didDiscoverCharacteristic")
        guard error == nil else {
            //logger.info("error: \(error!)")
            return
        }
        
        guard let characteristics = service.characteristics, characteristics.count > 0 else {
            //logger.info("No characteristics discovered for service \(service)")
            return
        }
        
        guard let idCharacteristic = characteristics.calibrationCharacteristic() else {
            //logger.info("id characteristic not discovered for peripheral \(peripheral.identifierWithName)")
            return
        }

        //logger.info("id characteristic found: \(idCharacteristic)")
        peripheral.readValue(for: idCharacteristic)
    }

    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        //logger.debug("didUpdateValueFor")
        if error != nil {
            //logger.info("characteristic \(characteristic) error: \(error!)")
            //return
        }
        // TEST: Crazy idea - iOS error here is not null, even if we can read the value...
        // RESULT: NOT SO CRAZY! It DOES return a value even if an error occurred
        /*
         16:19:23+0100 debug: didDiscoverCharacteristic
         16:19:23+0100 info: id characteristic found: <CBCharacteristic: 0x283222d00, UUID = 85BF337C-5B64-48EB-A5F7-A9FED135C972, properties = 0x2, value = {length = 16, bytes = 0xc7a0221e39804883bb0dc5d8a74854c9}, notifying = NO>
         16:19:23+0100 debug: didUpdateValueFor
         16:19:23+0100 info: characteristic <CBCharacteristic: 0x283222d00, UUID = 85BF337C-5B64-48EB-A5F7-A9FED135C972, properties = 0x2, value = {length = 16, bytes = 0xc7a0221e39804883bb0dc5d8a74854c9}, notifying = NO> error: Error Domain=CBATTErrorDomain Code=242 "Unknown ATT error." UserInfo={NSLocalizedDescription=Unknown ATT error.}
         16:19:23+0100 info: Found characteristic value: C7A0221E-3980-4883-BB0D-C5D8A74854C9
         16:19:23+0100 info: successfully read id from peripheral 2062ABC1-BD22-20CD-D169-4AE81D5F6D63 (Adam’s iPhone)
         16:19:23+0100 debug: didDisconnect
         */

        guard characteristic.uuid == BTLEQuickConnectListener.calibrationCharUUID else {
            //logger.info("characteristic \(characteristic) does not have correct UUID")
            return
        }

        guard let data = characteristic.value else {
            //logger.info("no data found in characteristic \(characteristic)")
            return
        }
        
        // TODO parse the received string as required
        let parsed = String(bytes: data, encoding: .utf8)
        guard let parsedOk = parsed else {
            //logger.info("could not parse string sent in characteristic \(characteristic)")
            return
        }
        //logger.debug("Parsed remote info: \(parsedOk)")
        let meta = CalibrationMetadata.fromString(data: parsedOk)
        rssiDelegate?.readMetadata(peripheralID: peripheral.identifier, metadata: meta)
        
        /*
        guard data.count == BroadcastIdEncrypter.broadcastIdLength else {
            logger.info("characteristic value is not a valid id, because it has length \(data.count). Expected \(BroadcastIdEncrypter.broadcastIdLength)")
            return
        }
*/
        
        //logger.info("successfully read calibration data from peripheral \(peripheral.identifierWithName)")
        //delegate?.btleListener(self, didFind: data, forPeripheral: peripheral)
        central?.cancelPeripheralConnection(peripheral)
        // TEST: CRAZY idea - don't disconnect, see if not calling this method alters iOS behaviour to allow repeated reads in the BG
        // RESULT: Works for up to 12 seconds approx, then in the background it's disconnected anyway. After about 30 more seconds, an error returned in logging.
        /*
         16:22:24+0100 debug: didUpdateValueFor
         16:22:24+0100 info: characteristic <CBCharacteristic: 0x280b48ea0, UUID = 85BF337C-5B64-48EB-A5F7-A9FED135C972, properties = 0x2, value = {length = 16, bytes = 0x0e826be112614b19bd3575f84b639371}, notifying = NO> error: Error Domain=CBATTErrorDomain Code=242 "Unknown ATT error." UserInfo={NSLocalizedDescription=Unknown ATT error.}
         16:22:24+0100 info: Found characteristic value: 0E826BE1-1261-4B19-BD35-75F84B639371
         16:22:24+0100 info: successfully read id from peripheral 2062ABC1-BD22-20CD-D169-4AE81D5F6D63 (Adam’s iPhone)
         16:22:59+0100 debug: didDisconnect
         16:22:59+0100 info: 2062ABC1-BD22-20CD-D169-4AE81D5F6D63 (Adam’s work iPhone) error: Error Domain=CBErrorDomain Code=7 "The specified device has disconnected from us." UserInfo={NSLocalizedDescription=The specified device has disconnected from us.}
         16:22:59+0100 info: -------------------
         */
        
        // FOLLOW ON TEST: See if the advertisement is still being sent (and thus filtered by iOS on the receiver) OR not being sent (and thus not advertising in iOS in the background)
        // DEPENDENCY: Bluetooth Scanner
        
    }

}
