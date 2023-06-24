/*
Dans l'IDE Arduino, choisir comme réglages:
- USB Type : Serial + MTP Disk
*/

/* Les paramètres (n° de PIN, activation du MTP, etc) sont dans le fichier Settings.h */
#include "Settings.h"



#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <TimeLib.h>
#include "RotaryDialer.h" /* Gestion du cadran rotatif */

#ifdef MTP_ENABLE
#include <MTP_Teensy.h>
#endif

#include "play_sd_wav.h"  // local copy with fixes
#define TOGGLE_WATCHDOG_LED()


#define INTRO_RECORD_ENABLE true
#define INTRO_FILENAME "intro.wav"

#include "phone_guestbook.h"

const int myInput = AUDIO_INPUT_MIC;

RotaryDialer dialer = RotaryDialer(PIN_READY, PIN_PULSE);
int numberSpecified = -1;
/* Constantes pour cadran UK */
int needToPrint = 0;
int count;
int lastState = LOW;
int trueState = LOW;
long unsigned int lastStateChangeTime = 0;
int cleared = 0;
long unsigned int dialHasFinishedRotatingAfterMs = 100;
long unsigned int debounceDelay = 10;

//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
//#define error(s) sd.errorHalt(&Serial, F(s))
//-----------------
//------------------------------------------------------------------------------
void setup() {
  initEnvironnement();
  guestbook.stopEverything();
  dialer.setup();
  if (0 == digitalRead(PIN_MODE_CHANGE)) {
    guestbook.setFeature(Feature::Recorder);
  } else {
    guestbook.setFeature(Feature::Player);
  }
}

