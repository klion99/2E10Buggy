#pragma once
#include "wifiAP.h"
#include <Arduino.h>

#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"


//declaring the matrix of the LED on the arduino
ArduinoLEDMatrix matrix;

//decleration of all motor related pins
const int LF = 4;
const int LB = 8;
const int RF = 7;
const int RB = 6;
const int CL = 10;
const int CR = 11;

//PID speed variables - voltage
float median_speedv = 100;
int median_speed = 100;

//IR D0 pins
const int LEYE = A1;
const int REYE = A0;


//ultrasound pins
const int US_TRIG = 13;
const int US_ECHO = 12;

//encoder pins
const int EL = 2 ; 
const int ER = 3 ; 

//decleation of all variables and constants used
const int delaytime = 2000;
int distance = 0;
int LastDistance = 0;

//strings used for  printing for debugging
String prevprint = "";
char prevcmd = '\n';

//variable for the time taken to recieve the echo 
long duration = 0;

//declaring the variables for the encoders
volatile int pulseCountL = 0;
volatile int pulseCountR = 0;

//declaring the variables for the distances from the left and right motors
float distanceL = 0;
float distanceR = 0;

//declaring the mean distance variable for each motor.
float meandistance = 0;



//interrupt functions that are used to update the pulsecounts to calculate distance
void IRAM_ATTR_L() {
  pulseCountL++;
}

void IRAM_ATTR_R() {
  pulseCountR++;
}


//setting up all motor and IR pins
void SetPins(){
  //setting motor pins
  pinMode(LF, OUTPUT);
  pinMode(LB, OUTPUT);
  pinMode(RF, OUTPUT);
  pinMode(RB, OUTPUT);

  //IR pins
  pinMode(LEYE, INPUT);
  pinMode(REYE, INPUT);

  //encoder pins setup
  pinMode(EL, INPUT_PULLUP);
  pinMode(ER, INPUT_PULLUP);

  //attaching interrupts
  attachInterrupt(digitalPinToInterrupt(EL), IRAM_ATTR_L, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ER), IRAM_ATTR_R, CHANGE);
}

//group number on arduino boot up function
void ShowGroupNo(){
  matrix.begin();
 //showing group number on bootup!
  matrix.beginDraw(); //initalize the matrix led
  matrix.stroke(0xFFFFFFF);//setting the color
  const char text[] = " Z9"; //converting text to an array of C style characters
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText();


  matrix.endDraw();
}


//setspeed function for motor speeds for left motor, right or both
void SetSpeed(char L_R_B, int Speed) {// L_R_B = Left or Right or Both
  if (L_R_B == 'L') {
    analogWrite(CL, Speed);
    digitalWrite(LF, HIGH);
    digitalWrite(LB, LOW);
  }
  else if (L_R_B == 'R'){
    analogWrite(CR, Speed);
    digitalWrite(RF, HIGH);
    digitalWrite(RB, LOW);
  }
  else if (L_R_B == 'B'){
      analogWrite(CR, Speed);
      digitalWrite(RF, HIGH);
      digitalWrite(RB, LOW);


      analogWrite(CL, Speed);
      digitalWrite(LF, HIGH);
      digitalWrite(LB, LOW);
    }
}


//move right function that spins left motor faster than right
void moveRight() {
  SetSpeed('R', 0);
  SetSpeed('L', 105);
  
}


//move left function that spins right motor faster than left
void moveLeft() {
  SetSpeed('R', 105);
  SetSpeed('L', 0);
}


//stops motors
void stopMotors() {
  SetSpeed('B', 0);
}


//can set the speed at which motors move forward here

//for go mode 
void moveForward() {
  SetSpeed('R', 85);
  SetSpeed('L', 85);

}

//for distance following mode
void moveForwardD() {
  SetSpeed('R', median_speed);
  SetSpeed('L', median_speed);
}

//for velocity following mode
void moveForwardv() {
  SetSpeed('R', median_speedv);
  SetSpeed('L', median_speedv);
}



//function which updates the distance variable for checking distance
void checkdistance(){
  digitalWrite( US_TRIG, LOW );
  delayMicroseconds(2);
  digitalWrite( US_TRIG, HIGH );
  delayMicroseconds( 10 );
  digitalWrite( US_TRIG, LOW );

  duration = pulseIn( US_ECHO, HIGH );
  distance = duration/58;

  //debugging code for distance when less than 10cm, prints to serial monitor
/*     if(distance < 10) {
      if (distance != LastDistance){
        Serial.print("Distance detected: ");
        Serial.print( distance );
        Serial.println(" cm");
        LastDistance = distance;
      }
    } */
}

//function for printing strings only once to monitor, for debugging
void PrintOnce(String Print) {
  if (Print != prevprint){
    Serial.println(Print);
    prevprint = Print;
  }
}

