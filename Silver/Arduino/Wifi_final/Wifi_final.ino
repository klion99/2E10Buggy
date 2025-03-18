#include <WiFiS3.h>
#include <wifidetails.ino>
char ssid[] = ssid;
char pass[] = pass;
void setup() {
 Serial.begin(9600);
 WiFi.begin(ssid, pass);
 IPAddress ip = WiFi.localIP();
 Serial.print("IP Address:");
 
 Serial.println(ip);
}
void loop() {
}
