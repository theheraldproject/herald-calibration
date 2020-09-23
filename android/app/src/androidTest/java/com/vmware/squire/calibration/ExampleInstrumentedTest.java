/*
 * Copyright 2020 VMware, Inc.
 * SPDX-License-Identifier: MIT
 * Author: Adam Fowler adamf@vmware.com adam@adamfowler.org
 */
package com.vmware.herald.calibration;

import android.content.Context;
import androidx.test.platform.app.InstrumentationRegistry;
import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.*;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class ExampleInstrumentedTest {
    @Test
    public void useAppContext() {
        // Context of the app under test.
        Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();

        assertEquals("org.adamfowler.ble.calibration", appContext.getPackageName());
    }
}
