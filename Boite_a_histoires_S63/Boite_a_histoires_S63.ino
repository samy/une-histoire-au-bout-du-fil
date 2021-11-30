#include <SoftwareSerial.h>

#include <DFRobotDFPlayerMini.h>

/**
   Print each digit over Serial as it is dialed.
   See the README for detailed documentation.
*/
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
#include <RotaryDialer.h>

#define PIN_READY  A2
#define PIN_PULSE 2
#define PIN_HANG A3
int numberSpecified = -1;
int hangCounter = 0;

RotaryDialer dialer = RotaryDialer(PIN_READY, PIN_PULSE);

void setup() {
  Serial.begin(9600);

  mySoftwareSerial.begin(9600);
  dialer.setup();
  if (!myDFPlayer.begin(mySoftwareSerial, true, false)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true);
  }
  myDFPlayer.pause();
  myDFPlayer.volume(10);
  pinMode(PIN_HANG, INPUT_PULLUP);
}

void loop() {
  if (isHangedUp()) {
    hangCounter++;
    if (hangCounter > 20) {
      myDFPlayer.pause();
      return;
    }
  } else {
    hangCounter = 0;
  }
  if (dialer.update()) {
    numberSpecified = getDialedNumber(dialer);
  }
  if (numberSpecified != -1) {
    myDFPlayer.play(1);
    numberSpecified = -1;
  }
}

int getDialedNumber(RotaryDialer dialerObject) {
  int number = dialer.getNextNumber();
  return number == 0 ? 10 : number;
}
bool isHangedUp() {
  int sensorValue = analogRead(PIN_HANG);
  return sensorValue > 1000;
}
