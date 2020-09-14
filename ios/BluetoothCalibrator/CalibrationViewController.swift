//
//  CalibrationViewController.swift
//  BluetoothCalibrator
//
//  Created by Adam Fowler on 06/Sep/2020.
//  Copyright © 2020 VMware, Inc.
//  SPDX-License-Identifier: MIT
//

import UIKit
//import Logging

class CalibrationViewController: UIViewController, RSSIDelegate, UIPickerViewDelegate, UIPickerViewDataSource {
    
    //var logger = Logger(label: "Calibration")
    
    @IBOutlet weak var distance: UISegmentedControl!
    @IBOutlet weak var remote: UIPickerView!
    @IBOutlet weak var butCalibration: UIButton!
    
    private var nameToUUID: [String : UUID] = [ : ]
    
    private var isCalibrating: Bool = false
    private var rssiValues: [Double] = [ ] // to save after
    // running average
    private var rssiRunningAverageValues: [Double] = [ ]
    private var rssiMostRecentValues: [Double] = [ ]
    private let rssiCountForAverage = 5
    // overview stats
    private var rssiRunningTotal: Double = 0.0
    private var rssiCount: Int = 0
    
    private var remoteMetadata: CalibrationMetadata?
    private var remoteName: String?
    private var remoteUUID: UUID?
    private var dist: Float = 0.8
    
