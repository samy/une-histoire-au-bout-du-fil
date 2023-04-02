#include "Bounce.h"
#include "play_sd_wav.h"
#include "synth_waveform.h"
#include "FatLib/FatFile.h"
#include "core_pins.h"
#include "phone_guestbook.h"
Mode phoneMode = Mode::Initialising;

/* Récupération de l'état de décroché/raccroché */
bool isHangedUp() {
  return 0 == digitalRead(PIN_HANG);
}


bool PhoneGuestBook::needToPlayIntro() {
  if (this->introHasBeenPlayed) {
    return false;
  }
  if ((this->getMode() == RECORD_MODE && this->introRecordEnabled)
      || (this->getMode() == PLAY_MODE && this->introPlayEnabled)) {
    introHasBeenPlayed = true;
    return true;
  }
  return false;
}
bool PhoneGuestBook::needToPlayBeep() {
  return BEEP_ENABLE;
}
void PhoneGuestBook::stopEverything() {
  if (!playWav1.isStopped()) {
    playWav1.stop();
  }
  if (guestbook.getMode() == Mode::Recording) {
    guestbook.stopRecording();
  }
  //Serial.println("stopEverything");
  if (guestbook.getMode() != Mode::Ready) {
    guestbook.setMode(Mode::Ready);
  }
  guestbook.hasAnAudioBeenPlayedBefore = false;
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

void PhoneGuestBook::setMode(Mode mode) {
  this->phoneMode = mode;
  print_mode();
}

void PhoneGuestBook::setFeature(Feature feature) {
  this->feature = feature;
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

AudioSynthWaveform waveform;                        //xy=404,310
AudioPlaySdWavX playWav1;                           //xy=412,441
AudioMixer4 mixer;                                  //xy=752,329
AudioOutputI2S i2s1;                                //xy=1048,323
AudioRecordQueue queue1;                            // Creating an audio buffer in memory before saving to SD
AudioInputI2S i2s2;                                 // I2S input from microphone on audio shield
AudioConnection patchCord1(waveform, 0, mixer, 0);  // wave to mixer
AudioConnection patchCord3(playWav1, 0, mixer, 1);  // wav file playback mixer
AudioConnection patchCord4(mixer, 0, i2s1, 0);      // mixer output to speaker (L)
AudioConnection patchCord6(mixer, 0, i2s1, 1);      // mixer output to speaker (R)
AudioConnection patchCord5(i2s2, 0, queue1, 0);     // mic input to queue (L)   // mic input to queue (L)

PhoneGuestBook guestbook;
char line[40];
char recordsNumber[10];
char tmpContent;
int phoneStatus = 0;
// The file where data is recorded
File frec;
File recordsDir = SD.open("/RECORD/");  //Root Directory
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
Bounce buttonRecord = Bounce(PIN_HANG, 750);  //High bounce delay since it is an ON/OFF and not a temporary pressed button
Bounce buttonReplay = Bounce(PIN_REPLAY, 40);
Bounce buttonReset = Bounce(PIN_RESET, 150);


void PhoneGuestBook::startPlaying() {
  Serial.println("startPlaying");
  playWav1.play("intro.wav");
  //mode = 2;
}

void PhoneGuestBook::playIntro() {
  playWav1.play(INTRO_FILENAME);
}

void PhoneGuestBook::playBeep() {
  waveform.begin(WAVEFORM_SINE);
  waveform.amplitude(0.75);

  delay(400);
  waveform.amplitude(0);
}





void PhoneGuestBook::continueRecording() {
#if defined(INSTRUMENT_SD_WRITE)
  uint32_t started = micros();
#endif  // defined(INSTRUMENT_SD_WRITE)
#define NBLOX 16
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

void PhoneGuestBook::startRecording() {
  digitalWrite(PIN_LED, HIGH);
#if defined(INSTRUMENT_SD_WRITE)
  worstSDwrite = 0;
  printNext = 0;
#endif  // defined(INSTRUMENT_SD_WRITE)
  // Find the first available file number
  //  for (uint8_t i=0; i<9999; i++) { // BUGFIX uint8_t overflows if it reaches 255
  for (uint16_t i = 0; i < 9999; i++) {
    // Format the counter as a five-digit number with leading zeroes, followed by file extension
    snprintf(filename, 20, "/%s/%05d.wav", RECORDS_FOLDER_NAME, i);
    // Create if does not exist, do not open existing, write, sync after write
    if (!SD.exists(filename)) {
      break;
    }
  }

  Serial.print("filename ");
  Serial.println(filename);
  frec = SD.open(filename, FILE_WRITE);
  Serial.println("Opened file !");
  if (frec) {
    Serial.print("Recording to ");
    Serial.println(filename);
    queue1.begin();
    phoneMode = Mode::Recording;
    print_mode();
    recByteSaved = 0L;
  } else {
    Serial.println("Couldn't open file to record!");
  }
}

void PhoneGuestBook::print_mode(void) {  // only for debugging
  Serial.print("Bascule en mode: ");
  // Initialising, Ready, Prompting, Recording, Playing
  if (phoneMode == Mode::Ready) Serial.println(" En veille");
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
  this->writeOutHeader();
  // Close the file
  frec.close();
  Serial.println("Closed file");
  phoneMode = Mode::Ready;
  print_mode();
  //Serial.println("stopRecording");
  digitalWrite(PIN_LED, LOW);
}


void PhoneGuestBook::startPlayingRandomAudio() {
  // Find the first available file number
  int counter = 0;
  for (int i = 0; i < 9999; i++) {
    snprintf(filename, 18, "%s%05d.wav", RECORDS_FOLDER_NAME, i);
    if (!SD.exists(filename)) {
      counter = i;
      break;
    }
  }

  
  snprintf(filename, 18, "%s%05d.wav", RECORDS_FOLDER_NAME, (int)random(0, counter));
  Serial.println(filename);
  if (guestbook.hasAnAudioBeenPlayedBefore) {
    delay(DELAY_BETWEEN_PLAYS);
  }
  guestbook.hasAnAudioBeenPlayedBefore = true;
  playWav1.play(filename);
}

void PhoneGuestBook::end_Beep(void) {
  waveform.frequency(523.25);
  waveform.amplitude(beep_volume);
  wait(250);
  waveform.amplitude(0);
  wait(250);
  waveform.amplitude(beep_volume);
  wait(250);
  waveform.amplitude(0);
  wait(250);
  waveform.amplitude(beep_volume);
  wait(250);
  waveform.amplitude(0);
  wait(250);
  waveform.amplitude(beep_volume);
  wait(250);
  waveform.amplitude(0);
}

void PhoneGuestBook::wait(unsigned int milliseconds) {
  elapsedMillis msec = 0;

  while (msec <= milliseconds) {
    // buttonRecord.update();
    // buttonPlay.update();
    // if (buttonRecord.fallingEdge()) Serial.println("Button (pin 0) Press");
    // if (buttonPlay.fallingEdge()) Serial.println("Button (pin 1) Press");
    // if (buttonRecord.risingEdge()) Serial.println("Button (pin 0) Release");
    // if (buttonPlay.risingEdge()) Serial.println("Button (pin 1) Release");
  }
}

void PhoneGuestBook::updateButtons() {
  buttonRecord.update();
  buttonReset.update();
}

