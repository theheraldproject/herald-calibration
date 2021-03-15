/*
 * 1B - RGB LED and Ultrasound Sensor validation
 * Validate that the multi colour LED and ultrasound sensor are working
 * Create by Adam Fowler on 27/Sep/2020
 * Original sketch was written by SparkFun Electronics, with lots of help from the Arduino community.
 * Copyright © 2020 VMware, Inc.
 * SPDX-License-Identifier: MIT
*/

#include <NewPing.h>

// NewPing with sinlge pin operation for HC-SR04
const int trigPin = 5;           //connects to the trigger pin on the distance sensor
const int echoPin = 5;           //connects to the echo pin on the distance sensor
const int maxDistance = 500; // cm

NewPing sonar(trigPin,echoPin,maxDistance);

const int redPin = A0;             //pin to control the red LED inside the RGB LED
const int greenPin = A1;           //pin to control the green LED inside the RGB LED
const int bluePin = A2;            //pin to control the blue LED inside the RGB LED

float distance = 0;               //stores the distance measured by the distance sensor

void setup()
{
  Serial.begin (9600);        //set up a serial connection with the computer
  Serial.println("======================================");

  //pinMode(trigPin, OUTPUT);   //the trigger pin will output pulses of electricity
  //pinMode(echoPin, INPUT);    //the echo pin will measure the duration of pulses coming back from the distance sensor

  //set the RGB LED pins to output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // cycle RGB LED to test
  analogWrite(redPin, 255);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
  delay(1000);
  analogWrite(redPin, 0);
  analogWrite(greenPin, 255);
  analogWrite(bluePin, 0);
  delay(1000);
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 255);
  delay(1000);
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}

void loop() {
  distance = getDistance();   //variable to store the distance measured by the sensor

  Serial.print(distance);     //print the distance that was measured
  Serial.println(" cm");      //print units after the distance

  if (distance <= 10) {                       //if the object is close

    //make the RGB LED red
    analogWrite(redPin, 255);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);

  } else if (10 < distance && distance < 20) { //if the object is a medium distance

    //make the RGB LED yellow
    analogWrite(redPin, 255);
    analogWrite(greenPin, 255);
    analogWrite(bluePin, 0);

  } else {                                    //if the object is far away

    //make the RGB LED green
    analogWrite(redPin, 0);
    analogWrite(greenPin, 255);
    analogWrite(bluePin, 0);
  }

  delay(50);      //delay 50ms between each reading
}

//------------------FUNCTIONS-------------------------------

float getDistance() {
  unsigned int echoTime;                   //variable to store the time it takes for a ping to bounce off an object
  float calculatedDistance;         //variable to store the distance calculated from the echo time
  
  echoTime = sonar.ping_median(20); // uS
  Serial.print("Echo time: ");
  Serial.println(echoTime);
  //pinMode(echoPin,OUTPUT);
  //digitalWrite(echoPin,LOW);
  //pinMode(echoPin,INPUT);
  
  calculatedDistance = sonar.convert_cm(echoTime); // rounds to nearest cm
  calculatedDistance = (1.0d * echoTime) / (1.0d * US_ROUNDTRIP_CM);
  return calculatedDistance;
}

//RETURNS THE DISTANCE MEASURED BY THE HC-SR04 DISTANCE SENSOR
float getDistanceOld()
{
  float echoTime;                   //variable to store the time it takes for a ping to bounce off an object
  float calculatedDistance;         //variable to store the distance calculated from the echo time

  //send out an ultrasonic pulse that's 10ms long
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(50);
  digitalWrite(trigPin, LOW);

  echoTime = pulseIn(echoPin, HIGH);      //use the pulsein command to see how long it takes for the
                                          //pulse to bounce back to the sensor
  Serial.print("Echo time: ");
  Serial.println(echoTime);

  // 2.56 is so output is CM
  calculatedDistance = echoTime / (148.0 * 2.56);  //calculate the distance of the object that reflected the pulse (half the bounce time multiplied by the speed of sound)

  return calculatedDistance;              //send back the distance that was calculated
}
