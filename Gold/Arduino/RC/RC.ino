
// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
#define BLYNK_TEMPLATE_ID "TMPL4xawEnwWI"
#define BLYNK_TEMPLATE_NAME "2E10Buggy"
#define BLYNK_AUTH_TOKEN "j5SQr2abW-x2-mPHU0n3ozpYZcJx9p9e"

#define BLYNK_PRINT Serial

//header files declaration
#include <WiFiS3.h>
#include <WiFiClient.h>
#include <BlynkSimpleWifi.h>
#include "wifiAP.h"
#include "functions.h"

#include "PID.h"
#include "PIDv.h"
#include "camera.h"

//initializing authentication for the Blynk cloud
char auth[] = BLYNK_AUTH_TOKEN;

//note that these details have been changed for privacy. 
//Deceleration of hotspot details for arduino - Blynk server communication. The hotstop runs from Guru's phone. 
char ssid[] = "Guru";
char pass[] = "------";

//processing communication variable for the mean distance travelled thus far.
String MD = "";
//timer for encoder based distance calculatioons
int timer;
//command recived from processing GUI or phone GUI
char cmd; 
char previouscmd;

//deceleration of current ID and previous ID for implementing tag functionality and memory. 
int current_ID;
int previous_ID;

//implementing memory, action count enables checking if a tag action has been performed or not.
int action_count = 0;

//special action variable for the spin functionality. 
bool spin_count = false;
//counter for implementing stop tag functionality. 
int stop_time;

//counter of initializing connection to Blynk server for RC control only when needed
int counter_B = 0;


//Blynk functions, implements behavior based on input from Blynk GUI. Functions are enabled from Blynk.run() function.
BLYNK_WRITE(V0) { //move forward  
      digitalWrite(CR, param.asInt());
      digitalWrite(RF, HIGH);
      digitalWrite(RB, LOW);


      digitalWrite(CL, param.asInt());
      digitalWrite(LF, HIGH);
      digitalWrite(LB, LOW);
}

BLYNK_WRITE(V1) { //move backward
      digitalWrite(CR, param.asInt());
      digitalWrite(RF, LOW);
      digitalWrite(RB, HIGH);


      digitalWrite(CL, param.asInt());
      digitalWrite(LF, LOW);
      digitalWrite(LB, HIGH);
}

BLYNK_WRITE(V2) { //turn left
      digitalWrite(CR, param.asInt());
      digitalWrite(RF, HIGH);
      digitalWrite(RB, LOW);

      digitalWrite(CL, 0);
}

BLYNK_WRITE(V3) { //turn right
      digitalWrite(CR, 0);

      digitalWrite(CL, param.asInt());
      digitalWrite(LF, HIGH);
      digitalWrite(LB, LOW);
}

BLYNK_WRITE(V4) { //return to processing based control of the buggy. Additionally enables tag recognition for gold immedietly. 
      cmd = 'c';
}


void setup()
{
  Serial.begin(115200);

  //initialize all relavent pins - see functions.h
  SetPins();

  //showing group number on LED display on bootup
  ShowGroupNo();

  //setting up arduino as server via access point method
  Wifisetup(); 

  //huskeylens initalization
  camerasetup();

  
  //encoder pins setup
  pinMode(EL, INPUT_PULLUP);
  pinMode(ER, INPUT_PULLUP);

  //attaching interrupts
  attachInterrupt(digitalPinToInterrupt(EL), IRAM_ATTR_L, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ER), IRAM_ATTR_R, CHANGE);

  //enabling PID setup with preset coefficents. Coefficents can also be set here via the setConstants function in the order of kp, kd and ki. 
  PIDsetupD();
  PIDsetupv();
}

