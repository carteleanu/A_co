#include <TeensyDMX.h>
#include <vector>
using namespace qindesign::teensydmx;

/* ---------- Configuration ---------- */
constexpr int DMX_ENABLE_PIN = 2;
constexpr int UART_BAUD = 115200;

/* ---------- DMX sender ---------- */
Sender dmx_sender{Serial1};

/*
  Software mirror of DMX output values.
  Index 1..512 correspond directly to DMX channels.
  Index 0 is intentionally unused.
*/
uint8_t dmx_values[513] = {0};

/* ---------- Fade state ---------- */
struct Fade {
  int channel;
  int start_value;
  int target_value;
  unsigned long start_time;
  unsigned long duration_ms;
};

std::vector<Fade> activeFades;

/* ---------- Setup ---------- */
void setup() {
  pinMode(DMX_ENABLE_PIN, OUTPUT);
  digitalWrite(DMX_ENABLE_PIN, HIGH);   // Enable RS-485

  dmx_sender.begin();
  Serial2.begin(UART_BAUD);
}

/* ---------- Main loop ---------- */
void loop() {
  if (Serial2.available()) {
    String command = Serial2.readStringUntil('\n');
    command.trim();
    handle_command(command);
  }

  update_fades();
}

/* ---------- Fade engine ---------- */
void update_fades() {
  unsigned long now = millis();

  for (size_t i = 0; i < activeFades.size(); ) {
    Fade &f = activeFades[i];
    unsigned long elapsed = now - f.start_time;

    if (elapsed >= f.duration_ms) {
      // Fade complete
      dmx_sender.set(f.channel, f.target_value);
      dmx_values[f.channel] = f.target_value;
      activeFades.erase(activeFades.begin() + i);
    } else {
      // Fade in progress
      float t = (float)elapsed / (float)f.duration_ms;
      int value = f.start_value + (f.target_value - f.start_value) * t;
      value = constrain(value, 0, 255);

      dmx_sender.set(f.channel, value);
      dmx_values[f.channel] = value;
      i++;
    }
  }
}

/* ---------- Command handler ---------- */
void handle_command(const String &cmd) {
  int firstAt  = cmd.indexOf('@');
  int secondAt = cmd.indexOf('@', firstAt + 1);

  if (firstAt <= 0 || secondAt <= firstAt) {
    Serial2.println("ERROR: Format channels@intensities@time");
    return;
  }

  String channelsStr    = cmd.substring(0, firstAt);
  String intensitiesStr = cmd.substring(firstAt + 1, secondAt);
  String timeStr        = cmd.substring(secondAt + 1);

  std::vector<int> channels;
  std::vector<int> intensities;

  parse_list(channelsStr, channels);
  parse_list(intensitiesStr, intensities);

  if (channels.size() == 0 || channels.size() != intensities.size()) {
    Serial2.println("ERROR: Channel/intensity count mismatch");
    return;
  }

  int fadeSeconds = timeStr.toInt();
  if (fadeSeconds < 0) fadeSeconds = 0;

  unsigned long now = millis();
  unsigned long duration_ms = (unsigned long)fadeSeconds * 1000UL;

  for (size_t i = 0; i < channels.size(); i++) {
    int ch  = constrain(channels[i], 1, 512);
    int tgt = constrain(intensities[i], 0, 255);
    int cur = dmx_values[ch];   // Read from software mirror

    Fade f;
    f.channel       = ch;
    f.start_value   = cur;
    f.target_value  = tgt;
    f.start_time    = now;
    f.duration_ms   = duration_ms;

    activeFades.push_back(f);
  }

  Serial2.print("OK: ");
  Serial2.print(channels.size());
  Serial2.println(" fades started");
}

/* ---------- Utility: parse comma-separated list ---------- */
void parse_list(const String &str, std::vector<int> &out) {
  int start = 0;

  while (true) {
    int comma = str.indexOf(',', start);
    if (comma < 0) {
      out.push_back(str.substring(start).toInt());
      break;
    }
    out.push_back(str.substring(start, comma).toInt());
    start = comma + 1;
  }
}
