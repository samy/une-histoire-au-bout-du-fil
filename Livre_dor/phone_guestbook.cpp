#include "synth_waveform.h"
#include "FatLib/FatFile.h"
#include "core_pins.h"
#include "phone_guestbook.h"
/* Récupération de l'état de décroché/raccroché */
bool isHangedUp() {
  return 1 == digitalRead(PIN_HANG);
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

void PhoneGuestBook::enableIntroBeforeRecord() {
  this->introRecordEnabled = true;
}

void PhoneGuestBook::enableIntroBeforePlay() {
  this->introPlayEnabled = true;
}

void PhoneGuestBook::enableRecordMode() {
  digitalWrite(PIN_LED, LOW);
  setMode(RECORD_MODE);
}

int PhoneGuestBook::getMode() {
  return this->phoneMode;
}

void PhoneGuestBook::setMode(int mode) {
  this->phoneMode = mode;
}

void PhoneGuestBook::writeOutHeader(File audioFile) {  // update WAV header with final filesize/datasize

  //  NumSamples = (recByteSaved*8)/bitsPerSample/numChannels;
  //  Subchunk2Size = NumSamples*numChannels*bitsPerSample/8; // number of samples x number of channels x number of bytes per sample
  Subchunk2Size = recByteSaved;
  ChunkSize = Subchunk2Size + 36;
  audioFile.seek(0);
  audioFile.write("RIFF");
  byte1 = ChunkSize & 0xff;
  byte2 = (ChunkSize >> 8) & 0xff;
  byte3 = (ChunkSize >> 16) & 0xff;
  byte4 = (ChunkSize >> 24) & 0xff;
  audioFile.write(byte1);
  audioFile.write(byte2);
  audioFile.write(byte3);
  audioFile.write(byte4);
  audioFile.write("WAVE");
  audioFile.write("fmt ");
  byte1 = Subchunk1Size & 0xff;
  byte2 = (Subchunk1Size >> 8) & 0xff;
  byte3 = (Subchunk1Size >> 16) & 0xff;
  byte4 = (Subchunk1Size >> 24) & 0xff;
  audioFile.write(byte1);
  audioFile.write(byte2);
  audioFile.write(byte3);
  audioFile.write(byte4);
  byte1 = AudioFormat & 0xff;
  byte2 = (AudioFormat >> 8) & 0xff;
  audioFile.write(byte1);
  audioFile.write(byte2);
  byte1 = numChannels & 0xff;
  byte2 = (numChannels >> 8) & 0xff;
  audioFile.write(byte1);
  audioFile.write(byte2);
  byte1 = sampleRate & 0xff;
  byte2 = (sampleRate >> 8) & 0xff;
  byte3 = (sampleRate >> 16) & 0xff;
  byte4 = (sampleRate >> 24) & 0xff;
  audioFile.write(byte1);
  audioFile.write(byte2);
  audioFile.write(byte3);
  audioFile.write(byte4);
  byte1 = byteRate & 0xff;
  byte2 = (byteRate >> 8) & 0xff;
  byte3 = (byteRate >> 16) & 0xff;
  byte4 = (byteRate >> 24) & 0xff;
  audioFile.write(byte1);
  audioFile.write(byte2);
  audioFile.write(byte3);
  audioFile.write(byte4);
  byte1 = blockAlign & 0xff;
  byte2 = (blockAlign >> 8) & 0xff;
  audioFile.write(byte1);
  audioFile.write(byte2);
  byte1 = bitsPerSample & 0xff;
  byte2 = (bitsPerSample >> 8) & 0xff;
  audioFile.write(byte1);
  audioFile.write(byte2);
  audioFile.write("data");
  byte1 = Subchunk2Size & 0xff;
  byte2 = (Subchunk2Size >> 8) & 0xff;
  byte3 = (Subchunk2Size >> 16) & 0xff;
  byte4 = (Subchunk2Size >> 24) & 0xff;
  audioFile.write(byte1);
  audioFile.write(byte2);
  audioFile.write(byte3);
  audioFile.write(byte4);
  audioFile.close();
  Serial.println("header written");
  Serial.print("Subchunk2: ");
  Serial.println(Subchunk2Size);
}

SdFat32 sd;
File32 file;

AudioSynthWaveform waveform;  //xy=404,310
AudioPlaySdWav playSdWav;     //xy=412,441
AudioMixer4 mixer;            //xy=752,329
AudioOutputI2S i2s1;          //xy=1048,323
AudioConnection patchCord1(waveform, 0, i2s1, 0);
AudioConnection patchCord2(playSdWav, 0, mixer, 1);
AudioConnection patchCord3(playSdWav, 1, mixer, 2);
AudioConnection patchCord4(mixer, 0, i2s1, 0);
AudioConnection patchCord5(mixer, 0, i2s1, 1);

PhoneGuestBook guestbook;
char line[40];
char recordsNumber[10];
char tmpContent;
int phoneStatus = 0;
// The file where data is recorded
File frec;
AudioRecordQueue queue1;  //xy=281,63
int mode = 0;             // 0=stopped, 1=recording, 2=playing
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
const int myInput = AUDIO_INPUT_MIC;




void PhoneGuestBook::startPlaying() {
  Serial.println("startPlaying");
  playSdWav.play("RECORD.RAW");
  mode = 2;
}

void PhoneGuestBook::playIntro() {
  guestbook.stopPlaying();
  if (this->getMode() == RECORD_MODE && this->introRecordEnabled) {
    if (sd.exists(RECORD_INTRO_FILENAME)) {
      playSdWav.play(RECORD_INTRO_FILENAME);

      introHasBeenPlayed = true;
    } else {
      Serial.println("Record intro file not found.");
    }
  }
  if (this->getMode() == PLAY_MODE && this->introPlayEnabled) {
    if (sd.exists(PLAY_INTRO_FILENAME)) {
      playSdWav.play(PLAY_INTRO_FILENAME);
      introHasBeenPlayed = true;
    } else {
      Serial.println("Record intro file not found.");
    }
  }
  if (!introHasBeenPlayed && (this->introPlayEnabled || this->introRecordEnabled) && sd.exists(COMMON_INTRO_FILENAME)) {
    playSdWav.play(COMMON_INTRO_FILENAME);
    introHasBeenPlayed = true;
  }

  mode = 2;
}

void PhoneGuestBook::playBeep() {
  waveform.begin(WAVEFORM_SINE);
  waveform.amplitude(0.75);
  
  delay(400);
  waveform.amplitude(0);
}




void PhoneGuestBook::startRecording() {
  Serial.println("startRecording");
  if (SD.exists("RECORD.RAW")) {
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    SD.remove("RECORD.RAW");
  }
  frec = SD.open("RECORD.RAW", FILE_WRITE);
  if (frec) {
    queue1.begin();
    mode = 1;
    recByteSaved = 0L;
  }
}

void PhoneGuestBook::continueRecording() {
  if (queue1.available() >= 2) {
    byte buffer[512];
    // Fetch 2 blocks from the audio library and copy
    // into a 512 byte buffer.  The Arduino SD library
    // is most efficient when full 512 byte sector size
    // writes are used.
    memcpy(buffer, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    memcpy(buffer + 256, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    // write all 512 bytes to the SD card
    //elapsedMicros usec = 0;
    frec.write(buffer, 512);
    // Uncomment these lines to see how long SD writes
    // are taking.  A pair of audio blocks arrives every
    // 5802 microseconds, so hopefully most of the writes
    // take well under 5802 us.  Some will take more, as
    // the SD library also must write to the FAT tables
    // and the SD card controller manages media erase and
    // wear leveling.  The queue1 object can buffer
    // approximately 301700 us of audio, to allow time
    // for occasional high SD card latency, as long as
    // the average write time is under 5802 us.
    //Serial.print("SD write, us=");
    //Serial.println(usec);
    recByteSaved += 512;
  }
}

void PhoneGuestBook::stopRecording() {
  Serial.println("stopRecording");
  queue1.end();
  if (mode == 1) {
    while (queue1.available() > 0) {
      frec.write((byte*)queue1.readBuffer(), 256);
      queue1.freeBuffer();
      recByteSaved += 256;
    }
    guestbook.writeOutHeader(frec);

    frec.close();
  }
  mode = 0;
}




void PhoneGuestBook::continuePlaying() {
  if (!playSdWav.isPlaying()) {
    playSdWav.stop();
    mode = 0;
  }
}

void PhoneGuestBook::stopPlaying() {
  Serial.println("stopPlaying");
  if (mode == 2) playSdWav.stop();
  mode = 0;
}