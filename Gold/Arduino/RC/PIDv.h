#pragma once
#include <WiFiS3.h>
#include "functions.h"
#include <cstring>
#include <Arduino.h>


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

void SetConstantsv(double KP = 1,double KI = 1,double KD = 1){
  kpv = KP;
  kiv = KI;
  kdv = KD;
}

void SetSpeedPIDv(float Speed) {
  
  median_speedv += Speed;

  // Clamp the median_speed within 0-255 range
  if (median_speedv < 0) {
    median_speedv = 0;
  } 
  else if (median_speedv > 255) {
    median_speedv = 255;
  }
}


void PIDsetupv(){
  SetConstantsv(0.0059,0.000002,0.12); //P is tracking, I is long term error, d is reactivity.
}

void PIDcalculatev(){
  if(millis() > time_for_rpm_calc){

    current_time_dv = millis() / 1000.0; // Ensure floating-point precision
    unsigned long time_diff_dv = current_time_dv - last_time_dv;

    if (time_diff_dv > 0) { // Prevent division by zero
      time_for_rpm_calc =  update + time_for_rpm_calc ;
      target_RPM = getSentSpeed();
      actual_rpm_L = ((pulseCountL / 8.0) / time_diff_dv) * 60.0;
      actual_rpm_R = ((pulseCountR / 8.0) / time_diff_dv) * 60.0;

      //resetting pulsecounters for each encoder
      pulseCountL = 0;
      pulseCountR = 0;
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

    if(integralv > 40){
      integralv = 40;
    }
    else if (integralv < -40){
      integralv = -40;
    }

    proportionalv = errorv;

    velocity_PWMv =  proportionalv * kpv + integralv * kiv + derivativev * kdv;

    SetSpeedPIDv(velocity_PWMv);

/*     Serial.print(velocity_PWMv);
    Serial.print(", ");
    Serial.print(actual_RPM); 
    Serial.print(", ");
    Serial.print(median_speedv); 
    Serial.println(", "); */

    if(right() || left()){
      actual_RPM = target_RPM;
    }
  }
}