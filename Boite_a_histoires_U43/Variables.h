/* Constantes pour cadran UK */
int needToPrint = 0;
int count;
int lastState = LOW;
int trueState = LOW;
long unsigned int lastStateChangeTime = 0;
int cleared = 0;
long unsigned int dialHasFinishedRotatingAfterMs = 100;
long unsigned int debounceDelay = 10;

/* Déclaration des variables */
SoftwareSerial mySoftwareSerial(9, 10);  // RX, TX
DFRobotDFPlayerMini myDFPlayer;
int numberSpecified = -1;
RotaryDialer dialer = RotaryDialer(PIN_READY, PIN_PULSE);
unsigned long timeSinceLastIntroPlay = 0;
int phoneStatus = 0;
int audioFilesCount = 0;
bool currentState = 0;

bool isFirstPlaySinceHangUp = true;

int dialedNumbers[DIALED_NUMBERS_MAX];
int dialedIndex = 0;

int numberDialed = -1;
int finalDialedNumber = 0;