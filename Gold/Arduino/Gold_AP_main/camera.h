#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

// Create HuskyLens object
HUSKYLENS huskylens;

/* void PrintOnce(String Print){
  if(){
    
  }
} */

int ID;
int current_time_c = 500;
int update_c = 500;
float size_irl = 0;

int getID(){
  return ID;
}

// I2C communication setup
void camerasetup() {
    Serial.begin(115200);
    Wire.begin();
    
    if (!huskylens.begin(Wire)) {
        Serial.println("HuskyLens not detected! Check wiring.");
        while (1);
    }

    Serial.println("HuskyLens connected successfully!");
    huskylens.writeAlgorithm(ALGORITHM_TAG_RECOGNITION); // Set to Object Recognition mode
}

String Temp = " ";

void cameraloop() {
  if(millis() > current_time_c){
    if (huskylens.request()) {
        if (huskylens.count()) {
            //Serial.println("Object detected!");
            for (int i = 0; i < huskylens.count(); i++) {
                HUSKYLENSResult result = huskylens.get(i);
/*              Serial.print("Object ID: "); Serial.println(result.ID);
                Serial.print("X: "); Serial.print(result.xCenter);
                Serial.print(", Y: "); Serial.print(result.yCenter);*/
                size_irl = result.width * result.height;
                Serial.println(size_irl);
              if(size_irl > 3000){
                ID = result.ID;
              }
            }
        } else{
            //Serial.println("NoObject");
            ID = 0;
        }
    }
    current_time_c = current_time_c + update_c;
  }
}