import controlP5.*;
import processing.net.*;

PImage img;
ControlP5 cp5;
Textarea logTextarea;
Textarea odometerTextarea;
int timer = 10;
String message = "";
String ip = "192.168.4.1";
//String ip = "192.168.0.183";
Client myClient; // Create a client object to send data to the Arduino

void setup() {
  size(800, 600);

  img = loadImage("Trinity_College_Dublin_Arms.svg.png");
  cp5 = new ControlP5(this);

  // Initialize the client to connect to the Arduino's IP address and port
  myClient = new Client(this,ip, 5200); // Replace with your Arduino's IP and port

  // Add buttons for movement
  cp5.addButton("Go")
     .setPosition(350, 200)
     .setSize(100, 50)
     .setLabel("Go!")
      .setColorBackground(color(0, 0, 255))  // Blue background
     .setColorForeground(color(0, 255, 0))  // Green foreground (when hovered)
     .setColorActive(color(0, 255, 0))      // Blue active (when pressed)
     .setColorLabel(color(255, 255, 255));  // White text */

  cp5.addButton("Stop")
     .setPosition(350, 300)
     .setSize(100, 50)
     .setLabel("Stop!")
     .setColorBackground(color(0, 0, 255))  // Blue background
     .setColorForeground(color(255, 0, 0))  // Red foreground (when hovered)
     .setColorActive(color(255, 0, 0))      // Red active (when pressed)
     .setColorLabel(color(255, 255, 255));  // White text */

  // Add a slider for speed control
  cp5.addSlider("Speed")
     .setPosition(250, 400)
     .setSize(300, 30)
     .setRange(0, 255)
     .setValue(128)
     .setLabel("Speed");

  // Add a text area for the odometer
  odometerTextarea = cp5.addTextarea("Odometer")
                   .setPosition(50, 250)
                   .setSize(200, 100)
                   .setFont(createFont("Arial", 12))
                   .setColorBackground(color(0, 0, 0, 50))
                   .setColorForeground(color(255, 255, 255, 200))
                   .setColorActive(color(255, 255, 255, 255))
                   .setLabel("Odometer")
                   .setText("Distance Travelled: \n")
                   .setScrollBackground(color(0, 0, 0, 100))
                   .setScrollForeground(color(255, 255, 255, 200));

  // Add a text area for logging
  logTextarea = cp5.addTextarea("Log")
                   .setPosition(230, 470)
                   .setSize(360, 100)
                   .setFont(createFont("Arial", 12))
                   .setColorBackground(color(0, 0, 0, 50))
                   .setColorForeground(color(255, 255, 255, 200))
                   .setColorActive(color(255, 255, 255, 255))
                   .setLabel("Log Output")
                   .setText("Log will appear here... \n")
                   .setScrollBackground(color(0, 0, 0, 100))
                   .setScrollForeground(color(255, 255, 255, 200));
}

void draw() {
  background(255, 145, 0);
  if(millis()>timer){
    myClient.write(message);
  }
  image(img, 320, 25, width / 5, height / 4.25);

  // Check for incoming data from the Arduino
  if (myClient.available() > 0) {
    String data = myClient.readString(); // Read the data until newline
     println(data);
    if(data.equals("O")){
      myClient.clear();
      delay(200);
      logTextarea.append("Recived from Junior: Object detected\n");
    }
    else if(data.equals("L")){
      myClient.clear();
      delay(200);
      logTextarea.append("Recived from Junior: Turning Left\n");
    }
    else if(data.equals("R")){
      myClient.clear();
      delay(200);
      logTextarea.append("Recived from Junior: Turning Right\n");
    }
    else if ((data != null)) {
      println(data);
      data = data.trim(); // Remove any leading/trailing whitespace
      odometerTextarea.setText("Distance Travelled: " + data + "\n"); // Update the odometer text area
    }
  }
  myClient.clear();
}

// Button event handlers
void Go() {
  message = "g"; // Message to send to Arduino
  myClient.write(message); // Send the message
  println(message);
  logTextarea.append("Sent: Go\n");
}

void Stop() {
  message = "s"; // Message to send to Arduino
  myClient.write(message); // Send the message
  println(message);
  logTextarea.append("Sent: Halt\n");
}

/* Slider event handler
void Speed(int value) {
  String message = "V" + value; // Message to send to Arduino (e.g., "V128")
  myClient.write(message); // Send the message
  println("Sent: " + message);
  logTextarea.append("Sent: " + message + "\n");
}  */
