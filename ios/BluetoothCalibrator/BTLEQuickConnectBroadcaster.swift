//
//  BTLEQuickConnectBroadcaster.swift
//  BluetoothCalibrator
//
//  Created by Adam Fowler on 06/Sep/2020.
//  Copyright © 2020 VMware, Inc.
//  SPDX-License-Identifier: MIT
//

import Foundation
import CoreBluetooth
import UIKit
//import Logging


class BTLEQuickConnectBroadcaster: NSObject, CBPeripheralManagerDelegate {

    //var logger = Logger(label: "BTLE")
    
    static var identityCharacteristic = CBMutableCharacteristic(type:  BTLEQuickConnectListener.calibrationCharUUID,
                                                         properties: CBCharacteristicProperties([.read]),
                                                         value: nil, // was BTLEBroadcasterQuickConnect.broadcastID.data
                                                         permissions: .readable)
    
    
    func doNotifyCheck() {
        // DUMMY
    }
    
    var peripheral: CBPeripheralManager?
    let metadata: CalibrationMetadata
    var listener: BTLEQuickConnectListener?
    
    init(meta: CalibrationMetadata) {
        // copy fields over
        metadata = meta
    }
    
    func setListener(list: BTLEQuickConnectListener) {
        listener = list
    }

    func tryStartAdvertising() {
        //logger.logLevel = Logger.Level.debug
        
        guard let peripheral = peripheral else { return }

        startAdvertising(peripheral: peripheral)
    }

    // MARK: CBPeripheralManagerDelegate

    func peripheralManagerDidUpdateState(_ peripheral: CBPeripheralManager) {
        //logger.info("state: \(peripheral.state)")
        
        //stateDelegate?.btleBroadcaster(self, didUpdateState: peripheral.state)

        switch (peripheral.state) {
            
        case .poweredOn:
            self.peripheral = peripheral
            tryStartAdvertising()
            
        default:
            break
        }
    }
    
    private func startAdvertising(peripheral: CBPeripheralManager) {
        guard peripheral.isAdvertising == false else {
            //logger.error("peripheral manager already advertising, won't start again")
            return
        }

        let service = CBMutableService(type:  BTLEQuickConnectListener.calibrationServiceUUID, primary: true)


        service.characteristics = [BTLEQuickConnectBroadcaster.identityCharacteristic]
        peripheral.add(service)
    }
    
    func peripheralManager(_ peripheral: CBPeripheralManager, didAdd service: CBService, error: Error?) {
        guard error == nil else {
            //logger.info("error: \(error!))")
            return
        }

        //logger.info("now advertising")
        
        peripheral.startAdvertising([
            //CBAdvertisementDataLocalNameKey: "BluetoothCalibrator",
            CBAdvertisementDataServiceUUIDsKey: [service.uuid]
        ])
        //logger.info("Finished start advertising call")
    }
    
    func peripheralManager(_ peripheral: CBPeripheralManager, willRestoreState dict: [String : Any]) {
        guard let services = dict[CBPeripheralManagerRestoredStateServicesKey] as? [CBMutableService],
              let idService = services.first else {
            //logger.info("No services to restore!")
            return
        }

        //logger.info("restoring \(idService)")
    }
    
    func peripheralManager(_ peripheral: CBPeripheralManager, didReceiveRead request: CBATTRequest) {
        // Get our metadata
        var cb: CalibrationMetadata = CalibrationMetadata()
        cb.persistentIdentifier = metadata.persistentIdentifier
            // TODO get txPower from listener for this device
        if let listWithValue = listener {
            cb.yourTransmitPower = listWithValue.getAdvertisedTxPower(identifier: request.central.identifier)
        }
        cb.model = metadata.model
        cb.operatingSystemName = metadata.operatingSystemName
        cb.operatingSystemVersion = metadata.operatingSystemVersion
        // See if we have metadata for the other end, and if so add that
        // send the data back encoded as UTF-8 bytes
        // cannot discover txPower or rxPower of myself in iOS :(
        request.value = cb.outputString.data(using: .utf8)
        peripheral.respond(to: request, withResult: .success)
    }
}

