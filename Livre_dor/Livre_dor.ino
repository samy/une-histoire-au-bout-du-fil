/*
Dans l'IDE Arduino, choisir comme réglages:
- USB Type : Serial + MTP Disk
*/


#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <TimeLib.h>
#include "RotaryDialer.h" /* Gestion du cadran rotatif */

#include <MTP_Teensy.h>

#include "play_sd_wav.h"  // local copy with fixes
#define TOGGLE_WATCHDOG_LED()


#define INTRO_RECORD_ENABLE true
#define INTRO_FILENAME "intro.wav"

#include "phone_guestbook.h"

const int myInput = AUDIO_INPUT_MIC;
#define PIN_READY 1
#define PIN_PULSE 2
RotaryDialer dialer = RotaryDialer(PIN_READY, PIN_PULSE);
int numberSpecified = -1;

//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
//#define error(s) sd.errorHalt(&Serial, F(s))
//-----------------
//------------------------------------------------------------------------------
void setup() {
  initEnvironnement();
  guestbook.stopEverything();
  dialer.setup();
}

void loop() {
  MTP.loop();  // This is mandatory to be placed in the loop code.

  //guestbook.adjustVolume();
  // First, read the buttons
  guestbook.updateButtons();
  switch (guestbook.phoneMode) {
    case Mode::Ready:                    //Téléphone raccroché
      if (0 == digitalRead(PIN_HANG)) {  //Si on décroche
        Serial.println("Décrochage");
        guestbook.phoneMode = Mode::Prompting;
        guestbook.print_mode();
      }
      if (1 == digitalRead(PIN_MODE_CHANGE)) {
        Serial.println("Bascule mode");
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
        if (1 == digitalRead(PIN_HANG)) {
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
        waveform.begin(0.5, 440, WAVEFORM_SINE);
        guestbook.wait(1250);
        waveform.amplitude(0);
      }

      Serial.println("Starting Recording");
      guestbook.startRecording();
      break;

    case Mode::Recording:

      // Handset is replaced
      if (buttonRecord.risingEdge()) {
        //Si on raccroche
        Serial.println("Stopping Recording");
        // Stop recording
        guestbook.stopEverything();
        break;
      } else {
        guestbook.continueRecording();
      }
      if (buttonReset.fallingEdge()) {
        Serial.println("Reset");
        guestbook.stopEverything();

        SD.remove(filename);
        guestbook.setMode(Mode::Prompting);
      }
      if (0 == digitalRead(PIN_MODE_CHANGE)) {
        Serial.println("Lecteur");
        guestbook.stopEverything();
        guestbook.setFeature(Feature::Player);
        guestbook.setMode(Mode::Playing);
        digitalWrite(PIN_LED, 0);
        return;
      }

      break;

    case Mode::Playing:
      guestbook.adjustVolume();
      if (1 == digitalRead(PIN_MODE_CHANGE)) {
        Serial.println("Bascule mode");
        guestbook.phoneMode = Mode::Prompting;
        guestbook.print_mode();
        return;
      }

      if (!playWav1.isStopped()) {
        guestbook.continuePlaying();



        while (playWav1.isPlaying()) {
          guestbook.updateButtons();
          if (dialer.update()) {
            numberSpecified = getDialedNumber(dialer);
            guestbook.stopPlaying();
          }


          //Si on passe le téléphone en mode enregistreur
          if (1 == digitalRead(PIN_MODE_CHANGE)) {
            Serial.println("Enregistreur");
            guestbook.stopEverything();
            guestbook.setFeature(Feature::Recorder);
            guestbook.setMode(Mode::Prompting);
            return;
          }
          //Si on raccroche
          if (1 == digitalRead(PIN_HANG)) {
            Serial.println("Stopping playing");
            guestbook.stopEverything();
            break;
          }
          if (buttonReset.fallingEdge()) {
            guestbook.stopPlaying();
          }
        }



      } else {
        //On joue un audio
        Serial.println("startPlayingRandomAudio");
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
  AudioMemory(200);

  audioShield.enable();
  audioShield.inputSelect(myInput);
  audioShield.volume(0.6);  //0-1
  mixer.gain(0, 0.5f);
  mixer.gain(1, 0.5f);
  audioShield.micGain(0);
  audioShield.unmuteLineout();

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
    Serial.println("Recorder");
    guestbook.setFeature(Feature::Recorder);
  } else {
    Serial.println("Player");
    guestbook.setFeature(Feature::Player);
  }
  digitalWrite(PIN_LED, LOW);
  guestbook.hasAnAudioBeenPlayedBefore = false;
  dialer.setup();
  MTP.begin();
  MTP.addFilesystem(SD, "Livre d'or");  // choose a nice name for the SD card volume to appear in your file explorer
  Serial.println("Added SD card via MTP");
  guestbook.MTPcheckInterval = MTP.storage()->get_DeltaDeviceCheckTimeMS();
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

/* Récupération du numéro composé (le DFPlayer démarre à 1, donc le 0 est converti en 10) */
int getDialedNumber(RotaryDialer dialerObject) {
  int number = dialer.getNextNumber();
  return number == 0 ? 10 : number;
}


