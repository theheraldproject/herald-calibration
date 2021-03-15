/*
 * 2C - Find minimum reliable speed
 * Calibrate minimum consistent wheel speed for this robot, weight, and surface
 * Create by Adam Fowler on 28/Sep/2020
 * Original sketch was written by SparkFun Electronics, with lots of help from the Arduino community.
 * Copyright © 2020 VMware, Inc.
 * SPDX-License-Identifier: MIT
*/

#include <NewPing.h>

#include "Wire.h"
#include "sensorbar.h"

// TEMPERATURE
const int tempPin = A3;

// ULTRASOUND

// NewPing with sinlge pin operation for HC-SR04
const int trigPin = 6;           //connects to the trigger pin on the distance sensor
const int echoPin = 6;           //connects to the echo pin on the distance sensor
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


// LINE FOLLOWER

// Uncomment one of the four lines to match your SX1509's address
//  pin selects. SX1509 breakout defaults to [0:0] (0x3E).
const uint8_t SX1509_ADDRESS = 0x3E;  // SX1509 I2C address (00)
//const byte SX1509_ADDRESS = 0x3F;  // SX1509 I2C address (01)
//const byte SX1509_ADDRESS = 0x70;  // SX1509 I2C address (10)
//const byte SX1509_ADDRESS = 0x71;  // SX1509 I2C address (11)

SensorBar mySensorBar(SX1509_ADDRESS);

//Define the states that the decision making machines uses:
#define IDLE_STATE 0
#define READ_LINE 1
#define GO_FORWARD 2
#define GO_LEFT 3
#define GO_RIGHT 4

uint8_t state;

// PROGRAMME VARIABLES

bool speedIsCalibrated = false;

int calibrationSpeedInterval = 10;
int calibrationSpeed = 50;
float metresPerSecond = 0.1f;

void setup()
{
  // SERIAL
  Serial.begin (115200);        //set up a serial connection with the computer
  Serial.println("======================================");

  // TEMPERATURE
  pinMode(tempPin, INPUT);

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

  // Dummy initial readings (ultrasound initialisation issue)
  getDistance();

  // LINE FOLLOWER 
  
  //Default: the IR will only be turned on during reads.
  mySensorBar.setBarStrobe();
  //Other option: Command to run all the time
  //mySensorBar.clearBarStrobe();

  //Default: dark on light
  mySensorBar.clearInvertBits();
  //Other option: light line on dark
  //mySensorBar.setInvertBits();
  
  //Don't forget to call .begin() to get the bar ready.  This configures HW.
  uint8_t returnStatus = mySensorBar.begin();
  if(returnStatus)
  {
    Serial.println("sx1509 IC communication OK");
  }
  else
  {
    Serial.println("sx1509 IC communication FAILED!");
  }
  Serial.println();
}

