/*
 * 1D - Validate the Infrared distance sensor works as expected
 * Calibrate distance readings with the Sharp infrared sensor
 * Create by Adam Fowler on 28/Sep/2020
 * Copyright © 2020 VMware, Inc.
 * SPDX-License-Identifier: MIT
*/
#include <SharpIR.h>

const int irPin = A4;  
const int irEnable = 2;
  
// GP2Y0A41SK0F = 30cm
// GP2Y0A21YK0F = 80cm
// GP2Y0A02YK0F = 150cm                                
SharpIR irDistance(SharpIR::GP2Y0A21YK0F, irPin);

void setup() {
  Serial.begin(9600);
  //pinMode(irPin, INPUT);
  pinMode(irEnable, OUTPUT);
  digitalWrite(irEnable, HIGH);
}

void loop() {
  /*
  float analog = analogRead(irPin);
  float volts = analog*0.0048828125;
  
  //float distance = pow(sz, -1);
  
  float sv = 32*(volts - 0.4);
  float sz = 150 - sv;
  //float distance = 32*pow(volts, -1);   

  float distance = (2/0.035)*volts;
  Serial.print("Analog value: ");
  Serial.print(analog);
  Serial.print(", volts: ");
  Serial.print(volts);
  Serial.print(", scaled volts: ");
  Serial.print(sv);
  Serial.print(", size: ");
  Serial.print(sz);
  Serial.print(", distance: ");
  */
  float distance = irDistance.getDistance();
  Serial.print(distance);     
  Serial.println(" cm");
  delay(200);                                     
}
