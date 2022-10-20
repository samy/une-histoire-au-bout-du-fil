
// include SPI, MP3 and SD libraries
#include <Adafruit_VS1053.h>
#include "definitions.h"

#define FILE_WRITE_CUSTOM (O_WRITE | O_READ | O_CREAT)


// define the pins used
#define RESET 7      // VS1053 reset pin (output)
#define CS 5        // VS1053 chip select pin (output)
#define DCS 4        // VS1053 Data/command select pin (output)
#define CARDCS 7     // Card chip select pin
#define DREQ 3

#define REC_BUTTON 1
uint32_t fileSize = 0;
uint32_t adpcmBlocks = 0;
uint16_t sampleRate = 8000;
int ch = 2;
static uint8_t recBuf[REC_BUFFER_SIZE];

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(RESET, CS, DCS, DREQ, CARDCS);
#include "patches.h"

File recording;  // the file we will save our recording to
#define RECBUFFSIZE 32  // 64 or 128 bytes.
uint8_t recording_buffer[RECBUFFSIZE];
int dataNeededInBuffer = REC_BUFFER_SIZE;
void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Adafruit VS1053 Ogg Recording Test");

  // initialise the music player
  if (!musicPlayer.begin()) {
    Serial.println("VS1053 not found");
    while (1);  // don't do anything more
  }

  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working

  if (!SD.begin(CARDCS)) {
    Serial.println("SD failed, or not present");
    while (1);  // don't do anything more
  }
  Serial.println("SD OK!");

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(5, 5);

  // when the button is pressed, record!
  pinMode(REC_BUTTON, INPUT);
  digitalWrite(REC_BUTTON, HIGH);


  pinMode(0, OUTPUT); //règle la borne numérique numéro 1 de la carte Arduino en mode sortie
  LoadPlugin(plugin, sizeof(plugin) / sizeof(plugin[0]));

}

uint8_t isRecording = false;

void loop() {
  if (!isRecording && !digitalRead(REC_BUTTON)) {
    // load plugin from SD card! We'll use mono 44.1KHz, high quality
    digitalWrite(0, HIGH); //le courant est envoyé sur la borne 1, la LED  s'allume

    Serial.println("Begin recording");
    isRecording = true;


    // Check if the file exists already
    char filename[15];
    strcpy(filename, "RECORD00.WAV");
    for (uint8_t i = 0; i < 100; i++) {
      filename[6] = '0' + i / 10;
      filename[7] = '0' + i % 10;
      // create if does not exist, do not open existing, write, sync after write
      if (! SD.exists(filename)) {
        break;
      }
    }
    Serial.print("Recording to "); Serial.println(filename);
    recording = SD.open(filename, FILE_WRITE_CUSTOM);
    if (! recording) {
      Serial.println("Couldn't open file to record!");
      while (1);
    }

    if (!prepareRecordWav()) {
      Serial.println("Couldn't prepare Wav"); while (1);
    }
    startRecordWav(true); // use microphone (for linein, pass in 'false')
  }
  if (isRecording) {
    saveRecordedData(isRecording);
  }
  if (isRecording && digitalRead(REC_BUTTON)) {
    digitalWrite(0, LOW); //le courant est envoyé sur la borne 1, la LED  s'allume

    Serial.println("End recording");
    stopRecordWav();
    isRecording = false;
    // flush all the data!
    saveRecordedData(isRecording);
    recording.seek(0);
    Serial.print("fileSize");
    Serial.println(fileSize);
    Set32(pcmHeader + 4, fileSize - 8);
    Set32(pcmHeader + 40, fileSize - 36);
    recording.write(pcmHeader, sizeof(pcmHeader));
    Serial.println("WAV : footer");

    recording.close();
    delay(1000);
    Serial.println("Finished");
  }
}



uint16_t saveRecordedData(boolean isrecord) {
  uint16_t written = 0;

  // read how many words are waiting for us
  int n;

  // try to process 256 words (512 bytes) at a time, for best speed
  if ((n = musicPlayer.recordedWordsWaiting()) > dataNeededInBuffer) {
    int i;
    uint8_t *rbp = recBuf;

    /* Always writes one or two IMA ADPCM block(s) at a time */
    n = dataNeededInBuffer / 2;
    adpcmBlocks += 2;
    for (i = 0; i < n; i++) {
      uint16_t w = musicPlayer.sciRead(SCI_RECDATA);
      *rbp++ = (uint8_t)(w & 0xFF);
      *rbp++ = (uint8_t)(w >> 8);
    }
    recording.write(recBuf, 2 * n);
    fileSize += 2 * n;
  }
}



