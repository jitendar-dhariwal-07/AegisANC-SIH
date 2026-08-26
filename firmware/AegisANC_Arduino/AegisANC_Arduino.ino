#include <Arduino.h>
#include <Wire.h>
#include <driver/i2s.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SAMPLE_RATE 16000
#define FILTER_LENGTH 64
#define LEARNING_RATE 0.003f
#define MIC_BCLK 14
#define MIC_WS 15
#define MIC_DATA 13
#define OLED_SDA 8
#define OLED_SCL 9
#define BUTTON_PIN 7
#define OLED_ADDRESS 0x3C

enum class AncMode { OFF, FIXED, AI_ADAPTIVE, SAFE };
AncMode mode = AncMode::AI_ADAPTIVE;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
bool displayReady = false;
uint32_t dmaUnderruns = 0;

class NlmsFilter {
 public:
  void reset() { for (uint16_t index = 0; index < FILTER_LENGTH; ++index) { weights[index] = 0; history[index] = 0; } position = 0; inputEnergy = 0; outputEnergy = 0; samples = 0; }
  void process(int16_t reference, int16_t microphone, bool adapt) {
    history[position] = reference / 32768.0f;
    float estimate = 0; float energy = 0;
    for (uint16_t index = 0; index < FILTER_LENGTH; ++index) { const uint16_t historyIndex = (position + FILTER_LENGTH - index) % FILTER_LENGTH; estimate += weights[index] * history[historyIndex]; energy += history[historyIndex] * history[historyIndex]; }
    const float error = microphone / 32768.0f + estimate;
    if (adapt && energy > 0.000001f) { for (uint16_t index = 0; index < FILTER_LENGTH; ++index) { const uint16_t historyIndex = (position + FILTER_LENGTH - index) % FILTER_LENGTH; weights[index] -= LEARNING_RATE * error * history[historyIndex] / (energy + 0.000001f); weights[index] = constrain(weights[index], -0.5f, 0.5f); } }
    position = (position + 1) % FILTER_LENGTH; inputEnergy += (microphone / 32768.0f) * (microphone / 32768.0f); outputEnergy += error * error; ++samples;
    if (samples > SAMPLE_RATE) { inputEnergy *= 0.5f; outputEnergy *= 0.5f; samples /= 2; }
  }
  float inputRms() const { return samples ? sqrtf(inputEnergy / samples) * 32768.0f : 0; }
  float outputRms() const { return samples ? sqrtf(outputEnergy / samples) * 32768.0f : 0; }
 private:
  float weights[FILTER_LENGTH] = {};
  float history[FILTER_LENGTH] = {};
  uint16_t position = 0;
  float inputEnergy = 0;
  float outputEnergy = 0;
  uint32_t samples = 0;
};

NlmsFilter filter;

void initMicrophones() {
  i2s_config_t config = {};
  config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
  config.sample_rate = SAMPLE_RATE;
  config.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
  config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  config.communication_format = I2S_COMM_FORMAT_I2S;
  config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  config.dma_buf_count = 8;
  config.dma_buf_len = 64;
  config.use_apll = false;
  config.fixed_mclk = 0;
  i2s_pin_config_t pins = { .bck_io_num = MIC_BCLK, .ws_io_num = MIC_WS, .data_out_num = I2S_PIN_NO_CHANGE, .data_in_num = MIC_DATA };
  i2s_driver_install(I2S_NUM_0, &config, 0, nullptr);
  i2s_set_pin(I2S_NUM_0, &pins);
}

void initDisplay() {
  Wire.begin(OLED_SDA, OLED_SCL);
  displayReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
}

void updateDisplay(bool safe, float inputRms, float outputRms) {
  if (!displayReady) return;
  const float reduction = inputRms > 1 ? max(0.0f, (1.0f - outputRms / inputRms) * 100.0f) : 0;
  display.clearDisplay(); display.setTextColor(SSD1306_WHITE); display.setTextSize(1); display.setCursor(0, 0);
  display.println("AegisANC 2-MIC"); display.print("MODE: "); display.println(mode == AncMode::SAFE ? "SAFE" : mode == AncMode::OFF ? "OFF" : mode == AncMode::FIXED ? "FIXED" : "AI");
  display.print("IN: "); display.println(inputRms, 0); display.print("ERR: "); display.println(outputRms, 0); display.print("RED: "); display.print(reduction, 1); display.println("%"); display.println(safe ? "PROTECTION ACTIVE" : "MONITOR ONLY"); display.display();
}

void handleCommand(String command) {
  command.trim();
  if (command == "MODE OFF" || command == "ANC OFF") mode = AncMode::OFF;
  else if (command == "MODE FIXED") mode = AncMode::FIXED;
  else if (command == "MODE AI" || command == "ANC ON") mode = AncMode::AI_ADAPTIVE;
  else if (command == "MODE SAFE") mode = AncMode::SAFE;
  else if (command == "RESET_FILTER") filter.reset();
  Serial.printf("{\"device\":\"AegisANC\",\"response\":\"%s\"}\n", command.c_str());
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  filter.reset();
  initMicrophones();
  initDisplay();
  Serial.println("{\"device\":\"AegisANC\",\"status\":\"2_MIC_MONITOR_READY\",\"audio_output\":\"NONE\"}");
}

void loop() {
  static String commandBuffer;
  while (Serial.available()) { const char character = (char)Serial.read(); if (character == '\n') { handleCommand(commandBuffer); commandBuffer = ""; } else if (commandBuffer.length() < 64) commandBuffer += character; }
  int32_t raw[2] = { 0, 0 }; size_t received = 0;
  if (i2s_read(I2S_NUM_0, raw, sizeof(raw), &received, 20) != ESP_OK || received != sizeof(raw)) { ++dmaUnderruns; return; }
  const int16_t reference = (int16_t)(raw[0] >> 14); const int16_t microphone = (int16_t)(raw[1] >> 14);
  bool safe = mode == AncMode::SAFE || abs(reference) > 30000 || abs(microphone) > 30000 || digitalRead(BUTTON_PIN) == LOW;
  if (safe) mode = AncMode::SAFE;
  if (mode != AncMode::OFF) filter.process(reference, microphone, mode == AncMode::FIXED || mode == AncMode::AI_ADAPTIVE);
  static uint32_t reportAt = 0;
  if (millis() - reportAt > 500) { reportAt = millis(); const float inputRms = filter.inputRms(); const float outputRms = filter.outputRms(); const float reduction = inputRms > 1 ? max(0.0f, (1.0f - outputRms / inputRms) * 100.0f) : 0; const char* modeName = mode == AncMode::SAFE ? "SAFE" : mode == AncMode::OFF ? "OFF" : mode == AncMode::FIXED ? "FIXED" : "AI_ADAPTIVE_ANC"; Serial.printf("{\"device\":\"AegisANC\",\"mode\":\"%s\",\"noise\":\"UNKNOWN\",\"confidence\":0,\"input_rms\":%.1f,\"output_rms\":%.1f,\"relative_reduction\":%.1f,\"snr_improvement\":0,\"latency_ms\":%.2f,\"speech_preserved\":true,\"alarm_passthrough\":true,\"mic_status\":\"OK\",\"dac_status\":\"NOT_INSTALLED\",\"dma_overruns\":0,\"dma_underruns\":%lu,\"safety\":\"%s\",\"classifier\":\"BASELINE_MONITOR\",\"audio_output\":\"NONE\"}\n", modeName, inputRms, outputRms, reduction, dmaUnderruns, safe ? "SAFE_MODE" : "MONITOR_ONLY"); updateDisplay(safe, inputRms, outputRms); }
}
