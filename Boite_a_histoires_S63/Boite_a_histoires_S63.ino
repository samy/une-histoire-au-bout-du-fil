/**
 * Print each digit over Serial as it is dialed.
 * See the README for detailed documentation.
 */

#include <RotaryDialer.h>

#define PIN_READY  A2
#define PIN_PULSE 2

RotaryDialer dialer = RotaryDialer(PIN_READY, PIN_PULSE);

void setup() {
  Serial.begin(9600);
  dialer.setup();
}

void loop() {
  if (dialer.update()) {
    Serial.print("Numéro ");
    Serial.println(dialer.getNextNumber());
  }
}
