/*
 * Copyright 2020 VMware, Inc.
 * SPDX-License-Identifier: MIT
 * Author: Adam Fowler adamf@vmware.com adam@adamfowler.org
 */
package org.adamfowler.ble.calibration;

public class CalibrationMetadata {
    public String deviceName = "";
    public String persistentIdentifier = "";
    public String model = "";
    public String operatingSystemName = "";
    public String operatingSystemVersion = "";
    public String transmitPower = "";
    public String receivePower = "";
    public String yourTransmitPower = "";

    public String outputString() {
        return "pid%" + persistentIdentifier + "::deviceName%" + deviceName + "::model%" + model +
                "::osname%" + operatingSystemName + "::osver%" + operatingSystemVersion +
                "::txpower%" + transmitPower + "::rxpower%" + receivePower + "::yourtxpower%" + yourTransmitPower;
    }

    public String csvQuotedString() {
        return "\"" + deviceName + "\",\"" + model + "\",\"" + operatingSystemName + "\",\"" +
                operatingSystemVersion + "\",\"" + transmitPower + "\",\"" + receivePower +
                "\",\"" + yourTransmitPower + "\"";
    }

    public static String csvHeaders() {
        return "deviceName,model,osname,osver,txpower,rxpower,yourtxpower";
    }

    public static CalibrationMetadata fromString(String data) {
        String[] parts = data.split("::");
        CalibrationMetadata m = new CalibrationMetadata();
        for (String part : parts) {
            String[] kvpair = part.split("%");
            if (kvpair.length > 1) {
                //sanity check
                String key = kvpair[0];
                String value = kvpair[1];
                switch (key) {
                    case "pid":
                        m.persistentIdentifier = value;
                        break;
                    case "deviceName":
                        m.deviceName = value;
                        break;
                    case "model":
                        m.model = value;
                        break;
                    case "osname":
                        m.operatingSystemName = value;
                        break;
                    case "osver":
                        m.operatingSystemVersion = value;
                        break;
                    case "txpower":
                        m.transmitPower = value;
                        break;
                    case "rxpower":
                        m.receivePower = value;
                        break;
                    case "yourtxpower":
                        m.yourTransmitPower = value;
                        break;
                    default:
                        // do nothing
                        break;
                }
            }
        }
        return m;
    }
}
