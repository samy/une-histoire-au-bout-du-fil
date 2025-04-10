#include "Arduino.h"

#pragma once

class RotaryDial2 {
public:
  static void setup(int PulsePin, int MaxDialNumbers);
  static int available();
  static int readPulses();
  static int read();
private:
  static unsigned long lastHigh;
  static int pulseCount;
  static int interruptPin;
  static int maxDialNumbers;
  static void change();
  static int last;
};