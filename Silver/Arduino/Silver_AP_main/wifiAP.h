//the following code shall only set up the wifi connection and run the relavent code for recieving commands from the client.

#pragma once
#include <WiFiS3.h>
#include "functions.h"
#include <cstring>
#include <Arduino.h>


char command;
char recieve;
int sent_speed = 100;
char s[3] = {'0','0','0'};

bool clientConnected = false; //bool to detect if the client was connected previously

//setting up the name of the SSID and password of the network that the arduino will host.
char ssid[] = "Try";
char pass[] = "abcd12345";



//setting up the port on the arduino that will be used to host the server
WiFiServer server(5200);

//initalizing the client 
WiFiClient client; 

//returns the command variable to the main code
char getcommand(){
  return command; 
}

int getSentSpeed(){
  return sent_speed; 
}


void Wifisetup() {
  //copying in the corrected name and pass for the SSID and pass of the wifi network

  WiFi.beginAP(ssid,pass);
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address:");
  //prints the ip address 
  Serial.println(ip);
  server.begin();

  //printing the wifi strenght when connected. 
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

}



void Wifiloop() {
  client = server.available();   // listen for incoming clients
  if (client) {                             
    //Serial.println("new client");                            
    while (client.connected()) { 
      //checking to see if the client is transmitting any data         
      if (client.available() > 0) {
        //reading the data
        recieve = client.read();
        if(recieve == 'V'){
          for (int i = 0; i < 3; i++){
            s[i] = client.read();
          }
          sent_speed = atoi(s);
          client.flush();
          recieve = '\n';
        }

        //updating the command variable for start and stop, ignoring any other data being sent.     
        if(recieve == 'g' || recieve == 's' || recieve == 't' || recieve == 'u'){
          command = recieve;
          break;
        }
      }
    }
  }
}