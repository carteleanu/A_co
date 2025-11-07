#include <TeensyDMX.h>
using namespace qindesign::teensydmx;

// --- DMX Configuration (Using Serial1) ---
constexpr int DMX_ENABLE_PIN = 2;
// DMX Sender object uses Serial1 (TX1 is Pin 1)
Sender dmx_sender{Serial1};

constexpr int CH_RED = 1;
constexpr int CH_GREEN = 2;
constexpr int CH_BLUE = 3;

// --- Timing for the Flash-On/Flash-Off Feature ---
// Set the duration the lights should stay ON (3000ms = 3 seconds)
constexpr unsigned long LIGHT_ON_TIME_MS = 3000; 
// Global variable to store the time when the lights should turn OFF
unsigned long turn_off_time = 0; 

// --- UART/Serial Configuration ---
constexpr int UART_BAUD = 115200;

void setup() {
  pinMode(DMX_ENABLE_PIN, OUTPUT);
  digitalWrite(DMX_ENABLE_PIN, HIGH);  // enable RS-485

  // Initialize DMX output on Serial1
  dmx_sender.begin();
  
  // Debug output to PC (Serial)
  Serial.begin(9600);
  
  // Initialize Teensy UART to Pico on Serial2 (RX2 is Pin 7)
  Serial2.begin(UART_BAUD);

  Serial.println("DMX sender ready. Waiting for commands on Serial2...");
}

void loop() {
  // 1. CHECK FOR INCOMING COMMANDS FROM PICO on Serial2
  if (Serial2.available()) {
    String command = Serial2.readStringUntil('\n');  // read until newline
    command.trim();  // remove any extra whitespace/newline

    Serial.print("Received command: ");
    Serial.println(command);

    // Call the handler to set the color/level based on the command
    handle_command(command);
    
    // ACTION: Set the turn_off_time for 3 seconds from now
    turn_off_time = millis() + LIGHT_ON_TIME_MS; 
    
    Serial.println("Lights ON. Scheduled to turn off soon.");
  }

  // 2. CHECK IF IT'S TIME TO TURN OFF THE LIGHTS
  // Checks if the timer is active AND the current time has passed the turn_off_time
  if (turn_off_time != 0 && millis() >= turn_off_time) {
    // Turn all DMX channels completely OFF (value 0)
    dmx_sender.set(CH_RED, 0);
    dmx_sender.set(CH_GREEN, 0);
    dmx_sender.set(CH_BLUE, 0);
    
    // Reset the timer so it doesn't trigger again until a new message arrives
    turn_off_time = 0; 
    Serial.println("Lights OFF (Timer expired).");
  }
}
void set_dmx_color(int r, int g, int b);

// Map commands to DMX channel output
void handle_command(const String &cmd) {

  // --- NEW FORMAT SUPPORT: "channel value"
  int spaceIndex = cmd.indexOf(' ');
  if (spaceIndex > 0) {
    int channel = cmd.substring(0, spaceIndex).toInt();
    int value = cmd.substring(spaceIndex + 1).toInt();

    channel = constrain(channel, 1, 12);  // ✅ your LTECH has 12 channels
    value = constrain(value, 0, 255);

    dmx_sender.set(channel, value);

    Serial.print("DMX → Channel ");
    Serial.print(channel);
    Serial.print(" = ");
    Serial.println(value);

    return; // ✅ Done for numeric DMX control
  }

  // --- EXISTING COMMANDS STILL WORK ---

  if (cmd.startsWith("Dimming_Level_")) {
    int level = cmd.substring(14).toInt();
    level = constrain(level, 0, 255);

    // This applies level to first RGB fixture (channels 1-3)
    dmx_sender.set(CH_RED, level);
    dmx_sender.set(CH_GREEN, level);
    dmx_sender.set(CH_BLUE, level);

    Serial.print("Set all RGB to level: ");
    Serial.println(level);
  }
  else if (cmd == "Red") {
    set_dmx_color(255, 0, 0);
  }
  else if (cmd == "Green") {
    set_dmx_color(0, 255, 0);
  }
  else if (cmd == "Blue") {
    set_dmx_color(0, 0, 255);
  }
  else {
    Serial.print("Unknown command: ");
    Serial.println(cmd);
  }
}

// Set RGB color immediately on channels 1–3
void set_dmx_color(int r, int g, int b) {
  dmx_sender.set(CH_RED, r);
  dmx_sender.set(CH_GREEN, g);
  dmx_sender.set(CH_BLUE, b);

  Serial.print("RGB Set → R:");
  Serial.print(r);
  Serial.print(" G:");
  Serial.print(g);
  Serial.print(" B:");
  Serial.println(b);
}

