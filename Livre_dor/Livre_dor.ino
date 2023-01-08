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
#define INTRO_PLAY_ENABLE false
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
}

void loop() {
  // First, read the buttons
  guestbook.updateButtons();

  switch (guestbook.phoneMode) {
    case Mode::Ready:                    //Téléphone raccroché
      if (buttonRecord.fallingEdge()) {  //Si on décroche
        Serial.println("Décrochage");
        guestbook.phoneMode = Mode::Prompting;
        print_mode();
      }
      // else if (buttonPlay.fallingEdge()) {
      //   //playAllRecordings();
      //   //playLastRecording();
      // }
      break;

    case Mode::Prompting:  //Téléphone décroché, il va démarrer
      // Wait a second for users to put the handset to their ear
      guestbook.wait(1000);
      // Play the greeting inviting them to record their message

      playWav1.play("intro.wav");
      // Wait until the  message has finished playing
      //      while (playWav1.isPlaying()) {
      while (!playWav1.isStopped()) {
        // Check whether the handset is replaced
        guestbook.updateButtons();

        //Si on raccroche le téléphone
        if (buttonRecord.risingEdge()) {
          guestbook.stopEverything();

          print_mode();
          return;
        }
        // if (buttonPlay.fallingEdge()) {
        //   playWav1.stop();
        //   //playAllRecordings();
        //   playLastRecording();
        //   return;
        // }
      }
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
      guestbook.updateButtons();

      // Handset is replaced
      if (buttonRecord.risingEdge()) {
        //Si on raccroche
        Serial.println("Stopping Recording");
        // Stop recording
        guestbook.stopEverything();
        break;
      }
      if (buttonReplay.risingEdge()) {
        guestbook.stopEverything();
        guestbook.playLastRecording();
        break;
      }
      if (buttonReset.risingEdge()) {
        guestbook.stopEverything();
        SD.remove(filename);
        guestbook.phoneMode = Mode::Prompting;

        break;
      }
      guestbook.continueRecording();
      break;

    case Mode::Playing:  // to make compiler happy
      break;

    case Mode::Initialising:  // to make compiler happy
      break;
  }
}

void loop_old() {
  guestbook.updateButtons();


  //guestbook.playIntro();
  delay(25);
  //guestbook.playBeep();
  return;

  return;
  if (isHangedUp()) {
    //stopEverything();

    phoneStatus = 0;
    return;
  } else {
    if (phoneStatus == 0) {
      phoneStatus = 1;
    }
  }


  /* INTRO */
  if (phoneStatus == 1 && guestbook.needToPlayIntro()) {
    phoneStatus = 2;
    guestbook.playIntro();
    if (guestbook.needToPlayBeep()) {
    }
  }
  phoneStatus = 2;
}

void initEnvironnement() {
  guestbook.phoneMode = Mode::Initialising;

  Serial.begin(9600);
  AudioMemory(60);

  // Wait for USB Serial
  // NECESSAIRE POUR REPROGRAMMER LE TEENSY
  audioShield.enable();
  audioShield.inputSelect(myInput);
  //audioShield.micGain(40);  //0-63
  audioShield.volume(0.3);  //0-1

  // Initialize the SD.
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  pinMode(PIN_HANG, INPUT_PULLUP);
  setSyncProvider(getTeensy3Time);
  FsDateTime::setCallback(dateTime);
  guestbook.phoneMode = Mode::Ready;
  print_mode();


  // waveform.frequency(440);
  //waveform.amplitude(1.0);
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

void print_mode(void) {  // only for debugging
  Serial.print("Mode switched to: ");
  // Initialising, Ready, Prompting, Recording, Playing
  if (guestbook.phoneMode == Mode::Ready) Serial.println(" Ready");
  else if (guestbook.phoneMode == Mode::Prompting) Serial.println(" Prompting");
  else if (guestbook.phoneMode == Mode::Recording) Serial.println(" Recording");
  else if (guestbook.phoneMode == Mode::Playing) Serial.println(" Playing");
  else if (guestbook.phoneMode == Mode::Initialising) Serial.println(" Initialising");
  else Serial.println(" Undefined");
}
