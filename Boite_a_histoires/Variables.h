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

/* Déclaration des variables */
#if (IS_RP2040)
#define mySoftwareSerial Serial1
#else
SoftwareSerial mySoftwareSerial(9, 10);  // RX, TX
#endif

DFRobotDFPlayerMini myDFPlayer;
int numberSpecified = -1;
unsigned long timeSinceLastIntroPlay = 0;
int phoneStatus = 0;
int audioFilesCount = 0;
bool currentState = 0;

bool isFirstPlaySinceHangUp = true;

int dialedNumbers[DIALED_NUMBERS_MAX];
int dialedIndex = 0;

int numberDialed = -1;
int finalDialedNumber = 0;


#ifndef STORAGE_DEVICE
#define STORAGE_DEVICE 1 //2 pour SD, 1 pour USB
#endif

#ifndef VOLUME_HANDLING
#define VOLUME_HANDLING false //A activer pour avoir la gestion du volume sur le PIN A1
#endif

#ifndef USE_KEYPAD
#define USE_KEYPAD false //A activer pour gérer un téléphone S63 à touches
#endif

#define DIALER_TYPE_ROTARY "rotary"
#define DIALER_TYPE_KEYPAD "keypad"