void loop()
{

while(cmd == 't'){
      if(counter_B < 1){
            Blynk.begin(auth, ssid, pass, "blynk.cloud", 8080);
            counter_B = counter_B + 1;
      }
      Blynk.run();
 }


  //all wifi related code that is looping, defined in WifiAP.h
  Wifiloop();
  if(counter_B == 1){
      counter_B = 0; 
      Wifisetup();
      delay(5000);
  } 


  //determines command sent from processing
  if((getcommand() == 't') && (previouscmd ==  't')){

  }
  else {
      cmd = getcommand(); 
      previouscmd = cmd;
  }


  printcmd(cmd); //printing command to console

    //command for simple line following (i.e bronze challange)
   if (cmd == 'g') {  
    //checking distance
    checkdistance(); //runs US and displays distance on arduino LED screen
    if(distance < 10){
      //condition to stop motors
      stopMotors(); 
      client.flush(); //clear transmisson and recieve stream
      delay(200); //wait to clear transmisson and recieve stream
      client.print("O"); //sending message to GUI on processor
      //what to do when distance is less than 15cm
      delay(200); //wait to update distance again.
      displayDistance(); //display measured distance on Arduino LED panel
      checkdistance(); //update distance again.
    }
    client.flush(); //clear transmisson and recieve stream
    if(distance > 10){
      client.print(MD); //sending message to GUI on processor only when the an object isn't detected i.e buggy isn't stationary
    }

    //if the bools left and right detected are off i.e IR sensors do not detect white lines, the motorforward command runs, motors turn at the same speed
    if (forward()) {
      moveForward();
      PrintOnce("Moving forward");
    }

    //if condition to turn the buggy to the right
    if (right()) {
      moveRight();
      client.flush();
      PrintOnce("Turning Right");
    }

    //if condition to turn the buggy to the left
    if (left()) {
        moveLeft();
        PrintOnce("Turning Left");
    }

    //if condition for when both the buggy's IR sensors detect white
    if (white()) {
      stopMotors();
      PrintOnce("something is wrong");
      delay(2000);
    }
    //counting the distance travelled via encoders
    if(millis() > timer) {
      //calculating the distance from the encoders of the left and right wheel
      distanceL = distanceL + (pulseCountL/8)*20.42;
      distanceR = distanceR + (pulseCountR/8)*20.42;

      //resetting pulsecounters for each encoder
      pulseCountL = 0;
      pulseCountR = 0;

      //calculating the mean distance as recorded from each wheel.
      meandistance = (distanceL + distanceR)/200;
      
      //updating the timer to only update distance every 2 seconds
      timer = timer + 2000;
      
      //updating the mean distance variable to send to the client
      MD = String(meandistance);
    }
  }
  //recieving stop command from processing
  else if(cmd=='s' ) {
    stopMotors();
  } 

  //distance tracking PID commented out and instead replaced with remote control mode. 

  /* else if(cmd=='t') {
    PIDcalculateD();

    client.flush();
    client.print("distance: " + String(distance) + " median_speed: " + String(median_speed));
    //delay(100);

    Serial.print("distance:");
    Serial.print(distance);
    Serial.print("  ");
    Serial.print("velocity_PWM:");
    Serial.print(velocity_PWM);
    Serial.print("median_speed:");

    Serial.print(median_speed);
    Serial.print(" Derivative:");
    Serial.println(kd * derivative);

    traverse();
  } */

  //command for initiating velocity control via PID
  else if (cmd == 'u'){
    //update PID variable, implementation in functions.h
    PIDcalculatev();
    //send the current speed and target speed to Processing.
    client.flush();
    client.print("Current Speed" + String(round((actual_RPM/60.0)*6.5*3.1415)) + " Target Speed: " + String(round((target_RPM/60.0)*6.5*3.1415)));
    //move the buggy via line control and use the speed calculated via PID
    traverse_v();
  }

  //command for initiating camera control
  else if (cmd == 'c') {
    cameraloop();

    //read in a tag, 0 is the no tag state the buggy defaults to. 
    if(getID() != 0){
        //increase action_count to 1 to know if a tag action has been performed or not.
        action_count = 1;
        //read in the ID as processed by camera.h
        current_ID = getID();
    }

    //implementing default turn, done via sudden burts of moving the buggy to the right. 
    while((current_ID == 0 ) && ( white())){
        stopMotors();
        delay(500);
        moveRight();
        delay(200);
        stopMotors();
        delay(500);
        Serial.println("works");
    }

    //default mode is line tracking
    if(current_ID == 0){
      traverseT();
      client.flush();
      client.print("STRAIGHT");
    }

    //tag read: 1 -> action: turn right at next junction
    //still performing default line tracking until junction, travel speed is a set speed. 
    else if (current_ID == 1){
      traverseT();
      client.flush();
      client.print("RIGHT");
      if (white()){
        while((digitalRead(LEYE)!=HIGH)){
          moveRight();
          action_count = 0;
          current_ID = 0;
          previous_ID = 1;
        }
      }
    }

    //tag read: 2 -> action: turn left at next junction
    //still performing default line tracking until junction, travel speed is a set speed. 
    else if (current_ID == 2){
      traverseT();
      client.flush();
      client.print("LEFT");

      if (white()){
        while((digitalRead(REYE)==LOW)){
          moveLeft();
          action_count = 0;
          current_ID = 0;
          previous_ID = 2;
        }
      }
    }
    //tag read: 3 -> action: max speed
    //performing line tracking implemented via PIDv.h and traverse_v
    else if (current_ID == 3){
      client.flush();
      client.print("20CMF \n");
      target_RPM = 59;
      PIDcalculatev();
      traverse_v();
      action_count = 0;
      client.flush();
      delay(20);
      client.print("VCurrent Speed" + String(round((actual_RPM/60.0)*6.5*3.1415)) + " Target Speed: " + String(round((target_RPM/60.0)*6.5*3.1415)));
      previous_ID = 3;
    }
    //tag read: 4 -> action: minimum speed
    //performing line tracking implemented via PIDv.h and traverse_v
    else if (current_ID == 4){
      client.flush();
      client.print("10CM \n");
      target_RPM = 29;
      PIDcalculatev();
      traverse_v();
      action_count = 0;
      client.flush();
      delay(20);
      client.print("VCurrent Speed" + String(round((actual_RPM/60.0)*6.5*3.1415)) + " Target Speed: " + String(round((target_RPM/60.0)*6.5*3.1415)));
      previous_ID = 4;
    }
    //tag read: 5 -> action: 15cm/s
    //performing line tracking implemented via PIDv.h and traverse_v
    else if (current_ID == 5){
      client.flush();
      client.print("15CM \n");
      target_RPM = 44;
      PIDcalculatev();
      traverse_v();
      action_count = 0;
      client.flush();
      delay(20);
      client.print("VCurrent Speed" + String(round((actual_RPM/60.0)*6.5*3.1415)) + " Target Speed: " + String(round((target_RPM/60.0)*6.5*3.1415)));
      previous_ID = 5;
    }
    //tag read: 6 -> action: 20cm/S
    //performing line tracking implemented via PIDv.h and traverse_v
    else if (current_ID == 6){
      client.flush();
      client.print("20CM \n");
      target_RPM = 59;
      PIDcalculatev();
      traverse_v();
      action_count = 0;
      client.flush();
      delay(20);
      client.print("VCurrent Speed" + String(round((actual_RPM/60.0)*6.5*3.1415)) + " Target Speed: " + String(round((target_RPM/60.0)*6.5*3.1415)));
      previous_ID = 6;
    }

    //tag read: 7 -> action: stop for 5 seconds
    else if (current_ID == 7){
      //only perform action once, i.e if the last tag read was 7 keep continuing as camera is read continously.
      if(previous_ID != 7){
        stop_time = millis();
        client.flush();
        client.print("STOP");
        current_ID = 0;
        action_count = 0;
        previous_ID = 7;
      }
      //using millis() to track time to stop.
      else {
        if(millis() < stop_time + 5000){
          client.flush();
          client.print("STOP");
          stopMotors();
        }
        //travel at speed speed 
        else{
          traverseT();
          client.flush();
          client.print("STRAIGHT");
        }
      }
    }
  
    //tag read: 8 -> action: 10cm/s
    //performing line tracking implemented via PIDv.h and traverse_v
    else if (current_ID == 8){
      client.flush();
      client.print("10CM");
      target_RPM = 29;
      PIDcalculatev();
      traverse_v();
      action_count = 0;
      client.flush();
      delay(20);
      client.print("VCurrent Speed" + String(round((actual_RPM/60.0)*6.5*3.1415)) + " Target Speed: " + String(round((target_RPM/60.0)*6.5*3.1415)));
      previous_ID = 8;
    }

    //tag read: 9 -> action: hairpin turn to the right
    //performing line tracking with default set speed
    else if (current_ID == 9){
      client.flush();
      client.print("HairpinR");
      //perform spin once. 
      if(!spin_count){
        spin();
        spin_count = true;
      }
      //perform right turn at next junction.
      traverseT();
      if (white()) {
        while((digitalRead(LEYE)!=HIGH)){
          moveRight();
          action_count = 0;
          current_ID = 0;
          spin_count = false;
          previous_ID = 9;
        }
      }
    }
    //tag read: 10 -> action: hairpin turn left
    //performing line tracking with default set speed
    else if (current_ID == 10){
      client.flush();
      client.print("HairpinL");

      //perform spin once
      if(!spin_count){
        spin();
        spin_count = true;
      }
      //perform left turn at next junction.
      traverseT();
      if (white()) {
        while((digitalRead(REYE)== LOW)){
          moveLeft();
          action_count = 0;
          current_ID = 0;
          previous_ID = 10;
          spin_count = false;
        }
      }
    }
  }
}

