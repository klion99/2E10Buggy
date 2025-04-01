#include "functions.h"
#include "wifiAP.h"
#include "PID.h"
#include "PIDv.h"
#include "camera.h"

//message string for sending mean distance to client 
String MD = "";
int timer;
char cmd; 
int current_ID;
int previous_ID;
int action_count = 0;


void setup() {
  Serial.begin(115200);
 
  //setup of all sensor pins and motor pins and their types
  SetPins();

  //showing group number on LED display on bootup
  ShowGroupNo();

  //setting up arduino as server via access point
  Wifisetup(); 

  //huskeylens code
  camerasetup();


  //encoder pins setup
  pinMode(EL, INPUT_PULLUP);
  pinMode(ER, INPUT_PULLUP);

  //attaching interrupts
  attachInterrupt(digitalPinToInterrupt(EL), IRAM_ATTR_L, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ER), IRAM_ATTR_R, CHANGE);

  PIDsetupD();
  PIDsetupv();
}

void loop() {

  //all wifi related code that is looping, defined in WifiAP.h
  Wifiloop(); 


  //determines command sent from processing
  cmd = getcommand(); 
  //cmd = 'c'; 

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
  else if(cmd=='t') {
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
  }
  else if (cmd == 'u'){
    PIDcalculatev();
    client.flush();
    client.print("Current Speed" + String(round((actual_RPM/60.0)*6.5*3.1415)) + " Target Speed: " + String(round((target_RPM/60.0)*6.5*3.1415)));
    traverse_v();
  }
else if (cmd == 'c'){

    cameraloop();
      if(getID() != 0){
        action_count = 1;
        current_ID = getID();

      }
      
    Serial.println(current_ID);
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
        }
      }
    }

    else if (current_ID == 3){
      client.flush();
      client.print("20CMF");
      target_RPM = 59;
      PIDcalculatev();
      traverse_v();
      action_count = 0;
    }

    else if (current_ID == 4){
      if(action_count == 1){
        client.flush();
        client.print("10CM");
      }
      target_RPM = 29;
      PIDcalculatev();
      traverse_v();
      action_count = 0;
      client.flush();
    client.print("Current Speed" + String(round((actual_RPM/60.0)*6.5*3.1415)) + " Target Speed: " + String(round((target_RPM/60.0)*6.5*3.1415)));
    }

    else if (current_ID == 5){
      if(action_count == 1){
        client.flush();
        client.print("15CM");
      }
      target_RPM = 44;
      PIDcalculatev();
      traverse_v();
      action_count = 0;
      client.flush();
     client.print("Current Speed" + String(round((actual_RPM/60.0)*6.5*3.1415)) + " Target Speed: " + String(round((target_RPM/60.0)*6.5*3.1415)));
    }

    else if (current_ID == 6){
      if(action_count == 1){
        client.flush();
        client.print("20CM");
      }
      target_RPM = 59;
      PIDcalculatev();
      traverse_v();
      action_count = 0;
      client.flush();
      client.print("Current Speed" + String(round((actual_RPM/60.0)*6.5*3.1415)) + " Target Speed: " + String(round((target_RPM/60.0)*6.5*3.1415)));
    }

    else if (current_ID == 7){
      client.print("STOP");
      stopMotors();
      delay(5000);
      cameraloop();
      current_ID = 0;
      action_count = 0;
    }

    else if (current_ID == 8){
      if(action_count == 1){
        client.flush();
        client.print("10CM");
      }
      target_RPM = 29;
      PIDcalculatev();
      traverse_v();
      action_count = 0;
      client.flush();
      client.print("Current Speed" + String(round((actual_RPM/60.0)*6.5*3.1415)) + " Target Speed: " + String(round((target_RPM/60.0)*6.5*3.1415)));
    }

    else if (current_ID == 9){
      client.print("Spinning time!");
      analogWrite(CR, 100);
      digitalWrite(RF, HIGH);
      digitalWrite(RB, LOW);
      
      analogWrite(CL, 100);
      digitalWrite(LF, LOW);
      digitalWrite(LB, HIGH);
      delay(1500);
      current_ID = 0;
      action_count = 0;
    }

    if(action_count == 0){
      previous_ID = current_ID;
    }
  }
}


