import controlP5.*;
import processing.net.*;
import processing.sound.*;

// Image declarations
PImage Tencms_Limit;
PImage Fifteencms_Limit;
PImage Twentycms_Limit;
PImage FullSpeed;
PImage Stop;
PImage Straight;
PImage Left;
PImage Right;
PImage img;

// Current sign variables
PImage currentDirectionSign = null;
PImage currentSpeedSign = null;

// Sound declarations
SoundFile buttonSound;
SoundFile goSound;
SoundFile stopSound;
SoundFile pidSound;
SoundFile trackingSound;
SoundFile leftSound;
SoundFile rightSound;
SoundFile straightSound;
SoundFile speed10Sound;
SoundFile speed15Sound;
SoundFile speed20Sound;
SoundFile fullSpeedSound;

ControlP5 cp5;
int timer = 10;
String message = "";
String ip = "192.168.4.1"; // Replace with your Arduino's IP
Client myClient; // Client object to send data to the Arduino
Knob ControlKnob;
Textarea dialogueTextarea;

// Variables for arrow flashing
boolean leftArrowActive = false;
boolean rightArrowActive = false;
int flashInterval = 500; // milliseconds between flashes
int lastFlashTime = 0;
boolean flashState = false;

void setup() {
  // Load images
  img = loadImage("Trinity_College_Dublin_Arms.svg.png");
  Tencms_Limit = loadImage("10cms Limit.png");
  Fifteencms_Limit = loadImage("15cms limit.png");
  Twentycms_Limit = loadImage("20cms limit.png");
  FullSpeed = loadImage("Full Speed.png");
  Stop = loadImage("Stop.png");
  Straight = loadImage("Straight On.png"); 
  Left = loadImage("Turn Left.png");
  Right = loadImage("Turn Right.png");
  
  // Load sound files - replace these with your actual sound files
  buttonSound = new SoundFile(this, "Button.mp3");
  goSound = new SoundFile(this, "Go.mp3");
  stopSound = new SoundFile(this, "Stop.mp3");
  leftSound = new SoundFile(this, "Turn Left.mp3");
  rightSound = new SoundFile(this, "Turn Right.mp3");
  straightSound = new SoundFile(this, "Forward.mp3");
  speed10Sound = new SoundFile(this, "TenCms.mp3");
  speed15Sound = new SoundFile(this, "FifteenCms.mp3");
  speed20Sound = new SoundFile(this, "TwentyCms.mp3");
  fullSpeedSound = new SoundFile(this, "Full Speed.mp3");
  
  size(1200, 700);

  cp5 = new ControlP5(this);

  // Initialize the client to connect to the Arduino's IP address and port
  myClient = new Client(this, ip, 5200); // Replace with your Arduino's IP and port

  // Add buttons for movement
  cp5.addButton("Go")
     .setPosition(150, 200)
     .setSize(100, 100)
     .setLabel("Go With Camera!")
     .setColorBackground(color(0, 0, 255))  // Blue background
     .setColorForeground(color(0, 255, 0))  // Green foreground (when hovered)
     .setColorActive(color(0, 255, 0))      // Green active (when pressed)
     .setColorLabel(color(255, 255, 255));  // White text

  cp5.addButton("Stop")
     .setPosition(150, 350)
     .setSize(100, 100)
     .setLabel("Stop!")
     .setColorBackground(color(0, 0, 255))  // Blue background
     .setColorForeground(color(255, 0, 0))  // Red foreground (when hovered)
     .setColorActive(color(255, 0, 0))      // Red active (when pressed)
     .setColorLabel(color(255, 255, 255));  // White text
    
  cp5.addButton("Go_With_PID")
     .setPosition(300, 200)
     .setSize(100, 100)
     .setLabel("Go With PID!")
     .setColorBackground(color(0, 0, 255))  // Blue background
     .setColorForeground(color(0, 255, 0))  // Green foreground (when hovered)
     .setColorActive(color(255, 0, 0))      // Red active (when pressed)
     .setColorLabel(color(255, 255, 255));  // White text
 
   cp5.addButton("Simple_Tracking")
     .setPosition(300, 350)
     .setSize(100, 100)
     .setLabel("Simple Tracking")
     .setColorBackground(color(0, 0, 255))  // Blue background
     .setColorForeground(color(0, 255, 0))  // Green foreground (when hovered)
     .setColorActive(color(255, 0, 0))      // Red active (when pressed)
     .setColorLabel(color(255, 255, 255));  // White text
   
   ControlKnob = cp5.addKnob("Speed")
               .setRange(0,120)
               .setValue(60)
               .setColorForeground(color(200,0,0))
               .setColorBackground(color(0,0,255))
               .setColorActive(color(0,225,0))
               .setPosition(950, 337.5)
               .setRadius(62.5)
               .setDragDirection(Knob.HORIZONTAL);
               
   // Add a text area for the odometer
   dialogueTextarea = cp5.addTextarea("dialogue")
                   .setPosition(352.5, 500)
                   .setSize(500, 150)
                   .setFont(createFont("Arial", 12))
                   .setColorBackground(color(0, 0, 0, 50))
                   .setColorForeground(color(255, 255, 255, 200))
                   .setColorActive(color(255, 255, 255, 255))
                   .setLabel("Dialogue")
                   .setText("Dialgue will appear here \n")
                   .setScrollBackground(color(0, 0, 0, 100))
                   .setScrollForeground(color(255, 255, 255, 200));
}

