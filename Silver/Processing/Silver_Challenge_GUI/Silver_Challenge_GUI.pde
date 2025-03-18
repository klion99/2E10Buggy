import controlP5.*;
import processing.net.*;

PImage img;
PImage img2;
ControlP5 cp5;
Textarea logTextarea;
Textarea odometerTextarea;
Textarea trackTextarea;
int timer = 100;
String message = "";
String ip = "192.168.4.1"; // Replace with your Arduino's IP
Client myClient; // Client object to send data to the Arduino

void setup() {
  img = loadImage("Trinity_College_Dublin_Arms.svg.png");
  img2 = loadImage("Linda_Doyle.png");
  size(1200, 600);

  cp5 = new ControlP5(this);

  // Initialize the client to connect to the Arduino's IP address and port
  myClient = new Client(this, ip, 5200); // Replace with your Arduino's IP and port

  // Add buttons for movement
  cp5.addButton("Go")
     .setPosition(350, 200)
     .setSize(100, 50)
     .setLabel("Go!")
     .setColorBackground(color(0, 0, 255))  // Blue background
     .setColorForeground(color(0, 255, 0))  // Green foreground (when hovered)
     .setColorActive(color(0, 255, 0))      // Green active (when pressed)
     .setColorLabel(color(255, 255, 255));  // White text

  cp5.addButton("Stop")
     .setPosition(350, 300)
     .setSize(100, 50)
     .setLabel("Stop!")
     .setColorBackground(color(0, 0, 255))  // Blue background
     .setColorForeground(color(255, 0, 0))  // Red foreground (when hovered)
     .setColorActive(color(255, 0, 0))      // Red active (when pressed)
     .setColorLabel(color(255, 255, 255));  // White text

  cp5.addButton("Start_Tracking") // Use underscores for method names
     .setPosition(500, 200)
     .setSize(100, 50)
     .setLabel("Start Tracking")
     .setColorBackground(color(0, 0, 255))  // Blue background
     .setColorForeground(color(255, 0, 0))  // Red foreground (when hovered)
     .setColorActive(color(255, 0, 0))      // Red active (when pressed)
     .setColorLabel(color(255, 255, 255));  // White text

  cp5.addButton("Stop_Tracking") // Use underscores for method names
     .setPosition(500, 300)
     .setSize(100, 50)
     .setLabel("Velocity Tracking")
     .setColorBackground(color(0, 0, 255))  // Blue background
     .setColorForeground(color(255, 0, 0))  // Red foreground (when hovered)
     .setColorActive(color(255, 0, 0))      // Red active (when pressed)
     .setColorLabel(color(255, 255, 255));  // White text

  // Add a slider for speed control
  cp5.addSlider("Speed")
     .setPosition(350, 400)
     .setSize(250, 30)
     .setRange(0, 46)
     .setValue(100)
     .setLabel("Speed");

  // Add a text area for the odometer
  odometerTextarea = cp5.addTextarea("Odometer")
                   .setPosition(700, 50)
                   .setSize(360, 100)
                   .setFont(createFont("Arial", 12))
                   .setColorBackground(color(0, 0, 0, 50))
                   .setColorForeground(color(255, 255, 255, 200))
                   .setColorActive(color(255, 255, 255, 255))
                   .setLabel("Odometer")
                   .setText("Telemetry: \n")
                   .setScrollBackground(color(0, 0, 0, 100))
                   .setScrollForeground(color(255, 255, 255, 200));

  // Add a text area for logging
  logTextarea = cp5.addTextarea("Log")
                   .setPosition(700, 200)
                   .setSize(360, 100)
                   .setFont(createFont("Arial", 12))
                   .setColorBackground(color(0, 0, 0, 50))
                   .setColorForeground(color(255, 255, 255, 200))
                   .setColorActive(color(255, 255, 255, 255))
                   .setLabel("Log Output")
                   .setText("Log will appear here... \n")
                   .setScrollBackground(color(0, 0, 0, 100))
                   .setScrollForeground(color(255, 255, 255, 200));

  // Add a text area for tracking logs
  trackTextarea = cp5.addTextarea("Tracking Monitor")
                   .setPosition(700, 350)
                   .setSize(360, 100)
                   .setFont(createFont("Arial", 12))
                   .setColorBackground(color(0, 0, 0, 50))
                   .setColorForeground(color(255, 255, 255, 200))
                   .setColorActive(color(255, 255, 255, 255))
                   .setLabel("Tracking Output")
                   .setText("Tracking Log will appear here... \n")
                   .setScrollBackground(color(0, 0, 0, 100))
                   .setScrollForeground(color(255, 255, 255, 200));
}

void draw() {
  background(192, 192, 192);
  
  image(img, 362.5, 25, width / 5.5, height / 4.5);
  image(img2, 25, 105, width / 5.5, height / 4.5);
  if(millis()>timer){
    myClient.write(message);
    timer = timer+100;
  }

  // Check for incoming data from the Arduino
  if (myClient.available() > 0) {
    String data = myClient.readString(); // Read the data until newline
    if (data != null) {
      data = data.trim(); // Remove any leading/trailing whitespace
      println(data); // Print the data to the console for debugging

      if (data.equals("O")) {
        logTextarea.append("Object Detected\n"); // Update the log text area
      } else {
        odometerTextarea.setText("Telemetry: " + data + "\n"); // Update the odometer text area
      }
      myClient.clear();  // Discards all incoming data
    }
  }
  
  // Printing Distance of Tracked Object
  if (myClient.available() > 0) {
    String data = myClient.readString(); // Read the data until newline
    if (data != null) {
      trackTextarea.append(data + "\n");
      println(data); // Print the data to the console for debugging
    }
    myClient.clear();  // Discards all incoming data
  }
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

void Start_Tracking() { // Corrected method name
  message = "t"; // Message to send to Arduino
  myClient.write(message); // Send the message
  println(message);
  trackTextarea.append("Sent: Tracking Started\n");
}

void Stop_Tracking() { // Corrected method name
  message = "u"; // Message to send to Arduino
  myClient.write(message); // Send the message
  println(message);
  trackTextarea.append("Sent: Velovcity Tracking Started \n");
}

void Speed(int value) {
  // Create a char array to hold the formatted string (e.g., "V009" or "V024")
  char[] buffer = new char[5]; // 5 characters: 'V', 3 digits, and a null terminator

  // Use String.format to format the value with leading zeros
  int velocity = value;
  value = round(value*60/(6.5*3.1415));
  String formattedValue = String.format("V%03d", value); // %03d ensures 3 digits with leading zeros

  // Convert the formatted string to a char array
  formattedValue.getChars(0, formattedValue.length(), buffer, 0);

  // Null-terminate the string (not strictly necessary in Java/Processing but good practice)
  buffer[4] = '\0';

  // Send the message
  myClient.clear();
  delay(100);
  String a = new String(buffer);
     myClient.write(a); // Convert char array back to String for sending
  delay(100);
 
  

  // Print the message to the console
  println(new String(buffer));

  // Clear the client (if needed)
  //myClient.write(message);
  myClient.clear();

  // Log the sent message
  logTextarea.append("Sent: " + String.format("%02d", velocity) + " cm/s" + "\n");
  
}
