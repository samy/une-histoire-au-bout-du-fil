/* Bibliothèques requises */
#include <Bounce2.h>
#include "RotaryDial2.h"
#include <Keypad_MC17.h>
#include <Wire.h>
#include <Keypad.h>
#include "DFRobotDFPlayerMini.h" /* Lecteur MP3 */




// Fonctionnalités
#define INTRO_ENABLE false                     /* Pour activer le message au décrochage  */
#define INTRO_DELTA 1                         /* Temps minimal en secondes entre deux diffusions du message de décrochage */
#define DIAL_RANDOM false                     /* Si le cadran doit lire au hasard */
#define DIALER_ENABLE true                    /* Pour activer le clavier */
#define DIALER_TYPE DIALER_TYPE_KEYPAD        /* Type : DIALER_TYPE_KEYPAD clavier à touches, DIALER_TYPE_ROTARY cadran rotatif */
#define DIALER_COUNTRY "FR"                   /* FR pour cadrans français, UK pour britanniques */
#define DIALED_NUMBERS_MAX 1                  /* Nombre maximal de numéros (1 : 10 chiffres, 2 : 100 chiffres, etc) */
#define STORAGE_DEVICE DFPLAYER_DEVICE_U_DISK /* Support de stockage: DFPLAYER_DEVICE_SD pour SD, DFPLAYER_DEVICE_U_DISK pour clé USB */
#define LED_ENABLE true                       /* Activation de la LED d'indication de fonctionnement */
#define LED_PIN D9                            /* Gestion de la LED */
#define USE_BOUNCE_INSTEAD_OF_DIRECT 0        /* Pour des systèmes de raccrochage un peu sensibles */

#define KEYPAD_REVERSED_MATRIX true

#define RANDOM_PLAY_ON_HANG false            /* Activation du mode Lecture automatique et aléatoire au décrochage de l'appareil */
#define RANDOM_PLAY_ON_HANG_START_ON_TRACK 1 /* En mode aléatoire, le numéro du morceau joué au démarrage */

#define VOLUME_HANDLING false /* Activation de la molette de volume branchée sur le PIN A1 */
#define MAX_VOLUME 30         /* Volume maximal quand la molette est au maximum */

#define HANG_REVERSE false /* Inversion du raccrochage, selon les appareils */

// Gestion bouton supplémentaire
#define EXTRA_HANG false        /* Activation du bouton secondaire pour le raccrochage */
#define EXTRA_HANG_PIN A2       /* PIN du bouton supplémentaire de raccrochage */
#define EXTRA_HANG_REVERSE true /* Si le fonctionnement de ce bouton supplémentaire est inversé */

// Inclusion des valeurs par défaut pour celles qui n'ont pas été personnalisées ci-dessus
#include "Variables.h"

Keypad_MC17 keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR);

// "Bouton" utilisé pour gérer le décrochage
Bounce2::Button button = Bounce2::Button();

void setup() {

  pinMode(PIN_HANG, INPUT_PULLUP);
  button.attach(PIN_HANG, INPUT_PULLUP);
  button.interval(5);
  button.setPressedState(HANG_REVERSE ? 0 : 1);

  /* Optionnel: Gestion du volume */
  if (VOLUME_HANDLING) {
    pinMode(A1, INPUT);
  }

  /* Optionnel: LED d'indication de fonctionnement */
  if (LED_ENABLE) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
  }

  /* Optionnel: 2e dispositif de raccrochage */
  if (EXTRA_HANG) {
    pinMode(EXTRA_HANG_PIN, INPUT_PULLUP);
  }

  /* Connexion série pour la remontée d'informations au PC */
  Serial.begin(9600);

  /* Connexion série pour la communication avec le DFPlayer */
  mySoftwareSerial.begin(9600);

  /* Gestion du clavier */
  if (DIALER_ENABLE) {
    /* Clavier à touches */
    if (DIALER_TYPE == DIALER_TYPE_KEYPAD) {
      Wire.begin();  // now needed
      keypad.begin();
    } else {
      /* Cadran rotatif */
      RotaryDial2::setup(PIN_PULSE, DIALED_NUMBERS_MAX);
    }
  }

  /* Connexion au DFPlayer */
  if (!myDFPlayer.begin(mySoftwareSerial, true, true)) {  //Use softwareSerial to communicate with mp3.
    Serial.println("bad");
    return;
  }
  Serial.println("Starting successfully");

  myDFPlayer.outputDevice(STORAGE_DEVICE);
  myDFPlayer.pause();
  myDFPlayer.volume(9);
  delay(2000);
}

