/*
 * 2B - Ultrasound Sensor temperature calibration
 * Correct ultrasound distance calculation for temperature of the air
 * Create by Adam Fowler on 28/Sep/2020
 * Original sketch for Improved Ultrasonic Range Sensing Created by Calvin Kielas-Jensen
 *   https://www.instructables.com/id/Improve-Ultrasonic-Range-Sensor-Accuracy/
 * Copyright © 2020 VMware, Inc.
 * SPDX-License-Identifier: MIT
*/

#include <NewPing.h>

// Temperature
const int tempPin = A3;

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

  // Set up temp pin
  pinMode(tempPin, INPUT);
}

void loop() {
  distance = getDistance();   //variable to store the distance measured by the sensor

  //Serial.print(distance);     //print the distance that was measured
  //Serial.println(" cm");      //print units after the distance

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

float getVoltage(int pin) { 
  return (analogRead(pin) * .004882814); 
  //Converting from 0 to 1024 to 0 to 5v 
}

float millisecondsToCentimeters(float milliseconds, float temp) { 
  return (milliseconds * (331.3 + 0.606 * temp)) / 2000.0f; // half the flight time, divided by 1000 for milliseconds
  //Multiplying the speed of sound through a certain temperature of air by the 
  //length of time it takes to reach the object and back, divided by two
}

float getDistance() {
  unsigned int echoTime;                   //variable to store the time it takes for a ping to bounce off an object
  float calculatedDistance;         //variable to store the distance calculated from the echo time

  float temperature = (getVoltage(tempPin) - 0.5) * 100;
  echoTime = sonar.ping_median(20); // uS
  //Serial.print("Echo time: ");
  //Serial.println(echoTime);
  
  float oldCalculatedDistance = (1.0d * echoTime) / (1.0d * US_ROUNDTRIP_CM);
  calculatedDistance = millisecondsToCentimeters(echoTime, temperature);
  Serial.print("Uncorrected distance calculation: ");
  Serial.print(oldCalculatedDistance);
  Serial.print(" => ");
  Serial.print(calculatedDistance);
  Serial.print(" @ ");
  Serial.print(temperature);
  Serial.println(" deg C");
  return calculatedDistance;
}
