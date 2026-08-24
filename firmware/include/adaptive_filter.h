#pragma once
#include <stdint.h>
#include "config.h"

class NlmsFilter {
 public:
  void reset();
  int16_t process(int16_t reference, int16_t microphone, bool adapt, float learningRate);
  float inputRms() const;
  float outputRms() const;
 private:
  float weights[FILTER_LENGTH] = {};
  float history[FILTER_LENGTH] = {};
  uint16_t position = 0;
  float inputEnergy = 0;
  float outputEnergy = 0;
  uint32_t samples = 0;
};
