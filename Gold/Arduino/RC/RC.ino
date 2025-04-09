
// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "TMPL4xawEnwWI"
#define BLYNK_TEMPLATE_NAME "2E10Buggy"
#define BLYNK_AUTH_TOKEN "j5SQr2abW-x2-mPHU0n3ozpYZcJx9p9e"

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#include <WiFiS3.h>
#include <WiFiClient.h>
#include <BlynkSimpleWifi.h>
#include "wifiAP.h"
#include "functions.h"

#include "PID.h"
#include "PIDv.h"
#include "camera.h"

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
/* char ssid[] = "VM1335311";
char pass[] = "hw2Rwkntfwcx"; */

char ssid[] = "Guru";
char pass[] = "oycp4582";


String MD = "";
int timer;
char cmd; 
char previouscmd;
int current_ID;
int previous_ID;
int action_count = 0;
bool spin_count = false;
int stop_time;
int counter_B = 0;


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

BLYNK_WRITE(V4) { //turn right
      cmd = 'c';
}


void setup()
{
  // Debug console
  Serial.begin(115200);

  SetPins();

  //showing group number on LED display on bootup
  ShowGroupNo();

  //setting up arduino as server via access point
  Wifisetup(); 

  //huskeylens code
  camerasetup();

  //Blynk.begin(auth, ssid, pass);
  // You can also specify server:

  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
  
    //encoder pins setup
  pinMode(EL, INPUT_PULLUP);
  pinMode(ER, INPUT_PULLUP);

  //attaching interrupts
  attachInterrupt(digitalPinToInterrupt(EL), IRAM_ATTR_L, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ER), IRAM_ATTR_R, CHANGE);

  Wifisetup();
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


  printcmd(cmd); //printing command to console, debugging 

   if (cmd == 'g') {  
     printcmd(cmd);
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
    client.flush(); //clear transmisson and recieve stream
    if(distance > 10){
      client.print(MD); //sending message to GUI on processor only when the an object isn't detected
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
  //recieving stop command
  else if(cmd=='s' ) {
    stopMotors();
  } 
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
  else if (cmd == 'u'){
    PIDcalculatev();
    client.flush();
    client.print("Current Speed" + String(round((actual_RPM/60.0)*6.5*3.1415)) + " Target Speed: " + String(round((target_RPM/60.0)*6.5*3.1415)));
    traverse_v();
  }
  else if (cmd == 'c') {
    cameraloop();

    if(getID() != 0){
        action_count = 1;
        current_ID = getID();
    }
      
    //Serial.println(current_ID);

    while((current_ID == 0 ) && ( white())){
        moveRight();
        delay(200);
        stopMotors();
        delay(500);
        Serial.println("works");
    }

    if(current_ID == 0){
      traverseT();
      client.flush();
      client.print("STRAIGHT");
    }


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

    else if (current_ID == 7){
      if(previous_ID != 7){
        stop_time = millis();
        client.flush();
        client.print("STOP");
        current_ID = 0;
        action_count = 0;
        previous_ID = 7;
      }
      else {
        if(millis() < stop_time + 5000){
          client.flush();
          client.print("STOP");
          stopMotors();
        }
        else{
          traverseT();
          client.flush();
          client.print("STRAIGHT");
        }
      }
    }
  

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

    else if (current_ID == 9){
      client.flush();
      client.print("HairpinR");

      if(!spin_count){
        spin();
        spin_count = true;
      }

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

    else if (current_ID == 10){
      client.flush();
      client.print("HairpinL");
      if(!spin_count){
        spin();
        spin_count = true;
      }

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