void draw() {
  background(187, 165, 61);
  
  textSize(30);
  text("Group Z9, Gold Challenge", 120, 40); 
  fill(255, 60, 255);
  
  // Display Trinity College logo
  image(img, 512.5, 25, width / 6.5, height / 5);
 
  // Draw white squares for signs
  fill(255);
  square(540, 200, 125); // For Direction
  square(950, 200, 125); // For Speed Limit
  
  // Display current signs if they exist
  if (currentDirectionSign != null) {
    image(currentDirectionSign, 540, 200, 125, 125);
  }
  
  if (currentSpeedSign != null) {
    image(currentSpeedSign, 950, 200, 125, 125);
  }
  
  // Check for incoming messages from Arduino
  checkClientMessages();
  
  // Update flash state if needed
  updateFlashState();
  
  // Draw arrow indicators based on current state
  if (leftArrowActive && flashState) {
    drawActiveArrow(50, 50, true);  // Left arrow (flashing on)
  } else {
    drawInactiveArrow(50, 50, true);  // Left arrow (flashing off or inactive)
  }
  
  if (rightArrowActive && flashState) {
    drawActiveArrow(width - 100, 50, false); // Right arrow (flashing on)
  } else {
    drawInactiveArrow(width - 100, 50, false); // Right arrow (flashing off or inactive)
  }
}

void drawActiveArrow(float x, float y, boolean left) {
  fill(0, 255, 0); // Green for active
  stroke(0);
  strokeWeight(2);
  if (left) {
    triangle(x + 20, y, x + 50, y + 20, x + 50, y - 20);
  } else {
    triangle(x + 50, y, x + 20, y + 20, x + 20, y - 20);
  }
}

void drawInactiveArrow(float x, float y, boolean left) {
  fill(255, 0, 0); // Red for inactive
  stroke(0);
  strokeWeight(2);
  if (left) {
    triangle(x + 20, y, x + 50, y + 20, x + 50, y - 20);
  } else {
    triangle(x + 50, y, x + 20, y + 20, x + 20, y - 20);
  }
}

void updateFlashState() {
  // Only flash if at least one arrow is active
  if (leftArrowActive || rightArrowActive) {
    int currentTime = millis();
    if (currentTime - lastFlashTime > flashInterval) {
      flashState = !flashState;
      lastFlashTime = currentTime;
    }
  } else {
    flashState = false; // Reset flash state when no arrows are active
  }
}

