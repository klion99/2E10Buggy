#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

// Create HuskyLens object
HUSKYLENS huskylens;

void PrintOnce(String Print){
  if(){
    
  }
}


// I2C communication setup
void setup() {
    Serial.begin(115200);
    Wire.begin();
    
    if (!huskylens.begin(Wire)) {
        Serial.println("HuskyLens not detected! Check wiring.");
        while (1);
    }

    Serial.println("HuskyLens connected successfully!");
    huskylens.writeAlgorithm(ALGORITHM_OBJECT_TRACKING); // Set to Object Recognition mode
}

String Temp = " ";

void loop() {
    if (huskylens.request()) {
        if (huskylens.count()) {
            Serial.println("Object detected!");
            for (int i = 0; i < huskylens.count(); i++) {
                HUSKYLENSResult result = huskylens.get(i);
                Serial.print("Object ID: "); Serial.println(result.ID);
                Serial.print("X: "); Serial.print(result.xCenter);
                Serial.print(", Y: "); Serial.print(result.yCenter);
                Serial.print(", Width: "); Serial.print(result.width);
                Serial.print(", Height: "); Serial.println(result.height);
            }
        } else if(){
            
            Serial.println(NoObject);
        }
    }
    delay(500);
}