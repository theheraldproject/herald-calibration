/*
 * 2A - Wheel speed calibration using ultrasound sensor
 * Validate that your calibration robot's motors are working as expected
 * Create by Adam Fowler on 27/Sep/2020
 * Original sketch was written by SparkFun Electronics, with lots of help from the Arduino community.
 * Copyright © 2020 VMware, Inc.
 * SPDX-License-Identifier: MIT
*/

#include <NewPing.h>

// ULTRASOUND

// NewPing with sinlge pin operation for HC-SR04
const int trigPin = 5;           //connects to the trigger pin on the distance sensor
const int echoPin = 5;           //connects to the echo pin on the distance sensor
const int maxDistance = 100; // cm // TODO does this affect precision if we lower it???

NewPing sonar(trigPin,echoPin,maxDistance);

const int redPin = A0;             //pin to control the red LED inside the RGB LED
const int greenPin = A1;           //pin to control the green LED inside the RGB LED
const int bluePin = A2;            //pin to control the blue LED inside the RGB LED

float distance = 0;               //stores the distance measured by the distance sensor

// MOTOR

//the motor will be controlled by the motor A pins on the motor driver
const int AIN1 = 13;           //control pin 1 on the motor driver for the right motor
const int AIN2 = 12;            //control pin 2 on the motor driver for the right motor
const int PWMA = 11;            //speed control pin on the motor driver for the right motor

const int BIN1 = 8;
const int BIN2 = 9;
const int PWMB = 10;

int switchPin = 7;             //switch to turn the robot on and off

int leftMotorSpeed = 0;       //starting speed for the motor
int rightMotorSpeed = 0;       //starting speed for the motor
bool leftMotorNext = true;
bool hasPrinted = false;

bool speedIsCalibrated = false;

int calibrationSpeed = 200;
float metresPerSecond = 0.1f;

void setup()
{
  // SERIAL
  Serial.begin (9600);        //set up a serial connection with the computer
  Serial.println("======================================");

  // ULTRASOUND
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
  delay(500);
  analogWrite(redPin, 0);
  analogWrite(greenPin, 255);
  analogWrite(bluePin, 0);
  delay(500);
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 255);
  delay(500);
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);

  // MOTOR
  
  pinMode(switchPin, INPUT_PULLUP);   //set this as a pullup to sense whether the switch is flipped

  //set the motor control pins as outputs
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);

  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

}

void loop() {
  if (!speedIsCalibrated) {
    calibrateSpeed();
  }
  distance = getDistance();   //variable to store the distance measured by the sensor

  Serial.print("Distance in main loop: ");
  Serial.print(distance);     //print the distance that was measured
  Serial.println(" cm");      //print units after the distance

  if (0 == distance) {
    red();
  }

  delay(50);      //delay 50ms between each reading
}

//------------------MID LEVEL FUNCTIONS-------------------------------

void calibrateSpeed() {
  delay(2000);
  Serial.println("Moving to 10cm (Naive)...");
  // Move to 10cm
  float distance = getDistance();
  Serial.print("  Distance Currently: ");
  Serial.println(distance);
  Serial.println("  Moving forward...");
  spinMotors(calibrationSpeed);
  while (distance > 10.1) {
    //delay(20);
    distance = getDistanceInstant();
    Serial.print("  Distance now: ");
    Serial.println(distance);
  }
  Serial.println("  Moving rearward...");
  spinMotors(-0.75*calibrationSpeed);
  while (distance < 9.9) {
    //delay(20);
    distance = getDistanceInstant();
    Serial.print("  Distance now: ");
    Serial.println(distance);
  }
  spinMotors(0);
  Serial.println("Now at approx 10cm");
  double initialDistance = getDistance();
  Serial.print("  Distance actually: ");
  Serial.println(initialDistance);
  
  // Speed to CALIBRATIONSPEED backwards for 2 seconds
  Serial.println("Moving rearwards for 2 seconds");
  spinMotors(-calibrationSpeed);
  delay(2000);
  spinMotors(0);
  Serial.println("  Done");
  // Measure distance
  double finalDistance = getDistance();
  Serial.print("  Distance now: ");
  Serial.println(finalDistance);
  // calculate speed
  double rearwardSpeed = (finalDistance - initialDistance) / 200.0d; // 2 seconds, 100 cm in 1m
  Serial.print("Speed in reverse: ");
  Serial.print(rearwardSpeed);
  Serial.println(" metres per second");
  // Move back to 10cm based on new speed
  double tForward = (finalDistance - 10.0d)/100.0d / (rearwardSpeed);
  Serial.print("Moving forwards for ");
  Serial.print(tForward);
  Serial.println(" seconds");
  spinMotors(calibrationSpeed);
  delay(tForward * 1000.0d);
  spinMotors(0);
  Serial.println("  Done");
  // measure new distance
  finalDistance = getDistance();
  // print out delta
  Serial.print("Should now be at exactly 10cm. Distance actually: ");
  Serial.println(finalDistance);

  Serial.print("FINAL CALCULATED SPEED: ");
  Serial.print(rearwardSpeed);
  Serial.println(" metres per second");
  metresPerSecond = rearwardSpeed;

  speedIsCalibrated = true;
}