//only black being detected on both IR sensors, returns bool to run the correct function;
bool forward(){
  if((digitalRead(LEYE) == HIGH) && (digitalRead(REYE) == HIGH)){
  return true;
  }
  else return false;
}

  /*this statement is updated if the left IR sensor detects a white line, initially it is set as false, the interupt will change to high making this true and runs
  the move left function that turns the right motor faster than the left*/
bool right(){
  if ((digitalRead(LEYE) == HIGH) && (digitalRead(REYE) == LOW)) {
    return true;
  }
  else return false;
}

  /*this statement is updated if the right IR sensor detects a white line, initially it is set as false, the interupt will change this to high making this true, and runs
  the move right function that turns the left motor faster than the right*/
bool left() {
  if ((digitalRead(LEYE)== LOW) && (digitalRead(REYE) == HIGH)) {
    return true;
  }
  else return false;
}
  
//case when both IR sensors are on white line, currently stops the motor via returning a boolean 
bool white() {
  if ((digitalRead(LEYE)==LOW) && (digitalRead(REYE)==LOW)) {
    return true;
  }
  else return false;
}


void printcmd(char cmd){
   if(cmd != prevcmd){
    Serial.println(cmd);
    prevcmd = cmd;
  }
}


//code that setsup LED display to show the current distance measured when less than 10.
void displayDistance(){
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(50);
  // add the text
  String d = "   " + String(distance);
  const char text[] = " cm";
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(d);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}

//for distance following mode
void traverse(){
  //checking distance
    checkdistance(); //runs US and displays distance on arduino
    if(distance < 5){
      stopMotors(); 
      client.flush(); //clear transmisson and recieve stream
      delay(200); //wait to clear transmisson and recieve stream
      client.print("O"); //sending message to GUI on processor
      //what to do when distance is less than 15cm
      delay(200); //wait to update distance again.
      displayDistance(); //display measured distance on Arduino LED panel
      checkdistance(); //update distance again.
    }
    //if the bools left and right detected are off i.e IR sensors do not detect white lines, the motorforward command runs, motors turn at the same speed
    if (forward()) {
      //uses median_speed, updated via PID
      moveForwardD();
      PrintOnce("Moving forward");
    }

    //if condition to turn the buggy to the right
    if (right()) {
      moveRight();
      PrintOnce("Turning Right");
    }

    //if condition to turn the buggy to the left
    if (left()) {
        moveLeft();
        PrintOnce("Turning Left");
    }

    //if condition for when both the buggy's IR sensors detect white
  if (white()) {
      //moveForwardD();
      stopMotors();
      PrintOnce("something is wrong");
    }
  }

void traverse_v(){
  //checking distance
    checkdistance(); //runs US and displays distance on arduino
    if(distance < 10){
      stopMotors(); 
      client.flush(); //clear transmisson and recieve stream
      delay(200); //wait to clear transmisson and recieve stream
      client.print("O"); //sending message to GUI on processor
      //what to do when distance is less than 15cm
      delay(200); //wait to update distance again.
      displayDistance(); //display measured distance on Arduino LED panel
      checkdistance(); //update distance again.
    }
    //if the bools left and right detected are off i.e IR sensors do not detect white lines, the motorforward command runs, motors turn at the same speed
    if (forward()) {
      //uses median_speedv, updated via PID
      moveForwardv();
      PrintOnce("Moving forward");
    }

    //if condition to turn the buggy to the right
    if (right()) {
      moveRight();
      PrintOnce("Turning Right");
    }

    //if condition to turn the buggy to the left
    if (left()) {
        moveLeft();
        PrintOnce("Turning Left");
    }

    //if condition for when both the buggy's IR sensors detect white
  while (white()) {
      stopMotors();
      PrintOnce("something is wrong");
    }
  }

void traverseT(){
  //checking distance
    checkdistance(); //runs US and displays distance on arduino
    if(distance < 5){
      stopMotors(); 
      client.flush(); //clear transmisson and recieve stream
      delay(200); //wait to clear transmisson and recieve stream
      client.print("O"); //sending message to GUI on processor
      //what to do when distance is less than 15cm
      delay(200); //wait to update distance again.
      displayDistance(); //display measured distance on Arduino LED panel
      checkdistance(); //update distance again.
    }
    //if the bools left and right detected are off i.e IR sensors do not detect white lines, the motorforward command runs, motors turn at the same speed
    if (forward()) {
      //uses median_speed, updated via PID
      moveForward();
      PrintOnce("Moving forward");
    }

    //if condition to turn the buggy to the right
    if (right()) {
      moveRight();
      PrintOnce("Turning Right");
    }

    //if condition to turn the buggy to the left
    if (left()) {
        moveLeft();
        PrintOnce("Turning Left");
    }

    //if condition for when both the buggy's IR sensors detect white
  if (white()) {
      //moveForwardD();
      stopMotors();
      PrintOnce("something is wrong");
    }
  }
  





  
