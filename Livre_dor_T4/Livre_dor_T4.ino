#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#define INTRO_RECORD_ENABLE true
#define INTRO_PLAY_ENABLE false


#include "phone_guestbook.h"

#define SD_FAT_TYPE 1

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else   // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASS

SdFat32 sd;
File32 file;

AudioPlaySdWav           playSdWav1;     //xy=546,333
AudioOutputI2S           i2s1;           //xy=1018,324
AudioConnection          patchCord1(playSdWav1, 0, i2s1, 0);
AudioConnection          patchCord2(playSdWav1, 1, i2s1, 1);

PhoneGuestBook guestbook;


char line[40];
char recordsNumber[10];
char tmpContent;
int phoneStatus = 0;



//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
#define error(s) sd.errorHalt(&Serial, F(s))
//-----------------
//------------------------------------------------------------------------------
void setup() {
  initEnvironnement();
}

void loop() {
  /* Si le téléphone est raccroché, on stoppe la lecture du MP3 (il n'a pas de véritable stop() et on passe à l'itération suivante */
  if (isHangedUp()) {
    //myDFPlayer.pause();

    phoneStatus = 0;
    return;
  } else {
    if (phoneStatus == 0) {
      phoneStatus = 1;
    }
  }
  if (phoneStatus == 1 && guestbook.needToPlayIntro()) {
    phoneStatus = 2;
    //playIntro();
  }
  phoneStatus = 2;
}

void initEnvironnement() {

  Serial.begin(9600);

  // Wait for USB Serial
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
}

// char getRecordsNumber() {
//   if (!file.open(RECORDS_NUMBER_FILE_NAME, FILE_WRITE)) {
//     error("open failed");
//   }

//   //int bits = file.attrib();
//   //file.attrib(bits | FS_ATTRIB_HIDDEN);

//   file.rewind();
//   tmpContent = 0;
//   while (file.available()) {
//     tmpContent = (char) file.read();
//   }
//   if (sizeof(tmpContent) == 0) {
//     tmpContent = 0;
//   }
//   file.close();
//   return tmpContent;
// }

// void setRecordsNumber(char number) {
//   if (!file.open(RECORDS_NUMBER_FILE_NAME, FILE_WRITE)) {
//     error("open failed");
//   }

//   //int bits = file.attrib();
//   //file.attrib(bits | FS_ATTRIB_HIDDEN);

//   file.truncate();
//   file.print(number);
//   file.close();
// }
