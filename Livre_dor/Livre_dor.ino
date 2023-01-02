#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#define INTRO_RECORD_ENABLE true
#define INTRO_PLAY_ENABLE false
#define INTRO_FILENAME "intro.wav"


#include "phone_guestbook.h"




//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
#define error(s) sd.errorHalt(&Serial, F(s))
//-----------------
//------------------------------------------------------------------------------
void setup() {

  initEnvironnement();
}

void loop() {
  if (isHangedUp()) {
    stopEverything();

    phoneStatus = 0;
    return;
  } else {
    if (phoneStatus == 0) {
      phoneStatus = 1;
    }
  }
  if (phoneStatus == 1 && guestbook.needToPlayIntro()) {
    phoneStatus = 2;
    guestbook.playIntro();
  }
  phoneStatus = 2;
}

void initEnvironnement() {

  Serial.begin(9600);

  // Wait for USB Serial
  // NECESSAIRE POUR REPROGRAMMER LE TEENSY
  while (!Serial) {
    yield();
  }
  while (!Serial.available()) {
    yield();
  }



  // Initialize the SD.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
    return;
  }
  audioShield.enable();
  audioShield.inputSelect(myInput);
  audioShield.micGain(40);  //0-63
  audioShield.volume(0.5);  //0-1

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
}

void stopEverything() {
  guestbook.stopPlaying();
  guestbook.stopRecording();
}
