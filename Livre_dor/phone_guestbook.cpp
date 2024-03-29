#include "AudioControl.h"
#include "Settings.h"

#include "Bounce.h"
#include "play_sd_wav.h"
#include "synth_waveform.h"
#include "FatLib/FatFile.h"
#include "core_pins.h"
#include "phone_guestbook.h"

#if MTP_ENABLE
#include <MTP_Teensy.h>
#endif

Mode phoneMode = Mode::Initialising;

/* Récupération de l'état de décroché/raccroché */
bool PhoneGuestBook::isRaccroche() {
  return 1 == digitalRead(PIN_HANG);
}


bool PhoneGuestBook::needToPlayIntro() {
  if (this->introHasBeenPlayed) {
    return false;
  }
  if ((this->getFeature() == Feature::Recorder && this->introRecordEnabled)
      || (this->getFeature() == Feature::Player && this->introPlayEnabled)) {
    this->introHasBeenPlayed = true;
    return true;
  }
  return false;
}
bool PhoneGuestBook::needToPlayBeep() {
  return BEEP_ENABLE;
}
void PhoneGuestBook::stopEverything() {
  guestbook.isOn = false;
  if (!playWav1.isStopped()) {
    playWav1.stop();
  }
  if (guestbook.getMode() == Mode::Recording) {
    guestbook.stopRecording();
  }
  if (guestbook.getMode() != Mode::Sleep) {
    guestbook.setMode(Mode::Sleep);
  }
  guestbook.hasAnAudioBeenPlayedBefore = false;
  guestbook.introHasBeenPlayed = false;
  digitalWrite(PIN_LED, LOW);
  audioShield.muteHeadphone();
}
void PhoneGuestBook::enableIntroBeforeRecord() {
  this->introRecordEnabled = true;
}

void PhoneGuestBook::adjustVolume() {
  int sensorValue = analogRead(A1);
  audioShield.lineOutLevel(13 + (int)((30 - 13) * ((float)sensorValue / 1023)));
}

void PhoneGuestBook::enableIntroBeforePlay() {
  this->introPlayEnabled = true;
}

int PhoneGuestBook::getMode() {
  return this->phoneMode;
}

int PhoneGuestBook::getFeature() {
  return this->feature;
}

void PhoneGuestBook::setMode(Mode mode) {
  this->phoneMode = mode;
  print_mode();
}

void PhoneGuestBook::setFeature(Feature feature) {
  this->introHasBeenPlayed = false;
  this->feature = feature;
  if (feature == Feature::Recorder) {
    digitalWrite(PIN_LED, HIGH);
  } else {
    digitalWrite(PIN_LED, LOW);
  }
  print_feature();
}

