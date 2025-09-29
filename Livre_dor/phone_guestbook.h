#include "FatLib/FatFile.h"
#include <Audio.h>

#include <Arduino.h>
#include <SD.h>
#include <Bounce.h>
#include "RotaryDial2.h" /* Gestion du cadran rotatif */

#ifndef PHONE_GUESTBOOK
#define PHONE_GUESTBOOK
enum Mode { Initialising,
            Sleep,
            Prompting,
            Recording,
            Playing,
};

enum Feature {
  Recorder,
  Player
};
class PhoneGuestBook {
public:
  void enableIntroBeforeRecord();
  void enableIntroBeforePlay();
  bool needToPlayIntro();
  bool needToPlayBeep();
  bool introRecordEnabled = false;
  bool introPlayEnabled;
  uint32_t MTPcheckInterval;  // default value of device check interval [ms]

  void enableRecordMode();
  int getMode();
  int getFeature();

  void setMode(Mode mode);
  void setFeature(Feature feature);

  void writeOutHeader();
  void setMTPdeviceChecks(bool nable);
  void startPlaying();
  void startRecording();
  void startRecording(int subfolder);

  void continuePlaying();
  void continueRecording();
  void stopRecording();
  void stopPlaying();

  bool isPlaying();
  bool isRaccroche();

  void playIntro();
  void playBeep();
  void playLastRecording();
  void wait(unsigned int milliseconds);
  void updateButtons();
  void stopEverything();
  Mode phoneMode;
  Feature feature;
  void adjustVolume();
  void print_mode();
  void print_feature();
  void playRandomAudio();
  void startPlayingRandomAudio();
  void startPlayingRandomAudioFromNumberFolders();
  bool hasAnAudioBeenPlayedBefore;
  bool isOn;
  bool introHasBeenPlayed;
};


#ifndef RECORDS_NUMBER_FILE_NAME
#define RECORDS_NUMBER_FILE_NAME ".records_file_number"
#endif

#ifndef INTRO_PLAY_ENABLE
#define INTRO_PLAY_ENABLE false
#endif

#ifndef INTRO_FILENAME
#define INTRO_FILENAME "intro.wav"
#endif




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
#define BEEP_ENABLE false
#endif

#ifndef RECORD_LED_ENABLE
#define RECORD_LED_ENABLE true
#endif

#ifndef RECORDS_FOLDER_NAME
#define RECORDS_FOLDER_NAME "/Messages/"
#endif

#ifndef DELAY_BETWEEN_PLAYS
#define DELAY_BETWEEN_PLAYS 2000
#endif

bool isHangedUp();
bool needToPlayIntro();



#endif

extern SdFat32 sd;
extern File32 file;
extern AudioPlaySdWavX playWav1;  //xy=546,333
extern AudioOutputI2S i2s1;       //xy=1018,324
extern AudioConnection patchCord1;
extern AudioRecordQueue queue1;  // Creating an audio buffer in memory before saving to SD

extern AudioConnection patchCord2;
extern AudioConnection patchCord3;
extern AudioConnection patchCord4;
extern AudioConnection patchCord5;
extern PhoneGuestBook guestbook;

extern AudioSynthWaveform waveform;
extern AudioMixer4 mixer;          // Allows merging several inputs to same output
extern AudioFilterBiquad biquad1;  //xy=970,445

extern char line[40];
extern char recordsNumber[10];
extern char tmpContent;
extern int phoneStatus;
extern uint32_t MTPcheckInterval;
extern File frec;
extern AudioRecordQueue queue1;  //xy=281,63
//extern int mode;                 // 0=stopped, 1=recording, 2=playing
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
extern char filename[15];
extern Bounce buttonHang;
extern Bounce buttonChange;

extern Bounce buttonReplay;
extern Bounce buttonReset;




// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14
