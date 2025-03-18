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
<<<<<<< Updated upstream
int median_speed = 100;
long long velocity_PWM = 0;
=======

float velocity_PWM = 0;
>>>>>>> Stashed changes

double rpm;


//Proportional, integral and derivative control variables
double kp;
double ki;
double kd;

double max_derivative = 1000; 

void SetConstantsD(double KP = 1,double KI = 1,double KD = 1){
  kp = KP;
  ki = KI;
  kd = KD;
}

void SetSpeedPIDD(char L_R_B, int Speed, char type) {
  
  if (type == '+') {
    median_speed += Speed;
  } 
  else if (type == '-') {
    median_speed -= Speed;
  }

  // Clamp the median_speed within 0-255 range
  if (median_speed < 0) {
    median_speed = 0;
  } 
  else if (median_speed > 255) {
    median_speed = 200;
  }

  // Apply speed based on the L_R_B parameter
/*   if (L_R_B == 'L') {
    SetSpeed('L', median_speed);
  } else if (L_R_B == 'R') {
    SetSpeed('R', median_speed);
  } else { 
    SetSpeed('B', 1.2*median_speed);
  } */
}



void PIDsetupD(){
  SetConstantsD(7, 0.01, 50); //P is tracking, I is long term error, d is reactivity.
}

void PIDcalculateD(){
/*   if(distance < 15){
  kp = 10;
  ki = 0.1;
  }
  else {
    kp = 1;
    ki = 0.02;
  } */

  //checking the distance between the target and the buggy via the US. PID will only be taken to run when the distance is less than 50cm to the buggy.
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

    integral += error;

    proportional = error;

    velocity_PWM = kp * proportional + ki * integral + kd * derivative;

    

    delay(10);
  }
}