void checkClientMessages() {
  // Check for incoming messages from the Arduino
  if (myClient.available() > 0) {
    String incomingMessage = myClient.readStringUntil('\n');
    if (incomingMessage != null) {
      incomingMessage = incomingMessage.trim();
      dialogueTextarea.setText("Received: " + incomingMessage + "\n" + dialogueTextarea.getText());
      
      // Parse the message to control the arrows and signs
      if (incomingMessage.equals("LEFT")) {
        leftArrowActive = true;
        rightArrowActive = false;
        currentDirectionSign = Left;
        leftSound.play(); // Play turning left sound
      } else if (incomingMessage.equals("RIGHT")) {
        rightArrowActive = true;
        leftArrowActive = false;
        currentDirectionSign = Right;
        rightSound.play(); // Play turning right sound
      } else if (incomingMessage.equals("STRAIGHT")) {
        leftArrowActive = false;
        rightArrowActive = false;
        currentDirectionSign = Straight;
        straightSound.play(); // Play straight on sound
      } else if (incomingMessage.equals("STOP")) {
        currentDirectionSign = Stop;
        stopSound.play(); // Play stop sound
      } else if (incomingMessage.equals("10CM")) {
        currentSpeedSign = Tencms_Limit;
        speed10Sound.play(); // Play speed limit 10 sound
      } else if (incomingMessage.equals("15CM")) {
        currentSpeedSign = Fifteencms_Limit;
        speed15Sound.play(); // Play speed limit 15 sound
      } else if (incomingMessage.equals("20CM")) {
        currentSpeedSign = Twentycms_Limit;
        speed20Sound.play(); // Play speed limit 20 sound
      } else if (incomingMessage.equals("FULLSPEED")) {
        currentSpeedSign = FullSpeed;
        fullSpeedSound.play(); // Play full speed sound
      }
    }
  }
}

// Button control functions
void Go() {
  buttonSound.play(); // Play button press sound
  message = "C"; // Message to send to Arduino
  myClient.write(message);
  println(message);
  dialogueTextarea.append("Sent: Go With Camera\n");
  goSound.play(); // Play go with camera sound
}

void Stop() {
  buttonSound.play(); // Play button press sound
  message = "S"; // Message to send to Arduino
  myClient.write(message);
  println(message);
  dialogueTextarea.append("Sent: Halt\n");
  stopSound.play(); // Play stop sound
}

void Go_With_PID() {
  buttonSound.play(); // Play button press sound
  message = "P"; // Message to send to Arduino
  myClient.write(message);
  println(message);
  dialogueTextarea.append("Sent: Go with PID\n");
  goSound.play(); // Play go with PID sound
}

void Simple_Tracking() {
  buttonSound.play(); // Play button press sound
  message = "T"; // Message to send to Arduino
  myClient.write(message);
  println(message);
  dialogueTextarea.append("Simple Tracking Underway \n");
  goSound.play(); // Play simple tracking sound
}

void Speed(int value) {
 
  // Create a char array to hold the formatted string (e.g., "V009" or "V024")
  char[] buffer = new char[5]; // 5 characters: 'V', 3 digits, and a null terminator

  // Use String.format to format the value with leading zeros
  String formattedValue = String.format("V%03d", value); // %03d ensures 3 digits with leading zeros

  // Convert the formatted string to a char array
  formattedValue.getChars(0, formattedValue.length(), buffer, 0);

  // Null-terminate the string (not strictly necessary in Java/Processing but good practice)
  buffer[4] = '\0';

  // Send the message
  myClient.write(new String(buffer)); // Convert char array back to String for sending

  // Print the message to the console
  println(new String(buffer));

  // Clear the client (if needed)
  myClient.clear();

  // Log the sent message
  dialogueTextarea.append("Sent: " + new String(buffer) + "\n");
  
  // Play appropriate speed sound based on value
  if (value <= 40) {
    speed10Sound.play();
  } else if (value <= 80) {
    speed15Sound.play();
  } else if (value < 120) {
    speed20Sound.play();
  } else {
    fullSpeedSound.play();
  }
}
