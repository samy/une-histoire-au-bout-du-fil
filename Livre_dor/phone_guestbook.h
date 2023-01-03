#include "FatLib/FatFile.h"
#include <Audio.h>

#include <Arduino.h>
#include <SD.h>

#ifndef PHONE_GUESTBOOK
#define PHONE_GUESTBOOK

class PhoneGuestBook {
public:
  void enableIntroBeforeRecord();
  void enableIntroBeforePlay();
  bool needToPlayIntro();
  bool needToPlayBeep();
  bool introRecordEnabled;
  bool introPlayEnabled;
  void enableRecordMode();
  int getMode();
  void setMode(int);
  void writeOutHeader(File audioFile);
  void startPlaying();
  void startRecording();
  void continuePlaying();
  void continueRecording();
  void stopRecording();
  void stopPlaying();
  void playIntro();
  void playBeep();

  void playFileSynchronous();
private:
  int phoneMode;
  bool introHasBeenPlayed;
};


#ifndef RECORDS_NUMBER_FILE_NAME
#define RECORDS_NUMBER_FILE_NAME ".records_file_number"
#endif



#define PIN_HANG 0         //Port lié au fil du décrochage
#define PIN_RESET 1        //Pour annuler l'enregistrement courant
#define PIN_REPLAY 2       //Pour réécouter le dernier enregistrement
#define PIN_MODE_CHANGE 3  //Pour indiquer qu'on doit changer de mode
#define PIN_LED 4          //Pour indiquer que le mode enregistrement est actif

#define RECORD_MODE 1
#define PLAY_MODE 0

#ifndef RECORD_INTRO_FILENAME
#define RECORD_INTRO_FILENAME "intro_record.wav"
#endif

#ifndef PLAY_INTRO_FILENAME
#define PLAY_INTRO_FILENAME "intro_play.wav"
#endif

#ifndef COMMON_INTRO_FILENAME
#define COMMON_INTRO_FILENAME "intro.wav"
#endif

#ifndef BEEP_ENABLE
#define BEEP_ENABLE true
#endif

bool isHangedUp();
bool needToPlayIntro();



#endif

extern SdFat32 sd;
extern File32 file;
extern AudioPlaySdWav playSdWav;  //xy=546,333
extern AudioOutputI2S i2s1;       //xy=1018,324
extern AudioConnection patchCord1;
extern AudioConnection patchCord2;
extern PhoneGuestBook guestbook;
extern AudioSynthWaveform waveform;
extern char line[40];
extern char recordsNumber[10];
extern char tmpContent;
extern int phoneStatus;
extern File frec;
extern AudioRecordQueue queue1;  //xy=281,63
extern int mode;                 // 0=stopped, 1=recording, 2=playing
extern unsigned long ChunkSize;
extern unsigned long Subchunk1Size;
extern unsigned int AudioFormat;
extern unsigned int numChannels;
extern unsigned long sampleRate;
extern unsigned int bitsPerSample;
extern unsigned long byteRate;  // samplerate x channels x (bitspersample / 8)
extern unsigned int blockAlign;
extern unsigned long Subchunk2Size;
extern unsigned long recByteSaved;
extern unsigned long NumSamples;
extern byte byte1, byte2, byte3, byte4;
extern AudioControlSGTL5000 audioShield;
extern AudioSynthWaveform waveform1;  // To create the "beep" sfx
extern float beep_volume;             // not too loud :-)



#define SD_FAT_TYPE 1

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else   // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN
// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14
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






extern const int myInput;