void loop() {
  button.update();

  /* Optionnel: Actualisation du volume */
  if (VOLUME_HANDLING) {
    int volume = getVolume();

    if (lastVolume == -1 || abs(lastVolume - volume) > 30) {
      lastVolume = volume;

      Serial.println(map(volume, 0, 1023, 0, MAX_VOLUME));
      delay(200);
      myDFPlayer.volume(map(volume, 0, 1023, 0, MAX_VOLUME));
    }
  }

  /* Si le téléphone est raccroché, on stoppe la lecture du MP3 (il n'a pas de véritable stop() et on passe à l'itération suivante */
  if (isHangedUp() || (EXTRA_HANG && isExtraHangedUp())) {

    Serial.println("Offline");
    delay(200);
    if (phoneStatus != 0) {
      myDFPlayer.pause();
    }

    phoneStatus = 0;
    dialedIndex = 0;
    return;
  } else {
    if (phoneStatus == 0) {
      myDFPlayer.available();

      delay(300);
      /* Si l'option de lecture aléatoire au décrochage est activée, on joue un audio au hasard
      le changement de phoneStatus évite qu'il soit joué plusieurs fois */
      if (RANDOM_PLAY_ON_HANG) {
        if (RANDOM_PLAY_ON_HANG_START_ON_TRACK != -1) {
          Serial.print("Play on start: track ");
          Serial.println(RANDOM_PLAY_ON_HANG_START_ON_TRACK);
          myDFPlayer.play(RANDOM_PLAY_ON_HANG_START_ON_TRACK);
        } else {
          Serial.println("Random play on start");
          myDFPlayer.randomAll();
        }
      }
      dialedIndex = 0;
      phoneStatus = 1;
    }
  }

  if (timeSinceLastIntroPlay == 99999 || (phoneStatus == 1 && needToPlayIntro())) {
    Serial.println("needToPlayIntro?");
    phoneStatus = 2;
    playIntro();
  }
  phoneStatus = 2;
  if (DIALER_ENABLE) {
    if (DIALER_TYPE == DIALER_TYPE_KEYPAD) {
      char key = keypad.getKey();
      if (key) {
        numberDialed = convertKeypadToInt(key);
      }
    } else {
      /* Si un numéro a été composé sur le téléphone, on le stocke */
      if (strcmp(DIALER_COUNTRY, "FR") == 0) {
        if (RotaryDial2::available()) {

          delay(200);

          numberDialed = RotaryDial2::read();
        }
      }
      if (strcmp(DIALER_COUNTRY, "UK") == 0) {
        numberDialed = getUkRotaryDialerNumber();
      }
    }

    /* Si un numéro a été composé, alors on joue le MP3 correspondant */
    if (numberDialed != -1) {
      dialedIndex++;

      /* Stockage des numéros composés */
      if (dialedIndex <= DIALED_NUMBERS_MAX) {
        dialedNumbers[dialedIndex - 1] = numberDialed;
        numberDialed = -1;
        if (dialedIndex < DIALED_NUMBERS_MAX) {
          return;
        }
      }
      if (dialedIndex > DIALED_NUMBERS_MAX) {
        dialedIndex = 0;
        numberDialed = -1;
        return;
      }
      for (int i = 0; i < dialedIndex; i++) {
        finalDialedNumber += pow(10, DIALED_NUMBERS_MAX - 1 - i) * dialedNumbers[i];
        dialedNumbers[i] = 0;
      }
      dialedIndex = 0;
      myDFPlayer.pause();
      if (DIAL_RANDOM) {
        finalDialedNumber = random(1, audioFilesCount + 1);
        myDFPlayer.available();
        delay(300);
        Serial.print("randomDialedNumber=");
        Serial.println(finalDialedNumber);
        myDFPlayer.randomAll();
      } else {
        Serial.print("finalDialedNumber=");
        Serial.println(finalDialedNumber);
        myDFPlayer.playMp3Folder(finalDialedNumber);
      }

      numberDialed = -1;
      finalDialedNumber = 0;
      return;
    }
  } else {
    /* Si le lecteur n'est pas actuellement en train de jouer */
    if (myDFPlayer.readState() == 514 || myDFPlayer.readState() == 512) {
      delay(1000);

      myDFPlayer.randomAll();
    }
    delay(1000);
  }
}