void loop() {
  if (!speedIsCalibrated) {
    calibrateSpeed();
    flashRed(5);
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
  // FIRST CALIBRATE MOVEMENT
  flashYellow(1);
  Serial.println("Calibrating minimum reliable distance speed");
  float distance = getDistance();
  float newDistance = distance;
  do {
    distance = newDistance;
    calibrationSpeed += calibrationSpeedInterval;
    calibrationSpeed = calibrationSpeed % 256;
    Serial.print("  Trying speed: ");
    Serial.println(calibrationSpeed);
    unsigned long target = millis() + 500;
    while (target > millis()) {
      steer(calibrationSpeed);
    }
    //delay(500);
    newDistance = getDistance();
  } while (abs(newDistance - distance) < 8); // pick something that's ~2 SDs from mean error
  steer(0);
  distance = getDistance();
  Serial.print("  Distance now: ");
  Serial.println(distance);
  calibrationSpeed *= 1.2; // just to be sure
  Serial.print("  Calibration speed: ");
  Serial.println(calibrationSpeed);

  // Pause
  delay(2000);
  
  // NOW CALIBRATE SPEED
  flashYellow(2);
  Serial.println("Moving to 20cm (Naive)...");
  // Move to 20cm
  Serial.print("  Distance Currently: ");
  Serial.println(distance);
  Serial.println("  Moving forward...");
  while (distance > 20.1) {
    steer(calibrationSpeed);
    distance = getDistanceInstant();
    Serial.print("  Distance now: ");
    Serial.println(distance);
  }
  Serial.println("  Moving rearward...");
  while (distance < 19.9) {
    steer(-0.9*calibrationSpeed);
    distance = getDistanceInstant();
    Serial.print("  Distance now: ");
    Serial.println(distance);
  }
  steer(0);
  Serial.println("Now at approx 20cm");
  double initialDistance = getDistance();
  Serial.print("  Distance actually: ");
  Serial.println(initialDistance);
  
  // Speed to CALIBRATIONSPEED backwards for 2 seconds
  flashYellow(3);
  Serial.println("Moving rearwards for 4 seconds");
  unsigned long target = millis() + 4000;
  while (target > millis()) {
    steer(-1 * calibrationSpeed);
  }
  steer(0);
  Serial.println("  Done");
  // Measure distance
  double finalDistance = getDistance();
  Serial.print("  Distance now: ");
  Serial.println(finalDistance);
  // calculate speed
  double rearwardSpeed = (finalDistance - initialDistance) / 400.0d; // 4 seconds, 100 cm in 1m
  Serial.print("Speed in reverse: ");
  Serial.print(rearwardSpeed);
  Serial.println(" metres per second");
  // Move back to 20cm based on new speed
  double tForward = (finalDistance - 20.0d)/100.0d / (rearwardSpeed);
  flashYellow(4);
  Serial.print("Moving forwards for ");
  Serial.print(tForward);
  Serial.println(" seconds");
  target = millis() + (tForward * 1000); // seconds to milliseconds!
  while (target > millis()) {
    steer(calibrationSpeed);
  }
  steer(0);
  Serial.println("  Done");
  // measure new distance
  finalDistance = getDistance();
  // print out delta
  Serial.print("Should now be at exactly 20cm. Distance actually: ");
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



//When using driveBot( int16_t driveInput ), pass positive number for forward and negative number for backwards
//driveInput can be -255 to 255
void driveBot( int16_t driveInput )
{
  int16_t rightVar;
  int16_t leftVar;
  rightVar = driveInput;
  leftVar = driveInput;
  spinRightMotor(rightVar);
  spinLeftMotor(leftVar);
}

//When using ( int16_t driveInput, float turnInput ), pass + for forward, + for right
//driveInput can be -255 to 255
//turnInput can be -1 to 1, where '1' means turning right, right wheel stopped
void driveTurnBot( int16_t driveInput, float turnInput )
{
  Serial.print("      driveInput(in): ");
  Serial.print(driveInput);
  Serial.print(", turnInput(in): ");
  Serial.print(turnInput);
  int16_t rightVar;
  int16_t leftVar;
  //if driveInput is negative, flip turnInput
  if( driveInput < 0 )
  {
    turnInput *= -1;
  }
  
  //If turn is positive
  if( turnInput > 0 )
  {
    rightVar = driveInput * ( 1 - turnInput );
    leftVar = driveInput * ( 1 + turnInput );
  }
  else
  {
    rightVar = driveInput * (1 + ( turnInput * -1));
    leftVar = driveInput * (1 - ( turnInput * -1));
  }
  Serial.print(", turnInput (modified): ");
  Serial.print(turnInput);
  Serial.print(", Rspeed: ");
  Serial.print(rightVar);
  Serial.print(", Lspeed: ");
  Serial.println(leftVar);

  spinRightMotor(rightVar);
  spinLeftMotor(leftVar);
  delay(5);
}

//When using turnBot( float turnInput ), pass + for spin right.
//turnInput can be -1 to 1, where '1' means spinning right at max speed
void turnBot( float turnInput )
{
  int16_t rightVar;
  int16_t leftVar;
  //If turn is positive
  if( turnInput > 0 )
  {
    rightVar = -1 * 255 * turnInput;
    leftVar = 1 * 255 * turnInput;
  }
  else
  {
    rightVar = 255  * turnInput * 1;
    leftVar = 255  * turnInput * -1;
  }

  spinRightMotor(rightVar);
  spinLeftMotor(leftVar);
  delay(5);
}

//------------------LOW LEVEL FUNCTIONS-------------------------------

// MULTI COLOR LED
void flashRed(int times) {
  for (int i = 0;i < times;i++) {
    red();
    delay(200);
    ledOff();
    delay(200);
  }
}
void flashYellow(int times) {
  for (int i = 0;i < times;i++) {
    yellow();
    delay(200);
    ledOff();
    delay(200);
  }
}
void flashGreen(int times) {
  for (int i = 0;i < times;i++) {
    green();
    delay(200);
    ledOff();
    delay(200);
  }
}
void ledOff() {
  // Turn the RGB LED off
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}
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
float getVoltage(int pin) { 
  return (analogRead(pin) * .004882814); 
  //Converting from 0 to 1024 to 0 to 5v 
}

float millisecondsToCentimeters(float milliseconds, float temp) { 
  return (milliseconds * (331.3 + 0.606 * temp)) / 20000.0f; // half the flight time, divided by 1000 for milliseconds
  //Multiplying the speed of sound through a certain temperature of air by the 
  //length of time it takes to reach the object and back, divided by two
}

double getDistanceInstant() {
  unsigned int echoTime;                   //variable to store the time it takes for a ping to bounce off an object
  double calculatedDistance;         //variable to store the distance calculated from the echo time

  float temperature = (getVoltage(tempPin) - 0.5) * 100;
  echoTime = sonar.ping();
  //echoTime = sonar.ping_median(3); // few readings, for reading whilst moving
  //Serial.print("Echo time: ");
  //Serial.println(echoTime);
  //pinMode(echoPin,OUTPUT);
  //digitalWrite(echoPin,LOW);
  //pinMode(echoPin,INPUT);
  
  //calculatedDistance = sonar.convert_cm(echoTime); // rounds to nearest cm
  //calculatedDistance = (1.0d * echoTime) / (1.0d * US_ROUNDTRIP_CM);
  calculatedDistance = millisecondsToCentimeters(echoTime, temperature);
  return calculatedDistance;
}

float getDistance() {
  unsigned int echoTime;                   //variable to store the time it takes for a ping to bounce off an object
  float calculatedDistance;         //variable to store the distance calculated from the echo time

  float temperature = (getVoltage(tempPin) - 0.5) * 100;
  echoTime = sonar.ping_median(20); // uS
  //Serial.print("Echo time: ");
  //Serial.println(echoTime);
  
  //float oldCalculatedDistance = (1.0d * echoTime) / (1.0d * US_ROUNDTRIP_CM);
  calculatedDistance = millisecondsToCentimeters(echoTime, temperature);
  //Serial.print("Uncorrected distance calculation: ");
  //Serial.print(oldCalculatedDistance);
  //Serial.print(" => ");
  //Serial.print(calculatedDistance);
  //Serial.print(" @ ");
  //Serial.print(temperature);
  //Serial.println(" deg C");
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

// LINE FOLLOWER
uint8_t readLine() {
  uint8_t nextState = state;
  if( mySensorBar.getDensity() < 7 )
  {
    nextState = GO_FORWARD;
    if( mySensorBar.getPosition() < -50 )
    {
      nextState = GO_LEFT;
    }
    if( mySensorBar.getPosition() > 50 )
    {
      nextState = GO_RIGHT;
    }
  }
  else
  {
    nextState = IDLE_STATE; // not on a line
  }
  return nextState;
}

void steer(int currentSpeed) {
  if (0 != currentSpeed && digitalRead(7) == HIGH) {
    //Serial.println("MOTOR SWITCH SET TO OFF!!!");
    currentSpeed = 0;
  }
  if (0 == currentSpeed) {
    green();
  } else {
    yellow();
  }
  uint8_t nextState = readLine();
  //Serial.print("  Next read line state: ");
  //Serial.println(nextState);
  if (GO_LEFT == nextState) {
    //Serial.println("    Steering left");
    driveTurnBot(currentSpeed,-0.20);
  } else if (GO_RIGHT == nextState) {
    //Serial.println("    Steering right");
    driveTurnBot(currentSpeed,0.20);
  } else if (IDLE_STATE == nextState) {
    //Serial.println("    Staying idle");
    driveTurnBot(0,0);
  } else {
    // move forward
    //Serial.println("    Steering straight");
    driveTurnBot(currentSpeed,0);
  }
  state = nextState;
}
