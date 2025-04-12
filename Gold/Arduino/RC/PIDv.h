//the following code implements PID control for maintaining a speed sent from the processing GUI.
//note that calculations are done in RPM. conversions are done in processing and arduino for communication purposes only.

#pragma once
#include <WiFiS3.h>
#include "functions.h"
#include <cstring>
#include <Arduino.h>

//velocity PID variables
//default target RPM is 70.
float target_RPM= 70; 
int actual_RPM; 
int actual_rpm_R;
int actual_rpm_L; 

long long errorv =0;
long long last_errorv = 0;

long long proportionalv = 0;
long long integralv = 0;
long long derivativev = 0;

long long  last_time_dv = 0;
long long  current_time_dv;
float velocity_PWMv = 0;

double rpmv;

double kpv;
double kiv;
double kdv;

double max_derivativev = 1000; 
int time_for_rpm_calc = 200; 
int update = 200;

//function to set PID constants for velocity control.
void SetConstantsv(double KP = 1,double KI = 1,double KD = 1){
  kpv = KP;
  kiv = KI;
  kdv = KD;
}

//function update PWM value passed to PWM pins 
void SetSpeedPIDv(float Speed) {
  
  median_speedv += Speed;

  // Clamp the median speed within 0-255 range
  if (median_speedv < 0) {
    median_speedv = 0;
  } 
  else if (median_speedv > 255) {
    median_speedv = 255;
  }
}

//intializing PID constants. 
void PIDsetupv(){
  SetConstantsv(0.0059,0.000002,0.12); //P is tracking, I is long term error, d is reactivity.
}

//calculating the update needed for PID control of the speed
void PIDcalculatev(){
  if(millis() > time_for_rpm_calc){

    current_time_dv = millis() / 1000.0; // Ensure floating-point precision
    unsigned long time_diff_dv = current_time_dv - last_time_dv;

    //calculating the current RPM from encoders to implement negative feedback loop.
    if (time_diff_dv > 0) { // Prevent division by zero
      time_for_rpm_calc =  update + time_for_rpm_calc ;
      target_RPM = getSentSpeed();
      actual_rpm_L = ((pulseCountL / 8.0) / time_diff_dv) * 60.0;
      actual_rpm_R = ((pulseCountR / 8.0) / time_diff_dv) * 60.0;

      //resetting pulsecounters for each encoder
      pulseCountL = 0;
      pulseCountR = 0;
      //RPM is taken as the average from the left and right encoder.
      actual_RPM = (actual_rpm_L + actual_rpm_R)/2;

      //derivative calculations
      derivativev = (errorv - last_errorv) / time_diff_dv;
        // Limit derivative to prevent overflow
      if (derivativev > max_derivativev) {
          derivativev = max_derivativev;
        } 
      else if (derivativev < -max_derivativev) {
          derivativev = -max_derivativev;
        }
    } 
    else {
      derivativev = 0;
    }
    //calculate error;
    errorv = target_RPM - actual_RPM ;

    last_time_dv = current_time_dv;
    last_errorv = errorv;

    integralv += errorv;

    //capping integral to prevent extreme growth. 
    if(integralv > 40){
      integralv = 40;
    }
    else if (integralv < -40){
      integralv = -40;
    }

    proportionalv = errorv;
    //calcuating the update 
    velocity_PWMv =  proportionalv * kpv + integralv * kiv + derivativev * kdv;
    //updating the PWM value passed to the PWM pins. 
    SetSpeedPIDv(velocity_PWMv);
    //if turning is occuring, artifical input is passed to ensure smoother transition to target speed. 
    if(right() || left()){
      actual_RPM = target_RPM;
    }
  }
}