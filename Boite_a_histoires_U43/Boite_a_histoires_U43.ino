#ifdef USE_TINYUSB
#include <Adafruit_TinyUSB.h>
#endif

/* Bibliothèques requises */
#include <SoftwareSerial.h>      /* Connexion série */
#include "DFRobotDFPlayerMini.h" /* Lecteur MP3 */
#include "RotaryDialer.h"        /* Gestion du cadran rotatif */

/* Définition des constantes */
// Cadran FR
#define PIN_READY A2
#define PIN_PULSE 6
#define PIN_HANG A3

#define PIN_LED_INFO 1

// Cadran UK
//#define PIN_READY 1
//#define PIN_PULSE 2
//#define PIN_HANG 3

/* Fonctionnalités */
#define INTRO_ENABLE false /* Pour activer le message au décrochage */
#define LED_ENABLE false
#define INTRO_DELTA 20 * 3600 * 24 /* Temps minimal en secondes entre deux diffusions du message de décrochage */
#define DIAL_RANDOM false          /* Si le cadran doit lire au hasard */
#define DIALER_TYPE "FR"           /* FR pour cadrans français, UK pour britanniques */
#define NO_DIALER false            /* FR pour cadrans français, UK pour britanniques */
#define DIALED_NUMBERS_MAX 2       /* FR pour cadrans français, UK pour britanniques */

#ifdef ARDUINO_ARCH_RP2040
#define IS_RP2040 true
#else
#define IS_RP2040 true /* A faire seulement si le TX DFPlayer est bien branché sur le RX du Xiao */
#endif

/* Gestion bouton supplémentaire */
#define EXTRA_HANG false        /* Si le cadran doit lire au hasard */
#define EXTRA_HANG_PIN A2       /* Si le cadran doit lire au hasard */
#define EXTRA_HANG_REVERSE true /* Si le cadran doit lire au hasard */

#include "Variables.h"

void setup() {
  Serial.begin(9600);
  /* Connexion série pour la remontée d'informations au PC */


  /* Connexion série pour la communication avec le DFPlayer */
  mySoftwareSerial.begin(9600);
  /* Initiation de la gestion du cadran rotatif */
  dialer.setup();

  /* Connexion au DFPlayer */
  if (!myDFPlayer.begin(mySoftwareSerial, true, false)) {  //Use softwareSerial to communicate with mp3.
    if (IS_RP2040) {
      Serial1.println("bad");
    } else {
      Serial.println("bad");
    }
  }
  if (IS_RP2040) {
    Serial1.println("OK");
  } else {
    Serial.println("OK");
  }
  /* Etat initial du DFPlayer */
  myDFPlayer.pause();
  myDFPlayer.volume(7);

  /* On écoute le décrochage sur le PIN indiqué */
  pinMode(PIN_HANG, INPUT_PULLUP);
  if (EXTRA_HANG) {
    pinMode(EXTRA_HANG_PIN, INPUT_PULLUP);
  }

  randomSeed(analogRead(0));
  delay(2000);
  audioFilesCount = myDFPlayer.readFileCounts();
  if (LED_ENABLE) {
    pinMode(PIN_LED_INFO, OUTPUT);
  }
}

void loop() {
  if (myDFPlayer.available()) {
    //
  }
  if (audioFilesCount <= 0) {
    if (IS_RP2040) {
      Serial1.println("Pas de fichiers audio dans le dossier MP3");
    } else {
      Serial.println("Pas de fichiers audio dans le dossier MP3");
    }
    return;
  }

  /* Si le téléphone est raccroché, on stoppe la lecture du MP3 (il n'a pas de véritable stop() et on passe à l'itération suivante */
  if (isHangedUp() || (EXTRA_HANG && isExtraHangedUp())) {
    if (LED_ENABLE) {
      digitalWrite(PIN_LED_INFO, LOW);
    }
    delay(200);
    myDFPlayer.pause();
    phoneStatus = 0;
    dialedIndex = 0;

    return;
  } else {
    if (LED_ENABLE) {
      digitalWrite(PIN_LED_INFO, HIGH);
    }
    if (phoneStatus == 0) {
      dialedIndex = 0;
      phoneStatus = 1;
    }
  }


  if (timeSinceLastIntroPlay == 99999 || (phoneStatus == 1 && needToPlayIntro())) {
    phoneStatus = 2;
    playIntro();
  }
  phoneStatus = 2;

  if (!NO_DIALER) {

    /* Si un numéro a été composé sur le téléphone, on le stocke */
    if (strcmp(DIALER_TYPE, "FR") == 0) {
      if (dialer.update()) {

        Serial.println("test_if_dialer_update");

        numberDialed = getDialedNumber(dialer);

        Serial.print("numberDialed");
        Serial.println(numberDialed);
      }
    }
    if (strcmp(DIALER_TYPE, "UK") == 0) {
      numberDialed = getUkDialerNumber();
    }

    /* Si un numéro a été composé, alors on joue le MP3 correspondant */
    if (numberDialed != -1) {
      Serial.print("dialedIndex");
      Serial.println(dialedIndex);
      dialedIndex++;

      /* We store dialed numbers */
      if (dialedIndex <= DIALED_NUMBERS_MAX) {
        dialedNumbers[dialedIndex] = numberDialed;
        numberDialed = -1;
        if (dialedIndex < DIALED_NUMBERS_MAX) {
          Serial.println("return");

          return;
        }
      }

      if (dialedIndex > DIALED_NUMBERS_MAX) {
        Serial.println("reset");
        dialedIndex = 0;
        numberDialed = -1;
        return;
      }

      for (int i = 1; i <= dialedIndex; i++) {
        finalDialedNumber += pow(10, DIALED_NUMBERS_MAX - i) * dialedNumbers[i];
      }
      dialedIndex = 0;
      myDFPlayer.pause();
      if (isFirstPlaySinceHangUp) {
        delay(1000);
        isFirstPlaySinceHangUp = false;
      }
      if (DIAL_RANDOM) {
        myDFPlayer.play(random(1, audioFilesCount + 1));  //We need to add 1 to let the last audio played
      } else {
        //myDFPlayer.playMp3Folder(finalDialedNumber);
        myDFPlayer.playMp3Folder(1);
      }

      Serial.print("finalDialedNumber");
      Serial.println(finalDialedNumber);
      numberDialed = -1;
      finalDialedNumber = 0;
      return;
    }
  } else {
    /* Si le lecteur n'est pas actuellement en train de jouer */
    if (myDFPlayer.readState() == 514 || myDFPlayer.readState() == 512) {
      delay(1000);

      myDFPlayer.playMp3Folder(random(0, audioFilesCount - 1));
    }
    delay(1000);
  }
}
/* Récupération du numéro composé (le DFPlayer démarre à 1, donc le 0 est converti en 10) */
int getDialedNumber(RotaryDialer dialerObject) {
  int number = dialer.getNextNumber();
  return number;
}

/* Récupération de l'état de décroché/raccroché */
bool isHangedUp() {
  return 1 == digitalRead(PIN_HANG);
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
  if ((millis() - timeSinceLastIntroPlay) > INTRO_DELTA * 1000) {
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

int getUkDialerNumber() {
  int reading = digitalRead(PIN_PULSE);
  int numberDialed = -1;
  if ((millis() - lastStateChangeTime) > dialHasFinishedRotatingAfterMs) {
    // the dial isn't being dialed, or has just finished being dialed.
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