/* Récupération de l'état de décroché/raccroché */
bool isHangedUp() {
  if (USE_BOUNCE_INSTEAD_OF_DIRECT) {
    return button.pressed();
  }
  return (HANG_REVERSE ? 0 : 1) == digitalRead(PIN_HANG);
}
/* Récupération de l'état de décroché/raccroché */
bool isExtraHangedUp() {
  return (EXTRA_HANG_REVERSE ? 0 : 1) == digitalRead(EXTRA_HANG_PIN);
}

/* Détermination de la nécessité de jouer le message d'intro */
bool needToPlayIntro() {
  if (!INTRO_ENABLE) {
    return false;
  }
  /* Si l'écart entre le compteur de temps actuel et la dernière lecture de l'introduction dépasse l'intervalle qu'on a configuré,
    et que le téléphone était décroché avant
    on joue l'intro */
  if (timeSinceLastIntroPlay == 0 || (millis() - timeSinceLastIntroPlay) > INTRO_DELTA * 1000) {
    return true;
  }
  return false;
}

/* Diffusion du message d'introduction */
void playIntro() {
  timeSinceLastIntroPlay = millis();

  delay(1000);                 /* On laisse aux utilisateurs le temps de décrocher */
  myDFPlayer.playMp3Folder(0); /* L'intro est stockée dans le fichier commençant par 0000 dans le dossier MP3 */
}

int getUkRotaryDialerNumber() {
  int reading = digitalRead(PIN_PULSE);
  int numberDialed = -1;
  if ((millis() - lastStateChangeTime) > dialHasFinishedRotatingAfterMs) {
    if (needToPrint) {
      // if it's only just finished being dialed, we need to send the number down the serial
      // line and reset the count. We mod the count by 10 because '0' will send 10 pulses.
      numberDialed = count % 10;
      needToPrint = 0;
      count = 0;
      cleared = 0;
    }
  }

  if (reading != lastState) {
    lastStateChangeTime = millis();
  }
  if ((millis() - lastStateChangeTime) > debounceDelay) {
    // debounce - this happens once it's stablized
    if (reading != trueState) {
      // this means that the switch has either just gone from closed->open or vice versa.
      trueState = reading;
      if (trueState == HIGH) {
        // increment the count of pulses if it's gone high.
        count++;
        needToPrint = 1;  // we'll need to print this number (once the dial has finished rotating)
      }
    }
  }
  lastState = reading;
  return numberDialed;
}

long getVolume() {
  return analogRead(A1);
}
int convertKeypadToInt(char key) {
  switch (key) {
    case '1':
      return 1;
      break;
    case '2':
      return 2;
      break;
    case '3':
      return 3;
      break;
    case '4':
      return 4;
      break;
    case '5':
      return 5;
      break;
    case '6':
      return 6;
      break;
    case '7':
      return 7;
      break;
    case '8':
      return 8;
      break;
    case '9':
      return 9;
      break;
    default:
      return 0;
      break;
  }
}