void LoadPlugin(const uint16_t *d, uint16_t len) {
  int i = 0;

  while (i < len) {
    unsigned short addr, n, val;
    addr = d[i++];
    n = d[i++];
    if (n & 0x8000U) { /* RLE run, replicate n samples */
      n &= 0x7FFF;
      val = d[i++];
      while (n--) {
        musicPlayer.sciWrite(addr, val);
      }
    } else {           /* Copy run, copy n samples */
      while (n--) {
        val = d[i++];
        musicPlayer.sciWrite(addr, val);
      }
    }
  }
}

boolean prepareRecordWav() {
  fileSize = 0;
  adpcmBlocks = 0;
  sampleRate = 8000;
  ch = 1;
  musicPlayer.sciWrite(VS1053_REG_CLOCKF, 0xC000); // set max clock
  delay(1);
  while (!musicPlayer.readyForData())
    ;

  musicPlayer.sciWrite(VS1053_REG_BASS, 0); // clear Bass

  musicPlayer.softReset();
  delay(1);
  while (!musicPlayer.readyForData())
    ;

  musicPlayer.sciWrite(VS1053_SCI_AIADDR, 0);
  // disable all interrupts except SCI
  musicPlayer.sciWrite(VS1053_REG_WRAMADDR, VS1053_INT_ENABLE);
  musicPlayer.sciWrite(VS1053_REG_WRAM, 0x02);
  Serial.println("WAV : headers");
  Set16(pcmHeader + 22, ch);
  Set32(pcmHeader + 24, sampleRate);
  Set32(pcmHeader + 28, 2L * sampleRate * ch);
  Set16(pcmHeader + 32, 2 * ch);
  recording.seek(0);
  recording.write(pcmHeader, sizeof(pcmHeader));
  LoadPlugin(imaFix, sizeof(imaFix) / sizeof(imaFix[0]));

  fileSize += sizeof(pcmHeader);
  return true;
}


void Set32(uint8_t *d, uint32_t  n) {
  int i;
  for (i = 0; i < 4; i++) {
    *d++ = (uint8_t)n;
    n >>= 8;
  }
}

void Set16(uint8_t *d, uint16_t n) {
  int i;
  for (i = 0; i < 2; i++) {
    *d++ = (uint8_t)n;
    n >>= 8;
  }
}
void startRecordWav(boolean mic)
{
  musicPlayer.softReset();
  while (!musicPlayer.readyForData() );
  musicPlayer.sciWrite(VS1053_SCI_AICTRL0, 8000);    //Sample rate between 8000 and 48000
  musicPlayer.sciWrite(VS1053_SCI_AICTRL1, 0);        //Recording gain : 1024 : 1.If 0, use AGC
  musicPlayer.sciWrite(VS1053_SCI_AICTRL2, 4096);     //Maximum AGC level: 1024 = 1. Only used if SCI_AICTRL1 is set to 0.
  //Miscellaneous bits that also must be set before recording.
  musicPlayer.sciWrite(VS1053_SCI_AICTRL3, 6); //joint stereo AGC + IMA ADPCM
  //sciWrite(VS1053_SCI_AICTRL3, 2); //LEFT only
  //sciWrite(VS1053_SCI_AICTRL3, 3); //RIGHT only
  //sciWrite(VS1053_SCI_AICTRL3, 4); //joint stereo AGC + LINEAR PCM
  if (mic)
  {
    musicPlayer.sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_RESET | VS1053_MODE_SM_ADPCM | VS1053_MODE_SM_SDINEW);
  }
  else
  {
    musicPlayer.sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_RESET | VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_ADPCM | VS1053_MODE_SM_SDINEW);
  }

  while (!musicPlayer.readyForData() );
  //  //IMA fix patch
    musicPlayer.sciWrite(VS1053_REG_WRAMADDR, 0x8010);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x3e12);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0xb817);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x3e14);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0xf812);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x3e01);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0xb811);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x0007);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x9717);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x0020);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0xffd2);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x0030);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x11d1);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x3111);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x8024);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x3704);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0xc024);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x3b81);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x8024);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x3101);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x8024);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x3b81);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x8024);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x3f04);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0xc024);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x2808);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x4800);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x36f1);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x9811);
    musicPlayer.sciWrite(VS1053_REG_WRAMADDR, 0x8028);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x2a00);
    musicPlayer.sciWrite(VS1053_REG_WRAM, 0x040e);
}

void stopRecordWav(void) {
  musicPlayer.sciWrite(VS1053_SCI_AICTRL3, 1);
}
