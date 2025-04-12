//the following code implements PID control for maintaining a target distance of 15cm. 

#pragma once
#include <WiFiS3.h>
#include "functions.h"
#include <cstring>
#include <Arduino.h>

//PID variables 

double target_distance = 15; 
double actual_distance; 


long long proportional = 0;
long long integral = 0;
long long derivative = 0;

long long error =0;
long long last_error = 0;

long long  last_time = 0;
long long  current_time;



float velocity_PWM = 0;

//current rpm. 
double rpm;


//Proportional, integral and derivative control variables
double kp;
double ki;
double kd;

double max_derivative = 1000; 

//
void SetConstantsD(double KP = 1,double KI = 1,double KD = 1){
  kp = KP;
  ki = KI;
  kd = KD;
}


void SetSpeedPIDD(float Speed) {
  
   median_speed += Speed;

  // Clamp the median_speed within 0-190 range, the median speed is the actual PWM value passed to the PWM pins.
  //Note: median_speed is declared in functions.h and used for the moment functions in traverse.
  if (median_speed < 0) {
    median_speed = 0;
  } 
  else if (median_speed > 190) {
    median_speed = 190;
  }
}


//passing in the desired PID coefficents to be used. 
void PIDsetupD(){
  SetConstantsD(1, 0.1, 100); //P is tracking, I is long term error, d is reactivity.
}

//calculating the update needed for PID control of the speed
void PIDcalculateD(){

  //checking the distance between the target and the buggy via the US. PID will only be taken to run when the distance is less than 70cm to the buggy.
  //implements the negative feedback loop.
  checkdistance();
  actual_distance = distance;

  if((distance < 70) && (distance > 5)) {
    current_time = millis() / 1000.0; // Ensure floating-point precision

    error = actual_distance - target_distance;

    double time_diff = current_time - last_time;
    
    if (time_diff > 0) { // Prevent division by zero
        derivative = (error - last_error) / time_diff;
        // Limit derivative to prevent overflow
      if (derivative > max_derivative) {
          derivative = max_derivative;
        } 
      else if (derivative < -max_derivative) {
          derivative = -max_derivative;
        }
    } 
    else {
      derivative = 0;
    }

    // RPM calculation
    rpm = ((pulseCountL / 8) / time_diff) * 60;

    last_time = current_time;
    last_error = error;

    //calculating the integral term
    integral += error;

    //limiting how large the integral term can get to ensure that the term doesn't grow too fast. 
    if(integral > 5){
      integral = 5;
    }
    else if (integral < -5){
      integral = -5;
    }

    //setting the proportional term
    proportional = error;

    //calculating the update to the PWM value via PID
    velocity_PWM = kp * proportional + ki * integral + kd * derivative;

    //updating the PWM value passed to the PWM pins 
    SetSpeedPIDD(velocity_PWM);
    //delay to ensure changes can be applied 
    delay(100);
  }
  //if no object is detected (i.e further than 70cm away, stop motors)
  else {
    stopMotors();
  }
}