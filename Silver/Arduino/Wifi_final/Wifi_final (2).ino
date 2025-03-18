#include <WiFiS3.h>
#include <txt_wifi>
char ssid[] = “my_wifi_network”;
char pass[] = “Password123";
void setup() {
 Serial.begin(9600);
 WiFi.begin(ssid, pass);
 IPAddress ip = WiFi.localIP();
 Serial.print("IP Address:");
 Serial.println(ip);
}
void loop() {
}