void PhoneGuestBook::writeOutHeader() {  // update WAV header with final filesize/datasize
                                         //  NumSamples = (recByteSaved*8)/bitsPerSample/numChannels;
                                         //  Subchunk2Size = NumSamples*numChannels*bitsPerSample/8; // number of samples x number of channels x number of bytes per sample
  Subchunk2Size = recByteSaved - 42;     // because we didn't make space for the header to start with! Lose 21 samples...
  ChunkSize = Subchunk2Size + 34;        // was 36;
  frec.seek(0);
  frec.write("RIFF");
  byte1 = ChunkSize & 0xff;
  byte2 = (ChunkSize >> 8) & 0xff;
  byte3 = (ChunkSize >> 16) & 0xff;
  byte4 = (ChunkSize >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  frec.write("WAVE");
  frec.write("fmt ");
  byte1 = Subchunk1Size & 0xff;
  byte2 = (Subchunk1Size >> 8) & 0xff;
  byte3 = (Subchunk1Size >> 16) & 0xff;
  byte4 = (Subchunk1Size >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  byte1 = AudioFormat & 0xff;
  byte2 = (AudioFormat >> 8) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  byte1 = numChannels & 0xff;
  byte2 = (numChannels >> 8) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  byte1 = sampleRate & 0xff;
  byte2 = (sampleRate >> 8) & 0xff;
  byte3 = (sampleRate >> 16) & 0xff;
  byte4 = (sampleRate >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  byte1 = byteRate & 0xff;
  byte2 = (byteRate >> 8) & 0xff;
  byte3 = (byteRate >> 16) & 0xff;
  byte4 = (byteRate >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  byte1 = blockAlign & 0xff;
  byte2 = (blockAlign >> 8) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  byte1 = bitsPerSample & 0xff;
  byte2 = (bitsPerSample >> 8) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write("data");
  byte1 = Subchunk2Size & 0xff;
  byte2 = (Subchunk2Size >> 8) & 0xff;
  byte3 = (Subchunk2Size >> 16) & 0xff;
  byte4 = (Subchunk2Size >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  frec.close();
  //Serial.println("header written");
  //Serial.print("Subchunk2: ");
  //Serial.println(Subchunk2Size);
}

SdFat32 sd;
File32 file;

AudioSynthWaveform waveform;  //xy=404,310
AudioPlaySdWavX playWav1;     //xy=412,441
AudioMixer4 mixer;            //xy=752,329
AudioOutputI2S i2s1;          //xy=1048,323
AudioRecordQueue queue1;      // Creating an audio buffer in memory before saving to SD
AudioInputI2S i2s2;           // I2S input from microphone on audio shield

AudioConnection patchCord1(waveform, 0, mixer, 0);  // wave to mixer
AudioConnection patchCord3(playWav1, 0, mixer, 1);  // wav file playback mixer
AudioConnection patchCord6(mixer, 0, i2s1, 0);
AudioConnection patchCord4(mixer, 0, i2s1, 1);
AudioConnection patchCord5(i2s2, 0, queue1, 0);  // mic input to queue (L)   // mic input to queue (L)

PhoneGuestBook guestbook;
char line[40];
char recordsNumber[10];
char tmpContent;
int phoneStatus = 0;
// The file where data is recorded
File frec;
File recordsDir = SD.open(RECORDS_FOLDER_NAME);  //Root Directory
//int mode = 0;             // 0=stopped, 1=recording, 2=playing
unsigned long ChunkSize = 0L;
unsigned long Subchunk1Size = 16;
unsigned int AudioFormat = 1;
unsigned int numChannels = 1;
unsigned long sampleRate = 44100;
unsigned int bitsPerSample = 16;
unsigned long byteRate = sampleRate * numChannels * (bitsPerSample / 8);  // samplerate x channels x (bitspersample / 8)
unsigned int blockAlign = numChannels * bitsPerSample / 8;
unsigned long Subchunk2Size = 0L;
unsigned long recByteSaved = 0L;
unsigned long NumSamples = 0L;
byte byte1, byte2, byte3, byte4;
AudioControlSGTL5000 audioShield;
char filename[15];

/* 2023-07-10 : 500 for TESLA model, 750 for others */
Bounce buttonHang = Bounce(PIN_HANG, 750);  //High bounce delay since it is an ON/OFF and not a temporary pressed button

Bounce buttonChange = Bounce(PIN_MODE_CHANGE, 150);  //High bounce delay since it is an ON/OFF and not a temporary pressed button
#ifdef REPLAY_ENABLE
Bounce buttonReplay = Bounce(PIN_REPLAY, 40);
#endif

#ifdef RESET_ENABLE
Bounce buttonReset = Bounce(PIN_RESET, 150);
#endif


void PhoneGuestBook::startPlaying() {
  Serial.println("startPlaying");
  playWav1.play("intro.wav");
  //mode = 2;
}


void PhoneGuestBook::playIntro() {
  audioShield.unmuteHeadphone();
  playWav1.play(INTRO_FILENAME);
}

void PhoneGuestBook::playBeep() {
  waveform.begin(WAVEFORM_SINE);
  waveform.amplitude(0.10);

  waveform.amplitude(0);
}





void PhoneGuestBook::continueRecording() {
#if defined(INSTRUMENT_SD_WRITE)
  uint32_t started = micros();
#endif  // defined(INSTRUMENT_SD_WRITE)
#define NBLOX 2
  // Check if there is data in the queue
  if (queue1.available() >= NBLOX) {
    byte buffer[NBLOX * AUDIO_BLOCK_SAMPLES * sizeof(int16_t)];
    // Fetch 2 blocks from the audio library and copy
    // into a 512 byte buffer.  The Arduino SD library
    // is most efficient when full 512 byte sector size
    // writes are used.
    for (int i = 0; i < NBLOX; i++) {
      memcpy(buffer + i * AUDIO_BLOCK_SAMPLES * sizeof(int16_t), queue1.readBuffer(), AUDIO_BLOCK_SAMPLES * sizeof(int16_t));
      queue1.freeBuffer();
    }
    // Write all 512 bytes to the SD card
    frec.write(buffer, sizeof buffer);
    recByteSaved += sizeof buffer;
  }

#if defined(INSTRUMENT_SD_WRITE)
  started = micros() - started;
  if (started > worstSDwrite)
    worstSDwrite = started;

  if (millis() >= printNext) {
    Serial.printf("Worst write took %luus\n", worstSDwrite);
    worstSDwrite = 0;
    printNext = millis() + 250;
  }
#endif  // defined(INSTRUMENT_SD_WRITE)
}
#if MTP_ENABLE
void PhoneGuestBook::setMTPdeviceChecks(bool nable) {
  if (nable) {
    MTP.storage()->set_DeltaDeviceCheckTimeMS(this->MTPcheckInterval);
    Serial.print("En");
  } else {
    MTP.storage()->set_DeltaDeviceCheckTimeMS((uint32_t)-1);
    Serial.print("Dis");
  }
  Serial.println("abled MTP storage device checks");
}
#endif

void PhoneGuestBook::startRecording() {
  audioShield.muteHeadphone();

  guestbook.setMode(Mode::Recording);
  print_mode();
#if MTP_ENABLE
  setMTPdeviceChecks(false);  // disable MTP device checks while recording
#endif

  digitalWrite(PIN_LED, HIGH);
#if defined(INSTRUMENT_SD_WRITE)
  worstSDwrite = 0;
  printNext = 0;
#endif  // defined(INSTRUMENT_SD_WRITE)
  // Find the first available file number
  //  for (uint8_t i=0; i<9999; i++) { // BUGFIX uint8_t overflows if it reaches 255
  for (uint16_t i = 0; i < 9999; i++) {
    // Format the counter as a five-digit number with leading zeroes, followed by file extension
    snprintf(filename, 12 + strlen(RECORDS_FOLDER_NAME), "%s%05d.wav", RECORDS_FOLDER_NAME, i);
    // Create if does not exist, do not open existing, write, sync after write
    if (!SD.exists(filename)) {
      break;
    }
  }

  //Serial.print("filename ");
  //Serial.println(filename);
  frec = SD.open(filename, FILE_WRITE);
  //Serial.println("Opened file !");
  if (frec) {
    Serial.print("Recording to ");
    Serial.println(filename);
    queue1.begin();
    recByteSaved = 0L;
  } else {
    Serial.println("Couldn't open file to record!");
  }
}

void PhoneGuestBook::startRecording(int subfolder) {
  audioShield.muteHeadphone();
  char subfolderPath[strlen(RECORDS_FOLDER_NAME) + 3];
  snprintf(subfolderPath, strlen(RECORDS_FOLDER_NAME) + 3, "%s%d/", RECORDS_FOLDER_NAME, subfolder);
  Serial.print("Dossier ");
  Serial.println(subfolderPath);
  if (!SD.exists(subfolderPath)) {
    SD.mkdir(subfolderPath);
  }

  guestbook.setMode(Mode::Recording);
  print_mode();
#if MTP_ENABLE
  setMTPdeviceChecks(false);  // disable MTP device checks while recording
#endif

  digitalWrite(PIN_LED, HIGH);
#if defined(INSTRUMENT_SD_WRITE)
  worstSDwrite = 0;
  printNext = 0;
#endif  // defined(INSTRUMENT_SD_WRITE)
  // Find the first available file number
  //  for (uint8_t i=0; i<9999; i++) { // BUGFIX uint8_t overflows if it reaches 255
  for (uint16_t i = 0; i < 9999; i++) {
    // Format the counter as a five-digit number with leading zeroes, followed by file extension
    snprintf(filename, 12 + strlen(subfolderPath), "%s%05d.wav", subfolderPath, i);
    // Create if does not exist, do not open existing, write, sync after write
    if (!SD.exists(filename)) {
      break;
    }
  }

  //Serial.print("filename ");
  //Serial.println(filename);
  frec = SD.open(filename, FILE_WRITE);
  //Serial.println("Opened file !");
  if (frec) {
    Serial.print("Recording to ");
    Serial.println(filename);
    queue1.begin();
    recByteSaved = 0L;
  } else {
    Serial.println("Couldn't open file to record!");
  }
}

void PhoneGuestBook::print_mode(void) {  // only for debugging
  Serial.print("Bascule en mode: ");
  // Initialising, Sleep, Prompting, Recording, Playing
  if (phoneMode == Mode::Sleep) Serial.println(" En veille");
  else if (phoneMode == Mode::Prompting) Serial.println(" Prêt à enregistrer");
  else if (phoneMode == Mode::Recording) Serial.println(" Enregistrement");
  else if (phoneMode == Mode::Playing) Serial.println(" Lecture");
  else if (phoneMode == Mode::Initialising) Serial.println(" Initialisation");
  else Serial.println(" Indéfini");
}
void PhoneGuestBook::print_feature(void) {  // only for debugging
  Serial.print("Feature switched to: ");
  if (feature == Feature::Player) Serial.println(" Player");
  else if (feature == Feature::Recorder) Serial.println(" Recorder");
}


void PhoneGuestBook::continuePlaying() {
  if (!playWav1.isPlaying()) {
    playWav1.stop();
  }
}

void PhoneGuestBook::stopPlaying() {
  Serial.println("stopPlaying");
  playWav1.stop();
}

bool PhoneGuestBook::isPlaying() {
  return playWav1.isPlaying();
}

void PhoneGuestBook::stopRecording() {
  // Stop adding any new data to the queue
  queue1.end();
  // Flush all existing remaining data from the queue
  while (queue1.available() > 0) {
    // Save to open file
    frec.write((byte*)queue1.readBuffer(), AUDIO_BLOCK_SAMPLES * sizeof(int16_t));
    queue1.freeBuffer();
    recByteSaved += AUDIO_BLOCK_SAMPLES * sizeof(int16_t);
  }
  Serial.println("Ecriture headers");
  this->writeOutHeader();
  // Close the file
  frec.close();
  //Serial.println("Closed file");
  guestbook.setMode(Mode::Sleep);
  digitalWrite(PIN_LED, LOW);
  Serial.println("Desactivation LED (stopRecording)");
#if MTP_ENABLE
  setMTPdeviceChecks(true);  // enable MTP device checks, recording is finished
#endif
}


void PhoneGuestBook::startPlayingRandomAudio() {
  guestbook.stopPlaying();
  // Find the first available file number
  int counter = 0;
  for (int i = 0; i < 9999; i++) {
    snprintf(filename, 12 + strlen(RECORDS_FOLDER_NAME), "%s%05d.wav", RECORDS_FOLDER_NAME, i);
    if (!SD.exists(filename)) {
      counter = i;
      break;
    }
  }
  if (counter == 0) {
    Serial.println("No files to play");
    return;
  }
  Serial.println("startPlayingRandomAudio");



  snprintf(filename, 12 + strlen(RECORDS_FOLDER_NAME), "%s%05d.wav", RECORDS_FOLDER_NAME, (int)random(0, counter));
  Serial.println(filename);
  if (guestbook.hasAnAudioBeenPlayedBefore) {
    delay(DELAY_BETWEEN_PLAYS);
  }
  guestbook.hasAnAudioBeenPlayedBefore = true;
  playWav1.play(filename);
}

void PhoneGuestBook::startPlayingRandomAudioFromNumberFolders() {
  guestbook.stopPlaying();
  int chosenFolder = (int)random(0, 9);
  // Find the first available file number
  int counter = 0;
  for (int i = 0; i < 9999; i++) {
    snprintf(filename, 12 + strlen(RECORDS_FOLDER_NAME) + 3, "%s%d/%05d.wav", RECORDS_FOLDER_NAME, chosenFolder, i);

    if (!SD.exists(filename)) {
      counter = i;
      break;
    }
  }
  if (counter == 0) {
    Serial.println("No files to play");

    return;
  }
  Serial.println("startPlayingRandomAudio");



  snprintf(filename, 12 + strlen(RECORDS_FOLDER_NAME), "%s%d/%05d.wav", RECORDS_FOLDER_NAME, chosenFolder, (int)random(0, counter));
  Serial.println(filename);
  if (guestbook.hasAnAudioBeenPlayedBefore) {
    delay(DELAY_BETWEEN_PLAYS);
  }
  guestbook.hasAnAudioBeenPlayedBefore = true;
  playWav1.play(filename);
}


void PhoneGuestBook::wait(unsigned int milliseconds) {
  elapsedMillis msec = 0;

  while (msec <= milliseconds) {
    // buttonHang.update();
    // buttonPlay.update();
    // if (buttonHang.fallingEdge()) Serial.println("Button (pin 0) Press");
    // if (buttonPlay.fallingEdge()) Serial.println("Button (pin 1) Press");
    // if (buttonHang.risingEdge()) Serial.println("Button (pin 0) Release");
    // if (buttonPlay.risingEdge()) Serial.println("Button (pin 1) Release");
  }
}

void PhoneGuestBook::updateButtons() {
  buttonHang.update();
  buttonChange.update();

#ifdef RESET_ENABLE

  buttonReset.update();
#endif
}
