/*
 * 1C - LIDAR Lite v3 validation
 * Validate that your LIDAR unit is working as expected and giving accurate results
 * Create by Adam Fowler on 27/Sep/2020
 * Original sketch was written by SparkFun Electronics, with lots of help from the Arduino community.
 * Copyright © 2020 VMware, Inc.
 * SPDX-License-Identifier: MIT
 */

#include <Wire.h>
#include <LIDARLite.h>

LIDARLite myLidarLite;

void setup()
{
  Serial.begin(9600); // Initialize serial connection to display distance readings

  /*
    begin(int configuration, bool fasti2c, char lidarliteAddress)

    Starts the sensor and I2C.

    Parameters
    ----------------------------------------------------------------------------
    configuration: Default 0. Selects one of several preset configurations.
    fasti2c: Default 100 kHz. I2C base frequency.
      If true I2C frequency is set to 400kHz.
    lidarliteAddress: Default 0x62. Fill in new address here if changed. See
      operating manual for instructions.
  */
  myLidarLite.begin(0, true); // Set configuration to default and I2C to 400 kHz

  /*
    configure(int configuration, char lidarliteAddress)

    Selects one of several preset configurations.

    Parameters
    ----------------------------------------------------------------------------
    configuration:  Default 0.
      0: Default mode, balanced performance.
      1: Short range, high speed. Uses 0x1d maximum acquisition count.
      2: Default range, higher speed short range. Turns on quick termination
          detection for faster measurements at short range (with decreased
          accuracy)
      3: Maximum range. Uses 0xff maximum acquisition count.
      4: High sensitivity detection. Overrides default valid measurement detection
          algorithm, and uses a threshold value for high sensitivity and noise.
      5: Low sensitivity detection. Overrides default valid measurement detection
          algorithm, and uses a threshold value for low sensitivity and noise.
    lidarliteAddress: Default 0x62. Fill in new address here if changed. See
      operating manual for instructions.
  */
  myLidarLite.configure(4); // Change this number to try out alternate configurations
}

void loop()
{
  /*
    distance(bool biasCorrection, char lidarliteAddress)

    Take a distance measurement and read the result.

    Parameters
    ----------------------------------------------------------------------------
    biasCorrection: Default true. Take aquisition with receiver bias
      correction. If set to false measurements will be faster. Receiver bias
      correction must be performed periodically. (e.g. 1 out of every 100
      readings).
    lidarliteAddress: Default 0x62. Fill in new address here if changed. See
      operating manual for instructions.
  */

  // Take a measurement with receiver bias correction and print to serial terminal
  //Serial.print("With bias correction:    ");
  Serial.println(distance());

  //Serial.print("Without bias correction: ");
  //Serial.println(distanceWithout());

  //delay(1000);
  //delay(100);
}


void swap(int *p,int *q) {
   int t;
   
   t=*p; 
   *p=*q; 
   *q=t;
}

void sort(int a[],int n) { 
   int i,j,temp;

   for(i = 0;i < n-1;i++) {
      for(j = 0;j < n-i-1;j++) {
         if(a[j] > a[j+1])
            swap(&a[j],&a[j+1]);
      }
   }
}

float sd(int a[],int n,float mean) {
  float sum = 0;
  for (int c = 0;c < n;c++) {
    sum += (a[c] - mean)*(a[c] - mean);
  }
  return sqrt(sum / n);
}

float distance() {
  float sum = 0;
  int count = 450; // at 500 SD skyrockets, at 400 SD is 20% higher, at 100 SD is 50% higher
  int data[count];
  data[0] += myLidarLite.distance();
  sum += data[0];
  for (int c = 1;c < count;c++) {
    data[c] = myLidarLite.distance(false);
    sum += data[c];
    //delay(20); // NO NEED FOR THIS - HAS ZERO AFFECT ON ACCURACY
    if (0 == (c % 100)) { // at 50 or 150 SD increases about 10%
      c++;
      data[c] = myLidarLite.distance(); // recalibrate every 100
    }
  }
  sort(data,count);
  int mpos = ((count + 1) / 2) - 1;
  Serial.print("count: ");
  Serial.print(count);
  Serial.print(", sum: ");
  Serial.print(sum);
  Serial.print(", mpos: ");
  Serial.print(mpos);
  Serial.print(", median: ");
  Serial.print(data[mpos]);
  Serial.print(", mean: ");
  Serial.print(sum/count);
  Serial.print(", sd: ");
  Serial.println(sd(data,count,sum/count));
  return data[mpos];
}
