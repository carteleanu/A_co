#include <TeensyDMX.h>
#include <vector>
using namespace qindesign::teensydmx;

constexpr int DMX_ENABLE_PIN = 2;
constexpr int UART_BAUD = 115200;

Sender dmx_sender{Serial1};

void setup() {
  pinMode(DMX_ENABLE_PIN, OUTPUT);
  digitalWrite(DMX_ENABLE_PIN, HIGH);
  dmx_sender.begin();
  Serial2.begin(UART_BAUD);
}

void loop() {
  if (Serial2.available()) {
    String command = Serial2.readStringUntil('\n');
    command.trim();
    handle_command(command);
  }
}

void handle_command(const String &cmd) {
  int atIndex = cmd.indexOf('@');
  
  if (atIndex <= 0) {
    Serial2.print("ERROR: Invalid format. Use 'channels@intensities' (e.g., 24,35,37@255,78,234)\n");
    return;
  }
  
  String channelsStr = cmd.substring(0, atIndex);
  String intensitiesStr = cmd.substring(atIndex + 1);
  
  std::vector<int> channels;
  std::vector<int> intensities;
  
  int startPos = 0;
  int commaPos = channelsStr.indexOf(',');
  while (commaPos > 0) {
    channels.push_back(channelsStr.substring(startPos, commaPos).toInt());
    startPos = commaPos + 1;
    commaPos = channelsStr.indexOf(',', startPos);
  }
  channels.push_back(channelsStr.substring(startPos).toInt());
  
  startPos = 0;
  commaPos = intensitiesStr.indexOf(',');
  while (commaPos > 0) {
    intensities.push_back(intensitiesStr.substring(startPos, commaPos).toInt());
    startPos = commaPos + 1;
    commaPos = intensitiesStr.indexOf(',', startPos);
  }
  intensities.push_back(intensitiesStr.substring(startPos).toInt());
  
  if (channels.size() != intensities.size()) {
    Serial2.print("ERROR: Channel and intensity count mismatch. Format: channels@intensities (e.g., 24,35@255,78)\n");
    return;
  }
  
  if (channels.size() == 0) {
    Serial2.print("ERROR: No channels provided. Format: channels@intensities (e.g., 24,35@255,78)\n");
    return;
  }
  
  for (size_t i = 0; i < channels.size(); i++) {
    int channel = constrain(channels[i], 1, 512);
    int intensity = constrain(intensities[i], 0, 255);
    dmx_sender.set(channel, intensity);
  }
  
  Serial2.print("OK: Set ");
  Serial2.print(channels.size());
  Serial2.print(" channels\n");
}