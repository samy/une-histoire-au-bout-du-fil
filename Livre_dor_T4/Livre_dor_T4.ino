#include <SD.h>

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

#define RECORDS_NUMBER_FILE_NAME ".records_file_number"
#define INTRO_RECORD_ENABLE true
#define INTRO_PLAY_ENABLE false


#define PIN_HANG 10  //Port lié au fil du décrochage

char phoneMode[] = "record";
char introHasBeenPlayed=false;

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
  if (phoneStatus == 1 && needToPlayIntro()) {
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

char getRecordsNumber() {
  if (!file.open(RECORDS_NUMBER_FILE_NAME, FILE_WRITE)) {
    error("open failed");
  }

  //int bits = file.attrib();
  //file.attrib(bits | FS_ATTRIB_HIDDEN);

  file.rewind();
  tmpContent = 0;
  while (file.available()) {
    tmpContent = (char) file.read();
  }
  if (sizeof(tmpContent) == 0) {
    tmpContent = 0;
  }
  file.close();
  return tmpContent;
}

void setRecordsNumber(char number) {
  if (!file.open(RECORDS_NUMBER_FILE_NAME, FILE_WRITE)) {
    error("open failed");
  }

  //int bits = file.attrib();
  //file.attrib(bits | FS_ATTRIB_HIDDEN);

  file.truncate();
  file.print(number);
  file.close();
}

bool needToPlayIntro() {
  if (introHasBeenPlayed) {
    return false;
  }
  if ((strcmp(phoneMode, "record") == 0 && INTRO_RECORD_ENABLE)
      || (strcmp(phoneMode, "play") == 0 && INTRO_PLAY_ENABLE)) {
    introHasBeenPlayed = true;
    return true;
  }
  return false;
}

/* Récupération de l'état de décroché/raccroché */
bool isHangedUp() {
  return 1 == digitalRead(PIN_HANG);
}