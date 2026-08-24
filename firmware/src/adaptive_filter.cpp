#include "adaptive_filter.h"
#include <math.h>

void NlmsFilter::reset() { for (uint16_t i = 0; i < FILTER_LENGTH; ++i) { weights[i] = 0; history[i] = 0; } position = 0; inputEnergy = outputEnergy = 0; samples = 0; }
int16_t NlmsFilter::process(int16_t reference, int16_t microphone, bool adapt, float learningRate) {
  history[position] = reference / 32768.0f;
  float estimate = 0, energy = 0;
  for (uint16_t i = 0; i < FILTER_LENGTH; ++i) { uint16_t index = (position + FILTER_LENGTH - i) % FILTER_LENGTH; estimate += weights[i] * history[index]; energy += history[index] * history[index]; }
  const float error = microphone / 32768.0f - estimate;
  if (adapt && energy > 0.000001f) { for (uint16_t i = 0; i < FILTER_LENGTH; ++i) { uint16_t index = (position + FILTER_LENGTH - i) % FILTER_LENGTH; weights[i] += learningRate * error * history[index] / (energy + 0.000001f); if (weights[i] > 0.5f) weights[i] = 0.5f; if (weights[i] < -0.5f) weights[i] = -0.5f; } }
  position = (position + 1) % FILTER_LENGTH; inputEnergy += (microphone / 32768.0f) * (microphone / 32768.0f); outputEnergy += error * error; ++samples;
  if (samples > SAMPLE_RATE) { inputEnergy *= 0.5f; outputEnergy *= 0.5f; samples /= 2; }
  float limited = error * OUTPUT_GAIN * 32767.0f; if (limited > 8191) limited = 8191; if (limited < -8191) limited = -8191; return (int16_t)limited;
}
float NlmsFilter::inputRms() const { return samples ? sqrtf(inputEnergy / samples) * 32768.0f : 0; }
float NlmsFilter::outputRms() const { return samples ? sqrtf(outputEnergy / samples) * 32768.0f : 0; }
