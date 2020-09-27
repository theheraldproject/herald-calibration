/*
 * 1A - Motor validation
 * Validate that your calibration robot's motors are working as expected
 * Create by Adam Fowler on 27/Sep/2020
 * Original sketch was written by SparkFun Electronics, with lots of help from the Arduino community.
 * Copyright © 2020 VMware, Inc.
 * SPDX-License-Identifier: MIT
 */

//PIN VARIABLES
//the motor will be controlled by the motor A pins on the motor driver
const int AIN1 = 13;           //control pin 1 on the motor driver for the right motor
const int AIN2 = 12;            //control pin 2 on the motor driver for the right motor
const int PWMA = 11;            //speed control pin on the motor driver for the right motor

const int BIN1 = 8;
const int BIN2 = 9;
const int PWMB = 10;

int switchPin = 7;             //switch to turn the robot on and off

//VARIABLES
int leftMotorSpeed = 0;       //starting speed for the motor
int rightMotorSpeed = 0;       //starting speed for the motor
bool leftMotorNext = true;
bool hasPrinted = false;

void setup() {
  pinMode(switchPin, INPUT_PULLUP);   //set this as a pullup to sense whether the switch is flipped

  //set the motor control pins as outputs
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);

  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  Serial.begin(9600);                       //begin serial communication with the computer
}

void loop() {
  if (digitalRead(7) == HIGH) {
    spinLeftMotor(0);  
    spinRightMotor(0);  
  }
  if (leftMotorNext) {
    if (!hasPrinted) {
      Serial.println("Enter left motor speed (-255 to 255): ");
      hasPrinted = true;
    }
    if (Serial.available() > 0) {         //if the user has entered something in the serial monitor
      leftMotorSpeed = Serial.parseInt();     //set the motor speed equal to the number in the serial message
      Serial.read(); // reads 'enter' key
  
      Serial.print("Left Motor Speed: ");      //print the speed that the motor is set to run at
      Serial.println(leftMotorSpeed);
  
      if (digitalRead(7) == LOW) {          //if the switch is on...
        spinLeftMotor(leftMotorSpeed);
      } else {                              //if the switch is off...
        spinLeftMotor(0);                   //turn the motor off
      }
      leftMotorNext = false;
      hasPrinted = false;
    }
  }
  if (!leftMotorNext) {
    if (!hasPrinted) {
      Serial.println("Enter right motor speed (-255 to 255): ");
      hasPrinted = true;
    }
    if (Serial.available() > 0) {         //if the user has entered something in the serial monitor
      rightMotorSpeed = 0 - Serial.parseInt();     //set the motor speed equal to the number in the serial message
      Serial.read(); // reads 'enter' key
  
      Serial.print("Right Motor Speed: ");      //print the speed that the motor is set to run at
      Serial.println(rightMotorSpeed);
  
      if (digitalRead(7) == LOW) {          //if the switch is on...
        spinRightMotor(rightMotorSpeed);
      } else {                              //if the switch is off...
        spinRightMotor(0);                   //turn the motor off
      }
      leftMotorNext = true;
      hasPrinted = false;
    }
  }


}

/********************************************************************************/
void spinRightMotor(int motorSpeed)                       //function for driving the right motor
{
  if (motorSpeed > 0)                                 //if the motor should drive forward (positive speed)
  {
    digitalWrite(AIN1, HIGH);                         //set pin 1 to high
    digitalWrite(AIN2, LOW);                          //set pin 2 to low
  }
  else if (motorSpeed < 0)                            //if the motor should drive backward (negative speed)
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