void spinMotors(int motorspeed) {
  if (0 != motorspeed && digitalRead(7) == HIGH) {
    Serial.println("MOTOR SWITCH SET TO OFF!!!");
    spinLeftMotor(0);
    spinRightMotor(0);
    green();
    return;
  }
  if (0 == motorspeed) {
    green();
  } else {
    yellow();
  }
  spinRightMotor(motorspeed);
  spinLeftMotor(motorspeed);
}

//------------------LOW LEVEL FUNCTIONS-------------------------------

// MULTI COLOR LED
void red() {
  // Turn the RGB LED red
  analogWrite(redPin, 255);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}

void green() {
  // Turn the RGB LED green
  analogWrite(redPin, 0);
  analogWrite(greenPin, 255);
  analogWrite(bluePin, 0);
}

void blue() {
  // Turn the RGB LED blue
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 255);
}

void yellow() {
  // Turn the RGB LED yellow
  analogWrite(redPin, 255);
  analogWrite(greenPin, 255);
  analogWrite(bluePin, 0);
}


// ULTRASOUND

double getDistanceInstant() {
  unsigned int echoTime;                   //variable to store the time it takes for a ping to bounce off an object
  double calculatedDistance;         //variable to store the distance calculated from the echo time

  echoTime = sonar.ping();
  //echoTime = sonar.ping_median(3); // few readings, for reading whilst moving
  //Serial.print("Echo time: ");
  //Serial.println(echoTime);
  //pinMode(echoPin,OUTPUT);
  //digitalWrite(echoPin,LOW);
  //pinMode(echoPin,INPUT);
  
  //calculatedDistance = sonar.convert_cm(echoTime); // rounds to nearest cm
  calculatedDistance = (1.0d * echoTime) / (1.0d * US_ROUNDTRIP_CM);
  return calculatedDistance;
}

double getDistance() {
  unsigned int echoTime;                   //variable to store the time it takes for a ping to bounce off an object
  float calculatedDistance;         //variable to store the distance calculated from the echo time
  
  echoTime = sonar.ping_median(100); // 100 readings, median. Returns median uS. Accurate to 24uS ~= 0.4cm
  //Serial.print("Echo time: ");
  //Serial.println(echoTime);
  //pinMode(echoPin,OUTPUT);
  //digitalWrite(echoPin,LOW);
  //pinMode(echoPin,INPUT);
  
  //calculatedDistance = sonar.convert_cm(echoTime); // rounds to nearest cm
  calculatedDistance = (1.0d * echoTime) / (1.0d * US_ROUNDTRIP_CM);
  return calculatedDistance;
}


// MOTOR FUNCTIONS

void spinRightMotor(int motorSpeed)                       //function for driving the right motor
{
  // right motor is the opposite way to left motor, so we've inverted them here
  if (motorSpeed < 0)                                 //if the motor should drive forward (positive speed)
  {
    digitalWrite(AIN1, HIGH);                         //set pin 1 to high
    digitalWrite(AIN2, LOW);                          //set pin 2 to low
  }
  else if (motorSpeed > 0)                            //if the motor should drive backward (negative speed)
  {
    digitalWrite(AIN1, LOW);                          //set pin 1 to low
    digitalWrite(AIN2, HIGH);                         //set pin 2 to high
  }
  else                                                //if the motor should stop
  {
    digitalWrite(AIN1, LOW);                          //set pin 1 to low
    digitalWrite(AIN2, LOW);                          //set pin 2 to low
  }
  analogWrite(PWMA, abs(motorSpeed));                 //now that the motor direction is set, drive it at the entered speed
}


void spinLeftMotor(int motorSpeed)                       //function for driving the left motor
{
  if (motorSpeed > 0)                                 //if the motor should drive forward (positive speed)
  {
    digitalWrite(BIN1, HIGH);                         //set pin 1 to high
    digitalWrite(BIN2, LOW);                          //set pin 2 to low
  }
  else if (motorSpeed < 0)                            //if the motor should drive backward (negative speed)
  {
    digitalWrite(BIN1, LOW);                          //set pin 1 to low
    digitalWrite(BIN2, HIGH);                         //set pin 2 to high
  }
  else                                                //if the motor should stop
  {
    digitalWrite(BIN1, LOW);                          //set pin 1 to low
    digitalWrite(BIN2, LOW);                          //set pin 2 to low
  }
  analogWrite(PWMB, abs(motorSpeed));                 //now that the motor direction is set, drive it at the entered speed
}
