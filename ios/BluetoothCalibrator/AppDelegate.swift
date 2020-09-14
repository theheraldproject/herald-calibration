//
//  AppDelegate.swift
//  BluetoothCalibrator
//
//  Created by Adam Fowler on 06/Sep/2020.
//  Copyright © 2020 VMware, Inc.
//  SPDX-License-Identifier: MIT
//

import UIKit
import CoreBluetooth

/**
 * Calibration UI (MVP)
 * - DONE Get basic RSSI mean, stddev working with UI
 * - See if there's any statistical value at all
 *   - DONE Add mode, median, skewness calculations to UI (Helps to see if Normal or Skewed)
 *   - DONE Get data for 10cm, 20cm 50cm, 1m, 2m, 5m, 7m 10m
 *   - DONE Save for external analysis -> CSV (calibrationDistance, rssi) -> receivedOrder
 *   - DONE Plot in R Studio and see skewness
 *   - Plot opposite side of contact and see if directly comparable both directions (otherwise we'll have to collect pairwise data all the time)
 *   - Do the same for less frequent data - see if it fits in the same distribution
 *   - Do the same for running average data - see if fits in the same distribution
 * - Useful calibration
 * - Add Remote device name, type, OS version sharing across characteristic value, and tx/rxpower if available
 * - Show these values in the calibration screen
 * - Implement save/export as JSON functionality - summary, rssiValues
 * - Implement Android version too
 *
 * Test UI (MVP)
 * - Show naive (fixed RSSI) result
 * - Show iPhone formulae (best fit RSSI based on some readings) result
 * - Show A.N.Other result on UI
 * - Implement Android version too
 */

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {
var window: UIWindow?
    public var listener : BTLEQuickConnectListener = BTLEQuickConnectListener()

    public var broadcaster: BTLEQuickConnectBroadcaster?
    
    public let myID: UUID = UUID()
    
    public var metadata: CalibrationMetadata = CalibrationMetadata()
    
    let broadcasterQueue: DispatchQueue? = DispatchQueue(label: "BTLE Broadcaster Queue")
    var peripheral: CBPeripheralManager?

    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        // Override point for customization after application launch.
        
        // configure device info
        metadata.model = UIDevice.current.modelName
        metadata.operatingSystemName = UIDevice.current.systemName
        metadata.operatingSystemVersion = UIDevice.current.systemVersion
        metadata.persistentIdentifier = myID.uuidString
        broadcaster = BTLEQuickConnectBroadcaster(meta: metadata)
        
        peripheral = CBPeripheralManager(delegate: broadcaster!, queue: broadcasterQueue, options: [ :
                   //CBPeripheralManagerOptionRestoreIdentifierKey: BluetoothNursery.peripheralRestoreIdentifier
                   /*
                   CBPeripheralManagerOptionRestoreIdentifierKey: BTLEBroadcasterReadWrite.restoreIdentifier,
                   CBPeripheralManagerRestoredStateServicesKey: BTLEBroadcasterReadWrite.restoreServicesKey,
                   CBPeripheralManagerRestoredStateAdvertisementDataKey: BTLEBroadcasterReadWrite.restoreAdvertisementKey
        */
                   ]
               )
               
        broadcaster!.tryStartAdvertising()
        listener.start()
        broadcaster?.setListener(list: listener) // to allow retransmission of txpower
        
        return true
    }

    // MARK: UISceneSession Lifecycle

    @available(iOS 13.0, *)
    func application(_ application: UIApplication, configurationForConnecting connectingSceneSession: UISceneSession, options: UIScene.ConnectionOptions) -> UISceneConfiguration {
        // Called when a new scene session is being created.
        // Use this method to select a configuration to create the new scene with.
        return UISceneConfiguration(name: "Default Configuration", sessionRole: connectingSceneSession.role)
    }
    
    @available(iOS 13.0, *)
    func application(_ application: UIApplication, didDiscardSceneSessions sceneSessions: Set<UISceneSession>) {
        // Called when the user discards a scene session.
        // If any sessions were discarded while the application was not running, this will be called shortly after application:didFinishLaunchingWithOptions.
        // Use this method to release any resources that were specific to the discarded scenes, as they will not return.
    }


}

