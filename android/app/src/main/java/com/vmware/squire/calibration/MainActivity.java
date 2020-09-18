/*
 * Copyright 2020 VMware, Inc.
 * SPDX-License-Identifier: MIT
 * Author: Adam Fowler adamf@vmware.com adam@adamfowler.org
 */
package com.vmware.squire.calibration;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.content.Intent;
import android.view.View;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void openCalibration(View view) {
        Intent intent = new Intent(this,CalibrationActivity.class);
        startActivity(intent);
    }
}
