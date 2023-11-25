#include "Arduino.h"
#include "RotaryDial2.h"

#define MS_DEBOUNCE 25
#define MS_DIGITEND 600

int RotaryDial2::interruptPin = 3;
int RotaryDial2::last = 0;
unsigned long RotaryDial2::lastHigh = 0;
int RotaryDial2::pulseCount = 0;

void RotaryDial2::setup(int pulsePin) {
  interruptPin = pulsePin;
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), RotaryDial2::change, CHANGE);
}

void RotaryDial2::change() {

  //Ignore interrupts that do not change state
  int state = digitalRead(interruptPin);
  if (state == last) return;
  last = state;

  //Ignore interrupts if there's unread digit
  if (available()) return;

  //Process signal edges
  if (state) {  //RISING

    lastHigh = millis();
    return;

  } else {  //FALLING

    unsigned long highLen = millis() - lastHigh;
    if (highLen > MS_DEBOUNCE) {
      //Serial.println(highLen);
      pulseCount++;
    }
  }

  return;
}

int RotaryDial2::available() {
  return false;
  return (pulseCount > 0 && millis() - lastHigh > MS_DIGITEND);
}

int RotaryDial2::readPulses() {
  if (available()) {
    int ret = pulseCount;
    pulseCount = 0;
    return ret;
  }
  return -1;
}

int RotaryDial2::read() {
  int ret = readPulses();
  if (ret == 10) return 0;
  return ret;
}