void loop() {
  guestbook.updateButtons();
#ifdef MTP_ENABLE
  if (guestbook.isOn) {
    MTP.loop();  // This is mandatory to be placed in the loop code.
  }
#endif

  //guestbook.adjustVolume();
  // First, read the buttons
  switch (guestbook.phoneMode) {
    case Mode::Ready:                                       //Téléphone raccroché
      if (1 == digitalRead(PIN_HANG) && !guestbook.isOn) {  //Si on décroche
        Serial.println("Décrochage");
        guestbook.isOn = true;
        if (0 == digitalRead(PIN_MODE_CHANGE)) {
          guestbook.phoneMode = Mode::Prompting;
        } else {
          guestbook.phoneMode = Mode::Playing;
        }

        guestbook.print_mode();
      } else {
        return;
      }

      /* Changement mode */
      if (0 == digitalRead(PIN_MODE_CHANGE) && guestbook.getFeature() != Feature::Recorder) {
        guestbook.setFeature(Feature::Recorder);
      }
      // else if (buttonPlay.fallingEdge()) {
      //   //playAllRecordings();
      //   //playLastRecording();
      // }
      break;

    case Mode::Prompting:  //Téléphone décroché, il va démarrer
      // Wait a second for users to put the handset to their ear
      guestbook.wait(1000);
      if (guestbook.isPlaying()) {
        guestbook.stopPlaying();
      }

      delay(2000);
      if (guestbook.getFeature() == Feature::Recorder) {
        guestbook.playIntro();
        Serial.println("Fin intro");
      }
      // Wait until the  message has finished playing
      while (!playWav1.isStopped()) {
        // Check whether the handset is replaced
        // guestbook.updateButtons();

        //Si on raccroche le téléphone
        if (0 == digitalRead(PIN_HANG)) {
          Serial.println("Raccrochage");
          guestbook.stopEverything();
          return;
        }

        //Si on passe le téléphone en mode lecteur
        if (1 == digitalRead(PIN_MODE_CHANGE)) {
          Serial.println("Lecteur");
          guestbook.stopEverything();
          guestbook.setFeature(Feature::Player);
          guestbook.setMode(Mode::Playing);
          return;
        }
      }

      // Play the tone sound effect
      if (guestbook.needToPlayBeep()) {
        waveform.begin(0.10, 440, WAVEFORM_SINE);
        guestbook.wait(1250);
        waveform.amplitude(0);
      }

      Serial.println("Starting Recording");
      guestbook.startRecording();
      break;

    case Mode::Recording:
      guestbook.updateButtons();
      // Handset is replaced
      if (0 == digitalRead(PIN_HANG)) {
        //Si on raccroche
        Serial.println("Arrêt enregistrement");
        Serial.println("Raccrochage");
        // Stop recording
        guestbook.stopEverything();
        break;
      } else {
        guestbook.continueRecording();
      }
#ifdef RESET_ENABLE
      if (buttonReset.fallingEdge()) {
        Serial.println("Reset");
        guestbook.stopEverything();

        SD.remove(filename);
        guestbook.setMode(Mode::Prompting);
      }
#endif

      if (1 == digitalRead(PIN_MODE_CHANGE)) {
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
      if (0 == digitalRead(PIN_MODE_CHANGE)) {
        Serial.println("Bascule mode");
        guestbook.setFeature(Feature::Recorder);
        guestbook.phoneMode = Mode::Prompting;
        guestbook.print_mode();
        return;
      }

      if (!playWav1.isStopped()) {
        guestbook.continuePlaying();



        while (playWav1.isPlaying()) {
          //guestbook.updateButtons();
          if (dialer.update()) {
            numberSpecified = getDialedNumber(dialer);
            Serial.println("Cadran");
            guestbook.stopPlaying();
            delay(500);
            guestbook.startPlayingRandomAudio();
          }

          //Si on passe le téléphone en mode enregistreur
          if (0 == digitalRead(PIN_MODE_CHANGE)) {
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
#ifdef RESET_ENABLE
          if (buttonReset.fallingEdge()) {
            guestbook.stopPlaying();
          }
#endif
        }
      } else {
        //On joue un audio
        if (playWav1.isStopped()) {
          guestbook.startPlayingRandomAudio();
          delay(1500);
        }

        //Si on passe le téléphone en mode enregistreur
        if (0 == digitalRead(PIN_MODE_CHANGE) && guestbook.getFeature() != Feature::Recorder) {
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
#ifdef RESET_ENABLE
        if (buttonReset.fallingEdge()) {
          guestbook.stopPlaying();
        }
#endif
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

  //Réglages pour Electret standard
  audioShield.volume(0.9);
  mixer.gain(0, 1.5f);
  mixer.gain(1, 1.5f);
  audioShield.micGain(0);

  audioShield.lineInLevel(0);

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
#ifdef RESET_ENABLE
  pinMode(PIN_RESET, INPUT_PULLUP);
#endif

  pinMode(PIN_MODE_CHANGE, INPUT_PULLUP);

  pinMode(PIN_LED, OUTPUT);
  setSyncProvider(getTeensy3Time);
  FsDateTime::setCallback(dateTime);

  guestbook.setMode(Mode::Ready);

  // Gestion du mode lecteur / enregistreur
  if (0 == digitalRead(PIN_MODE_CHANGE)) {
    Serial.println("Recorder");
    guestbook.setFeature(Feature::Recorder);
  } else {
    Serial.println("Player");
    guestbook.setFeature(Feature::Player);
  }
  digitalWrite(PIN_LED, LOW);
  guestbook.hasAnAudioBeenPlayedBefore = false;
  dialer.setup();

  if (!SD.exists(RECORDS_FOLDER_NAME)) {
    if (SD.mkdir(RECORDS_FOLDER_NAME)) {
      Serial.println("Created arduino/log directory");
    } else {
      Serial.println("Failed to create arduino/log directory");
    }
  }
#ifdef MTP_ENABLE
  MTP.begin();
  MTP.addFilesystem(SD, "Livre d'or");  // choose a nice name for the SD card volume to appear in your file explorer
  guestbook.MTPcheckInterval = MTP.storage()->get_DeltaDeviceCheckTimeMS();
  Serial.println("Added SD card via MTP");

#endif
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

int getUkDialerNumber() {
  int reading = digitalRead(PIN_PULSE);
  int numberSpecified = -1;
  if ((millis() - lastStateChangeTime) > dialHasFinishedRotatingAfterMs) {
    // the dial isn't being dialed, or has just finished being dialed.
    if (needToPrint) {
      // if it's only just finished being dialed, we need to send the number down the serial
      // line and reset the count. We mod the count by 10 because '0' will send 10 pulses.
      numberSpecified = count % 10;
      needToPrint = 0;
      count = 0;
      cleared = 0;
    }
  }

  if (reading != lastState) {
    lastStateChangeTime = millis();
  }
  if ((millis() - lastStateChangeTime) > debounceDelay) {
    // debounce - this happens once it's stablized
    if (reading != trueState) {
      // this means that the switch has either just gone from closed->open or vice versa.
      trueState = reading;
      if (trueState == HIGH) {
        // increment the count of pulses if it's gone high.
        count++;
        needToPrint = 1;  // we'll need to print this number (once the dial has finished rotating)
      }
    }
  }
  lastState = reading;
  return numberSpecified == 0 ? 10 : numberSpecified;
}

bool switchToRecordMode() {
  return buttonChange.fallingEdge();
}

bool switchToPlayMode() {
  return buttonChange.risingEdge();
}
