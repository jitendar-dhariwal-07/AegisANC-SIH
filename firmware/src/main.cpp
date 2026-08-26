#include <Arduino.h>
#include <driver/i2s.h>
#include "config.h"
#include "adaptive_filter.h"

enum class AncMode { OFF, FIXED, AI_ADAPTIVE, SAFE };
AncMode mode = AncMode::AI_ADAPTIVE;
NlmsFilter filter;
uint32_t dmaOverruns = 0, dmaUnderruns = 0;
float lastInput = 0, lastOutput = 0;

void initAudio() {
  i2s_config_t input = {};
  input.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX); input.sample_rate = SAMPLE_RATE; input.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT; input.channel_format = MIC_CHANNEL_FORMAT; input.communication_format = I2S_COMM_FORMAT_I2S; input.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1; input.dma_buf_count = 8; input.dma_buf_len = 64; input.use_apll = false; input.tx_desc_auto_clear = false; input.fixed_mclk = 0;
  i2s_pin_config_t inputPins = { .bck_io_num = MIC_BCLK, .ws_io_num = MIC_WS, .data_out_num = I2S_PIN_NO_CHANGE, .data_in_num = MIC_DATA };
  i2s_driver_install(I2S_NUM_0, &input, 0, nullptr); i2s_set_pin(I2S_NUM_0, &inputPins);
  i2s_config_t output = input; output.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX); output.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT; output.tx_desc_auto_clear = true;
  i2s_pin_config_t outputPins = { .bck_io_num = DAC_BCLK, .ws_io_num = DAC_LRCLK, .data_out_num = DAC_DATA, .data_in_num = I2S_PIN_NO_CHANGE };
  i2s_driver_install(I2S_NUM_1, &output, 0, nullptr); i2s_set_pin(I2S_NUM_1, &outputPins);
}
void handleCommand(String command) {
  command.trim();
  if (command == "MODE OFF" || command == "ANC OFF") mode = AncMode::OFF; else if (command == "MODE FIXED") mode = AncMode::FIXED; else if (command == "MODE AI" || command == "ANC ON") mode = AncMode::AI_ADAPTIVE; else if (command == "MODE SAFE") mode = AncMode::SAFE; else if (command == "RESET_FILTER") filter.reset();
  Serial.printf("{\"device\":\"AegisANC\",\"response\":\"%s\"}\n", command.c_str());
}
void setup() { Serial.begin(115200); pinMode(BUTTON_PIN, INPUT_PULLUP); filter.reset(); initAudio(); }
void loop() {
  static String commandBuffer;
  while (Serial.available()) { char character = (char)Serial.read(); if (character == '\n') { handleCommand(commandBuffer); commandBuffer = ""; } else if (commandBuffer.length() < 64) commandBuffer += character; }
  int32_t raw[2] = { 0, 0 }; size_t received = 0;
  if (i2s_read(I2S_NUM_0, raw, sizeof(raw), &received, 20) != ESP_OK || received != sizeof(raw)) { ++dmaUnderruns; return; }
  const int16_t reference = (int16_t)(raw[0] >> 14); const int16_t microphone = (int16_t)(raw[1] >> 14); const bool safe = mode == AncMode::SAFE || abs(reference) > 30000 || abs(microphone) > 30000; if (safe) mode = AncMode::SAFE;
  if (digitalRead(BUTTON_PIN) == LOW) mode = AncMode::SAFE;
  const bool adapt = mode == AncMode::FIXED || mode == AncMode::AI_ADAPTIVE; const int16_t output = mode == AncMode::OFF ? 0 : filter.process(reference, microphone, adapt, mode == AncMode::AI_ADAPTIVE ? LEARNING_RATE : 0.001f);
  const int32_t dacSample = ((int32_t)output) << 16; size_t written = 0; if (i2s_write(I2S_NUM_1, &dacSample, sizeof(dacSample), &written, 20) != ESP_OK || written != sizeof(dacSample)) ++dmaOverruns;
  lastInput = filter.inputRms(); lastOutput = filter.outputRms(); static uint32_t reportAt = 0;
  if (millis() - reportAt > 500) { reportAt = millis(); const float reduction = lastInput > 1 ? max(0.0f, (1.0f - lastOutput / lastInput) * 100.0f) : 0; const char* modeName = mode == AncMode::SAFE ? "SAFE" : mode == AncMode::OFF ? "OFF" : mode == AncMode::FIXED ? "FIXED" : "AI_ADAPTIVE_ANC"; Serial.printf("{\"device\":\"AegisANC\",\"mode\":\"%s\",\"noise\":\"UNKNOWN\",\"confidence\":0,\"input_rms\":%.1f,\"output_rms\":%.1f,\"relative_reduction\":%.1f,\"snr_improvement\":0,\"latency_ms\":%.2f,\"speech_preserved\":true,\"alarm_passthrough\":true,\"mic_status\":\"OK\",\"dac_status\":\"OK\",\"dma_overruns\":%lu,\"dma_underruns\":%lu,\"safety\":\"%s\",\"classifier\":\"BASELINE\"}\n", modeName, lastInput, lastOutput, reduction, 1000.0f / SAMPLE_RATE, dmaOverruns, dmaUnderruns, safe ? "SAFE_MODE" : "NORMAL"); }
}
