/* Constantes pour cadran UK */
int needToPrint = 0;
int count;
int lastState = LOW;
int trueState = LOW;
long unsigned int lastStateChangeTime = 0;
int cleared = 0;
long unsigned int dialHasFinishedRotatingAfterMs = 100;
long unsigned int debounceDelay = 10;

/* Les paramètres (n° de PIN, activation du MTP, etc) sont dans le fichier Settings.h */
#include "Settings.h"

#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <TimeLib.h>
#include "RotaryDial2.h" /* Gestion du cadran rotatif */

#if MTP_ENABLE
#include <MTP_Teensy.h>
#endif

#include "play_sd_wav.h"  // local copy with fixes
#define TOGGLE_WATCHDOG_LED()


#define INTRO_RECORD_ENABLE true
#define INTRO_FILENAME "INTRO.WAV"

#include "phone_guestbook.h"

const int myInput = AUDIO_INPUT_MIC;

int numberSpecified = -1;

//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
//#define error(s) sd.errorHalt(&Serial, F(s))
//-----------------
//------------------------------------------------------------------------------
void setup() {
  initEnvironnement();
}

void loop() {
  guestbook.updateButtons();

  /* Bascule en mode "livre d'or" */
  if (phoneSwitchedToRecordMode()) {
    guestbook.setFeature(Feature::Recorder);
    guestbook.stopEverything();
    guestbook.setMode(Mode::Prompting);
    return;
  }

  /* Bascule en lecteur */
  if (phoneSwitchedToPlayMode()) {
    guestbook.setFeature(Feature::Player);
    guestbook.setMode(Mode::Playing);

    return;
  }

  /* Raccrochage */
  if (phoneClosed()) {
    guestbook.isOn = false;
    guestbook.stopEverything();
    delay(1000);
    return;
  }

#if MTP_ENABLE
  if (guestbook.isOn) {
    MTP.loop();  // This is mandatory to be placed in the loop code.
  }
#endif
  // First, read the buttons
  switch (guestbook.phoneMode) {
    case Mode::Sleep:
      if (guestbook.isRaccroche()) {
        guestbook.isOn = false;
        audioShield.muteHeadphone();
        guestbook.introHasBeenPlayed = false;
        return;
      }
      if (!guestbook.isOn) {  //Si on décroche

        guestbook.isOn = true;
        if (isInRecordModeAccordingToSwitch()) {
          guestbook.setMode(Mode::Prompting);
        } else {
          Serial.println("switch to play");
          guestbook.setMode(Mode::Playing);
        }
        return;
      }

      /* Changement mode */
      if (phoneSwitchedToRecordMode()) {
        guestbook.setFeature(Feature::Recorder);
        return;
      }
      if (switchToPlayMode()) {
        guestbook.setFeature(Feature::Player);
        return;
      }
      break;

    case Mode::Prompting:  //Téléphone décroché, il va démarrer
      Serial.println("Mode Prompting");
      // Wait a second for users to put the handset to their ear
      guestbook.wait(1000);
      if (guestbook.isPlaying()) {
        guestbook.stopPlaying();
      }
      if (guestbook.getFeature() == Feature::Recorder && guestbook.needToPlayIntro()) {
        delay(200);
        Serial.println("Debut intro");
        guestbook.playIntro();

        // Wait until the  message has finished playing
        while (!playWav1.isStopped()) {
          // Check whether the handset is replaced
          // guestbook.updateButtons();

          //Si on raccroche le téléphone
          if (guestbook.isRaccroche()) {
            Serial.println("Raccrochage");
            guestbook.stopEverything();
            delay(2000);
            return;
          }

          //Si on passe le téléphone en mode lecteur
          if (switchToPlayMode()) {
            Serial.println("Lecteur");
            guestbook.stopEverything();
            guestbook.setFeature(Feature::Player);
            guestbook.setMode(Mode::Playing);
            return;
          }
        }
        Serial.println("Fin intro");

        audioShield.muteHeadphone();
      }


      // Play the tone sound effect
      if (!RECORD_ON_DIAL && guestbook.needToPlayBeep()) {
        waveform.begin(0.10, 440, WAVEFORM_SINE);
        guestbook.wait(1250);
        waveform.amplitude(0);
      }

      if (!RECORD_ON_DIAL) {
        Serial.println("Starting Recording");

        guestbook.startRecording();

      } else {
        if (RotaryDial2::available() || numberSpecified != -1) {
          if (numberSpecified == -1) {
            Serial.print("Lecture numéro");
            numberSpecified = RotaryDial2::read();
            Serial.println(numberSpecified);
          }
          if (numberSpecified != -1) {
            Serial.println("Cadran");
            delay(500);

            guestbook.startRecording(numberSpecified);
            numberSpecified = -1;
            return;
          }
        } else {
          //guestbook.setMode(Mode::Prompting);
        }
      }


      break;

    case Mode::Recording:

      guestbook.updateButtons();
      // Handset is replaced
      if (phoneClosed() || guestbook.isRaccroche()) {
        //Si on raccroche
        Serial.println("Arrêt enregistrement");
        Serial.println("Raccrochage");
        // Stop recording
        guestbook.stopEverything();
        delay(2000);
        break;
      } else {
        if (switchToPlayMode()) {
          Serial.println("Desactivation LED (switchToPlayMode)");

          Serial.println("Lecteur");
          guestbook.stopEverything();
          guestbook.setFeature(Feature::Player);
          guestbook.setMode(Mode::Playing);
          digitalWrite(PIN_LED, LOW);
          return;
        }
        if (RotaryDial2::available()) {
          Serial.println("Nouveau numéro pendant enregistrement");
          numberSpecified = RotaryDial2::read();
          Serial.println("Arrêt enregistrement");
          guestbook.stopEverything();
          guestbook.startRecording(numberSpecified);
          numberSpecified = -1;

        } else {
          guestbook.continueRecording();
        }
      }
#ifdef RESET_ENABLE
      if (buttonReset.fallingEdge()) {
        Serial.println("Reset");
        guestbook.stopEverything();

        SD.remove(filename);
        guestbook.setMode(Mode::Prompting);
      }
#endif

      break;

    case Mode::Playing:

      if (RotaryDial2::available() || numberSpecified != -1) {
        if (numberSpecified == -1) {
          numberSpecified = RotaryDial2::read();
        }

        if (numberSpecified != -1) {
          Serial.println("Cadran");
          delay(500);
          numberSpecified = -1;
          if (RECORD_ON_DIAL) {
            guestbook.startPlayingRandomAudioFromNumberFolders();
          } else {
            guestbook.startPlayingRandomAudio();
          }
          return;
        }
      }
      if (phoneSwitchedToRecordMode()) {

        Serial.println("Bascule mode");
        guestbook.setFeature(Feature::Recorder);

        guestbook.setMode(Mode::Prompting);
        return;
      }

      if (!playWav1.isStopped()) {
        guestbook.continuePlaying();
        while (playWav1.isPlaying()) {
          guestbook.updateButtons();


          //Si on passe le téléphone en mode enregistreur
          if (phoneSwitchedToRecordMode()) {
            Serial.println("Enregistreur");
            guestbook.stopEverything();
            guestbook.setFeature(Feature::Recorder);

            guestbook.setMode(Mode::Prompting);
            return;
          }
          //Si on raccroche
          if (guestbook.isRaccroche()) {
            Serial.println("Stopping playing");
            guestbook.stopEverything();
            delay(2000);
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
        if (playWav1.isStopped() && AUTO_PLAY) {
          if (RECORD_ON_DIAL) {
            guestbook.startPlayingRandomAudioFromNumberFolders();
          } else {
            guestbook.startPlayingRandomAudio();
          }
          return;
        }
        if (RotaryDial2::available() || numberSpecified != -1) {
          if (numberSpecified == -1) {
            numberSpecified = RotaryDial2::read();
          }

          if (numberSpecified != -1) {
            Serial.println("Cadran");
            delay(500);
            numberSpecified = -1;
            if (RECORD_ON_DIAL) {
              guestbook.startPlayingRandomAudioFromNumberFolders();
            } else {
              guestbook.startPlayingRandomAudio();
            }
            return;
          }
        }

        //Si on passe le téléphone en mode enregistreur
        if (phoneSwitchedToRecordMode()) {
          Serial.println("Enregistreur");
          guestbook.stopEverything();
          guestbook.setFeature(Feature::Recorder);

          guestbook.setMode(Mode::Prompting);
          return;
        }
        //Si on raccroche
        if (guestbook.isRaccroche()) {
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
  guestbook.enableIntroBeforeRecord();
  Serial.begin(9600);
  AudioMemory(60);
  audioShield.enable(); 
  audioShield.inputSelect(myInput);

  

  //Réglages pour Electret standard
  audioShield.unmuteHeadphone();
  audioShield.volume(0.5);
  audioShield.micGain(0);



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

  guestbook.setMode(Mode::Sleep);


  // Gestion du mode lecteur / enregistreur
  if (1 == digitalRead(PIN_MODE_CHANGE)) {
    guestbook.setFeature(Feature::Recorder);

  } else {
    guestbook.setFeature(Feature::Recorder);
  }
  if (!guestbook.isRaccroche()) {
    if (guestbook.getFeature() == Feature::Recorder) {
      guestbook.setMode(Mode::Prompting);
    } else {
      guestbook.setMode(Playing);
    }
  }

  digitalWrite(PIN_LED, LOW);
  guestbook.hasAnAudioBeenPlayedBefore = false;

  if (!SD.exists(RECORDS_FOLDER_NAME)) {
    if (SD.mkdir(RECORDS_FOLDER_NAME)) {
      Serial.println("Created arduino/log directory");
    } else {
      Serial.println("Failed to create arduino/log directory");
    }
  }
#if MTP_ENABLE
  MTP.begin();
  MTP.addFilesystem(SD, "Livre d'or");  // choose a nice name for the SD card volume to appear in your file explorer
  guestbook.MTPcheckInterval = MTP.storage()->get_DeltaDeviceCheckTimeMS();
  Serial.println("Added SD card via MTP");
#endif
  Serial.println("Activation cadran");
  RotaryDial2::setup(PIN_PULSE);
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

bool phoneSwitchedToRecordMode() {
  bool var = (REVERSE_MODE_CHANGE ? buttonChange.fallingEdge() : buttonChange.risingEdge()) && guestbook.getFeature() != Feature::Recorder;
  if (var) {
    Serial.println("phoneSwitchedToRecordMode");
  }
  return var;
}

bool phoneSwitchedToPlayMode() {
  bool var = (REVERSE_MODE_CHANGE ? buttonChange.risingEdge() : buttonChange.fallingEdge()) && guestbook.getFeature() != Feature::Player;
  if (var) {
    Serial.println("phoneSwitchedToPlayMode");
  }
  return var;
}

bool phoneClosed() {
  bool var = buttonHang.risingEdge() && guestbook.getMode() != Mode::Sleep;
  if (var) {
    Serial.println("phoneClosed");
  }
  return var;
}

bool isInRecordModeAccordingToSwitch() {
  bool var = (REVERSE_MODE_CHANGE ? 0 : 1) == digitalRead(PIN_MODE_CHANGE);
  if (var) {
    Serial.println("Yes, phone in record Mode according to switch");
  }
  return var;
}

bool switchToPlayMode() {
  return (REVERSE_MODE_CHANGE ? buttonChange.risingEdge() : buttonChange.fallingEdge());
}
