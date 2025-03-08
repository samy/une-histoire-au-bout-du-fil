/* Constantes pour cadran UK */
int needToPrint = 0;
int count;
int lastState = LOW;
int trueState = LOW;
long unsigned int lastStateChangeTime = 0;
int cleared = 0;
long unsigned int dialHasFinishedRotatingAfterMs = 100;
long unsigned int debounceDelay = 10;

int volume = 0;
int lastVolume = -1;

#define mySoftwareSerial Serial1

DFRobotDFPlayerMini myDFPlayer;
int numberSpecified = -1;
unsigned long timeSinceLastIntroPlay = 0;
int phoneStatus = 0;
int audioFilesCount = 0;
bool currentState = 0;

bool isFirstPlaySinceHangUp = true;

int dialedNumbers[DIALED_NUMBERS_MAX] = {};
int dialedIndex = 0;

int numberDialed = -1;
int finalDialedNumber = 0;

// Cadran FR
#define PIN_PULSE D10
#define PIN_HANG D3

// Cadran UK
//#define PIN_PULSE 2
//#define PIN_HANG 3

#ifndef STORAGE_DEVICE
#define STORAGE_DEVICE 1  //2 pour SD, 1 pour USB
#endif

#ifndef VOLUME_HANDLING
#define VOLUME_HANDLING false  //A activer pour avoir la gestion du volume sur le PIN A1
#endif

#ifndef USE_KEYPAD
#define USE_KEYPAD false  //A activer pour gérer un téléphone S63 à touches
#endif

#ifndef KEYPAD_ROWS_NUMBER
#define KEYPAD_ROWS_NUMBER 4
#endif

#ifndef KEYPAD_COLS_NUMBER
#define KEYPAD_COLS_NUMBER 4
#endif

#define DIALER_TYPE_ROTARY "rotary"
#define DIALER_TYPE_KEYPAD "keypad"

#ifndef DIALER_TYPE
#define DIALER_TYPE DIALER_TYPE_ROTARY
#endif

#ifndef RANDOM_PLAY_ON_HANG
#define RANDOM_PLAY_ON_HANG false
#endif

#ifndef RANDOM_PLAY_ON_HANG_START_ON_TRACK
#define RANDOM_PLAY_ON_HANG_START_ON_TRACK -1
#endif

#ifndef MAX_VOLUME
#define MAX_VOLUME 30
#endif

#define I2CADDR 0x20


const byte ROWS = KEYPAD_ROWS_NUMBER;  //four rows
const byte COLS = KEYPAD_COLS_NUMBER;  //three columns
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 7, 6, 5, 4 };  //connect to the row pinouts of the keypad
byte colPins[COLS] = { 3, 2, 1, 0 };  //connect to the column pinouts of the keypad