#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <TimeLib.h>
//#include <MTP_Teensy.h>
#include "play_sd_wav.h"  // local copy with fixes

#define INTRO_RECORD_ENABLE true
#define INTRO_FILENAME "intro.wav"

#include "phone_guestbook.h"

const int myInput = AUDIO_INPUT_MIC;



//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
#define error(s) sd.errorHalt(&Serial, F(s))
//-----------------
//------------------------------------------------------------------------------
void setup() {
  initEnvironnement();
  guestbook.stopEverything();
}

void loop() {
  guestbook.adjustVolume();
  // First, read the buttons
  guestbook.updateButtons();
  switch (guestbook.phoneMode) {
    case Mode::Ready:                    //Téléphone raccroché
      if (1 == digitalRead(PIN_HANG)) {  //Si on décroche
        Serial.println("Décrochage");
        guestbook.phoneMode = Mode::Prompting;
        guestbook.print_mode();
      }
      // else if (buttonPlay.fallingEdge()) {
      //   //playAllRecordings();
      //   //playLastRecording();
      // }
      break;

    case Mode::Prompting:  //Téléphone décroché, il va démarrer
      // Wait a second for users to put the handset to their ear
      guestbook.wait(1000);

      guestbook.playIntro();
      // Wait until the  message has finished playing
      while (!playWav1.isStopped()) {
        // Check whether the handset is replaced
        guestbook.updateButtons();

        //Si on raccroche le téléphone
        if (0 == digitalRead(PIN_HANG)) {
          Serial.println("Raccrochage");
          guestbook.stopEverything();
          return;
        }

        //Si on passe le téléphone en mode lecteur
        if (0 == digitalRead(PIN_MODE_CHANGE)) {
          Serial.println("Lecteur");
          guestbook.stopEverything();
          guestbook.setFeature(Feature::Player);
          guestbook.setMode(Mode::Playing);
          return;
        }
      }
      Serial.println("Fin intro");

      // Play the tone sound effect
      if (guestbook.needToPlayBeep()) {
        waveform.begin(0.8, 440, WAVEFORM_SINE);
        guestbook.wait(1250);
        waveform.amplitude(0);
      }

      Serial.println("Starting Recording");
      guestbook.startRecording();
      break;

    case Mode::Recording:
      digitalWrite(PIN_LED, HIGH);

      // Handset is replaced
      if (0 == digitalRead(PIN_HANG)) {
        //Si on raccroche
        Serial.println("Stopping Recording");
        // Stop recording
        guestbook.stopEverything();
        break;
      }
      if (buttonReset.fallingEdge()) {
        Serial.println("Falling");
        guestbook.stopEverything();

        SD.remove(filename);
        guestbook.setMode(Mode::Prompting);
      }
      //Si on passe le téléphone en mode lecteur
      if (0 == digitalRead(PIN_MODE_CHANGE)) {
        Serial.println("Lecteur");
        guestbook.stopEverything();
        guestbook.setFeature(Feature::Player);
        guestbook.setMode(Mode::Playing);
        return;
      }
      guestbook.continueRecording();
      break;

    case Mode::Playing:
      guestbook.adjustVolume();
      if (!playWav1.isStopped()) {
        while (playWav1.isPlaying()) {
          //Si on passe le téléphone en mode enregistreur
          if (1 == digitalRead(PIN_MODE_CHANGE)) {
            Serial.println("Enregistreur");
            guestbook.stopEverything();
            guestbook.setFeature(Feature::Recorder);
            guestbook.setMode(Mode::Prompting);
            return;
          }
          //Si on raccroche
          if (0 == digitalRead(PIN_HANG)) {
            Serial.println("Stopping playing");
            guestbook.stopEverything();
            break;
          }
        }
      } else {
        //On joue un audio
        guestbook.startPlayingRandomAudio();
      }

      break;

    case Mode::Initialising:
      break;
  }
}

void initEnvironnement() {
  guestbook.phoneMode = Mode::Initialising;

  Serial.begin(9600);
  AudioMemory(60);

  audioShield.enable();
  audioShield.inputSelect(myInput);
  //audioShield.micGain(40);  //0-63
  audioShield.volume(0.3);  //0-1

  // Initialize the SD.
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  pinMode(PIN_HANG, INPUT_PULLUP);
  pinMode(PIN_RESET, INPUT_PULLUP);
  pinMode(PIN_MODE_CHANGE, INPUT_PULLUP);

  pinMode(PIN_LED, OUTPUT);
  setSyncProvider(getTeensy3Time);
  FsDateTime::setCallback(dateTime);

  guestbook.setMode(Mode::Ready);

  // Gestion du mode lecteur / enregistreur
  if (1 == digitalRead(PIN_MODE_CHANGE)) {
    guestbook.setFeature(Feature::Recorder);
  } else {
    guestbook.setFeature(Feature::Player);
  }
}

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}
// Callback to assign timestamps for file system operations
void dateTime(uint16_t* date, uint16_t* time, uint8_t* ms10) {

  // Return date using FS_DATE macro to format fields.
  *date = FS_DATE(year(), month(), day());

  // Return time using FS_TIME macro to format fields.
  *time = FS_TIME(hour(), minute(), second());

  // Return low time bits in units of 10 ms.
  *ms10 = second() & 1 ? 100 : 0;
}