    @IBOutlet weak var lblReadings: UILabel!
    @IBOutlet weak var lblMean: UILabel!
    @IBOutlet weak var lblVariance: UILabel!
    @IBOutlet weak var lblStdDev: UILabel!
    @IBOutlet weak var lblMedian: UILabel!
    @IBOutlet weak var lblMode: UILabel!
    @IBOutlet weak var lblFPSkewness: UILabel!
    @IBOutlet weak var lblModel: UILabel!
    @IBOutlet weak var lblOSName: UILabel!
    @IBOutlet weak var lblOSVersion: UILabel!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view.
        //logger.logLevel = .debug
        remote.delegate = self
        remote.dataSource = self
        // now link in to the bluetooth system
        let appDel = UIApplication.shared.delegate as! AppDelegate
        appDel.listener.setRssiDelegate(rssiDelegate: self)
    }
    
    @IBAction func saveToFile(_ sender: UIButton) {
        // create filename
        let formatter = DateFormatter()
        var calendar = Calendar(identifier: .iso8601)
        calendar.timeZone = TimeZone(identifier: "UTC")!
        formatter.calendar = calendar
        //formatter.locale = Locale(identifier: "en_GB_POSIX")
        formatter.timeZone = TimeZone(secondsFromGMT: 0)
        formatter.dateFormat = "yyyy-MM-dd'T'HHmmss'Z'"
        let dateString = formatter.string(from: Date())
        var from = "unknown"
        var to = "unknown"

        let myModel = UIDevice.current.modelName
        let myOSName = UIDevice.current.systemName
        let myOSVer = UIDevice.current.systemVersion
        let myHeader = ",recmodel,recosname,recosver"
        let myRow = ",\"\(myModel)\",\"\(myOSName)\",\"\(myOSVer)\"" // Fix for badly formatted CSV 06 Sep 2020
        
        if (nil != remoteMetadata) {
            from = remoteMetadata!.model.replacingOccurrences(of: ".", with: "_") + "-" +
                remoteMetadata!.operatingSystemName.replacingOccurrences(of: ".", with: "_") + "-" + remoteMetadata!.operatingSystemVersion.replacingOccurrences(of: ".", with: "_")
        }
        to = myModel.replacingOccurrences(of: ".", with: "_") + "-" +
            myOSName.replacingOccurrences(of: ".", with: "_") + "-" + myOSVer.replacingOccurrences(of: ".", with: "_")
        
        let filename = "from-\(from)-to-\(to)-at-\(dist)m-readings-\(dateString).csv"
        // get folder reference
        let dir = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first!
        let fileUrl = dir.appendingPathComponent(filename)
        // output as csv
        var csv = "distance,rssi,\(CalibrationMetadata.csvHeaders)\(myHeader)\n"
        var csvData: String = ""
        if (nil != remoteMetadata) {
            csvData = remoteMetadata!.csvQuotedString
        }
        for rssi in rssiValues {
            csv = csv + "\(dist),\(rssi),\(csvData)\(myRow)\n"
        }
        do {
            try csv.write(to: fileUrl, atomically: false, encoding: .utf8)
        } catch {
            print("Error \(error)")
        }
        
        let dialogMessage = UIAlertController(title: "Data Saved", message: "Your data has been saved", preferredStyle: .alert)
        
        // Create OK button with action handler
        let ok = UIAlertAction(title: "OK", style: .default, handler: { (action) -> Void in
             print("Ok button tapped")
        })
        
        //Add OK and Cancel button to dialog message
        dialogMessage.addAction(ok)
        
        // Present dialog message to user
        self.present(dialogMessage, animated: true, completion: nil)
        
        
    }
    
    @IBAction func calibration(_ sender: Any) {
        if isCalibrating {
            // stop calibrating
            butCalibration.setTitle("Start Calibration", for: .normal)
        } else {
            rssiValues = [ ]
            rssiRunningTotal = 0.0
            rssiCount = 0
            lblModel.text = "-"
            lblOSName.text = "-"
            lblOSVersion.text = "-"
            remoteMetadata = nil
            remoteUUID = nil
            dist = Float(distance.titleForSegment(at: distance.selectedSegmentIndex)!)!
            //logger.debug("Distance for calibration: \(dist)")
            if nil != remoteName {
                remoteUUID = nameToUUID[remoteName!]!
            }
            
            butCalibration.setTitle("Stop Calibration", for: .normal)
        }
        isCalibrating = !isCalibrating
    }
    
    func discovered(peripheralID: UUID, name: String?) -> Void {
        //logger.debug("discovered called: UUID: \(peripheralID) name:\(String(describing: name))")
        // iPhone 4S seems to never discover deviceName from remotes
        if nil != name {
            nameToUUID[name!] = peripheralID
            remote.reloadAllComponents()
        } else {
            nameToUUID[peripheralID.uuidString] = peripheralID
            remote.reloadAllComponents()
        }
    }
    
    func readMetadata(peripheralID: UUID, metadata: CalibrationMetadata) {
        // do something value useful with this...
        // first time, if right mac, cache it
        if (nil == remoteMetadata && peripheralID==self.remoteUUID) {
            remoteMetadata = metadata
            return
        }
        // subsequent times, if correct persistent ID, replace it and the ID
        // (this takes care of mac address rotation)
        if (remoteMetadata?.persistentIdentifier != "" && remoteMetadata?.persistentIdentifier == metadata.persistentIdentifier) {
            remoteMetadata = metadata
            remoteUUID = peripheralID // fixes mac rotation
            
            lblModel.text = remoteMetadata?.model
            lblOSName.text = remoteMetadata?.operatingSystemName
            lblOSVersion.text = remoteMetadata?.operatingSystemVersion
        }
    }
    
    func readRSSI(peripheralID: UUID, rssi: Double) -> Void {
        if (!isCalibrating) {
            return
        }
        //logger.debug("readRSSI called")
        // add to current totals
        if peripheralID == remoteUUID {
            //logger.debug("Received from correct peripheral")
            rssiValues.append(rssi)
            rssiRunningTotal = rssiRunningTotal + rssi
            rssiCount = rssiCount + 1
            lblReadings.text = String(rssiCount)
            
            // now add to running averages
            rssiMostRecentValues.append(rssi)
            if (rssiMostRecentValues.count > rssiCountForAverage) {
                rssiMostRecentValues.remove(at: 0)
            }
            var rssiAverageRunningTotal = 0.0
            for value in rssiMostRecentValues {
                rssiAverageRunningTotal = rssiAverageRunningTotal + value
            }
            rssiRunningAverageValues.append(rssiAverageRunningTotal / Double(rssiMostRecentValues.count))
            
            // calculate std dev etc every 100 readings
            if 0 == rssiCount % 100 {
                let mean: Double = rssiRunningTotal / Double(rssiCount)
                lblMean.text = String(mean)
                var variance: Double = 0.0
                for reading in rssiValues {
                    variance = variance + pow(reading - mean,2)
                }
                variance = variance / Double(rssiCount)
                lblVariance.text = String(variance)
                lblStdDev.text = String(pow(variance,0.5))
                
                // TODO mode, median, and skewness
                var valueCounts: [Double: Int] = [ : ]
                let N = Double(rssiValues.count)
                let S3 = pow(pow(variance,0.5),3)
                let fisherPearsonCoefficient = pow(N*(N-1),0.5) / N-2
                var fisherPearsonSkewness = 0.0
                for rssiVal in rssiValues {
                    // skewness
                    fisherPearsonSkewness = fisherPearsonSkewness +
                        (pow(rssiVal - mean,3) / N)
                    
                    // counts
                    let vc = valueCounts[rssiVal]
                    if nil == vc {
                        valueCounts[rssiVal] = 1
                    } else {
                        valueCounts[rssiVal] = vc! + 1
                    }
                }
                fisherPearsonSkewness = fisherPearsonCoefficient *
                    (fisherPearsonSkewness / S3)
                lblFPSkewness.text = String(fisherPearsonSkewness)
                
                // Now sort by count
                let ascending = valueCounts.sorted(by: { $0.value < $1.value })
                // idiot check
                //let first = ascending.index(ascending.startIndex, offsetBy: 0)
                let firstKey = ascending[0].key
                let firstValue = ascending[0].value
                let lastKey = ascending[ascending.count - 1].key
                let lastValue = ascending[ascending.count - 1].value
                //let midKey = ascending[ascending.count / 2].key
                //let midValue = ascending[ascending.count / 2].value
                let halfPopulation = rssiValues.count / 2
                var runningPopulation = 0
                var median: Double = -1.0
                for ordered in ascending {
                    runningPopulation = runningPopulation + ordered.value
                    if (runningPopulation >= halfPopulation && -1.0 == median) {
                        median = ordered.key
                    }
                }
                //logger.debug("Median value is \(median) ")
                //logger.debug("First (Lowest?) value is \(firstKey) with freq \(firstValue)")
                //logger.debug("Last (highest?) value is \(lastKey) with freq \(lastValue)")
                // Highest count is the mode
                lblMode.text = String(lastKey)
                // mid count is the median
                lblMedian.text = String(median)
            }
        }
    }
    
    // MARK: UIPickerViewDataSource
    func numberOfComponents(in pickerView: UIPickerView) -> Int {
        return 1
    }
    
    func pickerView(_ pickerView: UIPickerView, numberOfRowsInComponent component: Int) -> Int {
        return nameToUUID.count
    }
    
    func pickerView(_ pickerView: UIPickerView, titleForRow row: Int, forComponent component: Int) -> String? {
        //logger.debug("titleForRow row: \(row) component: \(component)")
        let idx = self.nameToUUID.index(nameToUUID.startIndex, offsetBy: row)
        if nil == remoteName {
            remoteName = nameToUUID.keys[idx]
        }
        return remoteName
    }
    
    func pickerView(_ pickerView: UIPickerView, didSelectRow row: Int, inComponent component: Int) {
        //logger.debug("UPDATED REMOTE SELECTION row: \(row) component: \(component) startIndex: \(nameToUUID.startIndex)")
        let idx = self.nameToUUID.index(nameToUUID.startIndex, offsetBy: row)
        //logger.debug("  - REMOTE SELECTION index: \(idx)")
        remoteName = nameToUUID.keys[idx]
        //logger.debug("  - REMOTE SELECTION remoteName: \(remoteName!)")
    }
    
}
