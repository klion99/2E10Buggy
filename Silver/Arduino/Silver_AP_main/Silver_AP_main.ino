#include "functions.h"
#include "wifiAP.h"
#include "PID.h"
#include "PIDv.h"


//message string for sending mean distance to client 
String MD = "";
int timer;
char cmd; 


void setup() {
  Serial.begin(115200);
 
  //setup of all sensor pins and motor pins and their types
  SetPins();

  //showing group number on LED display on bootup
  ShowGroupNo();

  //setting up arduino as server via access point
  Wifisetup(); 


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
    client.print("Current RPM" + String((actual_RPM)*6.5*3.1415/60) + " Target RPM: " + String((target_RPM)*6.5*3.1415/60));
    traverse_v();
  }
}


