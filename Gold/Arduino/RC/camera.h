#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

// Create HuskyLens object
HUSKYLENS huskylens;

//initializing Huskylens variables. 
int ID;
int current_time_c = 500;
int update_c = 500;
float size_irl = 0;

//getter function for processing ID to RC.ino
int getID(){
  return ID;
}

// I2C communication setup
void camerasetup() {
    Serial.begin(115200);
    Wire.begin();
    //debugging settup.
    if (!huskylens.begin(Wire)) {
        Serial.println("HuskyLens not detected! Check wiring.");
        while (1);
    }

    Serial.println("HuskyLens connected successfully!");
    huskylens.writeAlgorithm(ALGORITHM_TAG_RECOGNITION); // Set to Tag recognition mode
}

//continously reading the Huskylens, updated every 500 ms. 
void cameraloop() {
  if(millis() > current_time_c){
    if (huskylens.request()) {
        if (huskylens.count()) {
            for (int i = 0; i < huskylens.count(); i++) {
                HUSKYLENSResult result = huskylens.get(i);
                size_irl = result.width * result.height;
                Serial.println(size_irl);
              //update the ID only if the size is greater than 4000, ensures tags faraway are not read preemtively. 
              if(size_irl > 4000){
                ID = result.ID;
              }
            }
        } else{
            //default ID when no ID is detected
            ID = 0;
        }
    }
    //updating time to ensure tag is read only every 500 ms. 
    current_time_c = current_time_c + update_c;
  }
}