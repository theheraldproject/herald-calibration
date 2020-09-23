/*
 * Copyright 2020 VMware, Inc.
 * SPDX-License-Identifier: MIT
 * Author: Adam Fowler adamf@vmware.com adam@adamfowler.org
 */
package com.vmware.herald.calibration;

import android.os.Bundle;
import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.google.android.material.snackbar.Snackbar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import android.view.View;

import java.util.Optional;
import java.util.UUID;

public class CalibrationActivity extends AppCompatActivity implements RSSIDelegate {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_calibration);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        FloatingActionButton fab = findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            }
        });
    }

    public void goBack(View view) {
        finish();
    }

    public void saveToFile() {

    }

    @Override
    public void discovered(UUID peripheralID, Optional<String> name) {
        //if (name.isPresent()) {

        //}
    }

    @Override
    public void readMetadata(UUID peripheralID, CalibrationMetadata metadata) {

    }

    @Override
    public void readRSSI(UUID peripheralID, Double rssi) {

    }
}
