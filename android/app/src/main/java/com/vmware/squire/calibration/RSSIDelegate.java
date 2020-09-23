/*
 * Copyright 2020 VMware, Inc.
 * SPDX-License-Identifier: MIT
 * Author: Adam Fowler adamf@vmware.com adam@adamfowler.org
 */
package com.vmware.herald.calibration;

import java.util.Optional;
import java.util.UUID;

public interface RSSIDelegate {
    public void discovered(UUID peripheralID, Optional<String> name);
    public void readMetadata(UUID peripheralID, CalibrationMetadata metadata);
    public void readRSSI(UUID peripheralID, Double rssi);
}
