
/* Bibliothèques requises */
#include <SoftwareSerial.h>      /* Connexion série */
#include "DFRobotDFPlayerMini.h" /* Lecteur MP3 */
#include "RotaryDialer.h"        /* Gestion du cadran rotatif */

/* Définition des constantes */
// Cadran FR
#define PIN_READY A2
#define PIN_PULSE 6
#define PIN_HANG A3

// Cadran UK
//#define PIN_READY 1
//#define PIN_PULSE 2
//#define PIN_HANG 3

/* Fonctionnalités */
#define INTRO_ENABLE false         /* Pour activer le message au décrochage */
#define INTRO_DELTA 20 * 3600 * 24 /* Temps minimal en secondes entre deux diffusions du message de décrochage */
#define DIAL_RANDOM true           /* Si le cadran doit lire au hasard */
#define DIALER_TYPE "FR"           /* FR pour cadrans français, UK pour britanniques */

/* Gestion bouton supplémentaire */
#define EXTRA_HANG false         /* Si le cadran doit lire au hasard */
#define EXTRA_HANG_PIN A5        /* Si le cadran doit lire au hasard */
#define EXTRA_HANG_REVERSE false /* Si le cadran doit lire au hasard */

#include "Variables.h"

void setup() {

  /* Connexion série pour la remontée d'informations au PC */
  Serial.begin(9600);
  /* Connexion série pour la communication avec le DFPlayer */
  mySoftwareSerial.begin(9600);

  /* Initiation de la gestion du cadran rotatif */
  dialer.setup();

  /* Connexion au DFPlayer */
  if (!myDFPlayer.begin(mySoftwareSerial, true, false)) {  //Use softwareSerial to communicate with mp3.
    Serial.println("bad");
    while (true)
      ;
  }
  Serial.println("OK");

  /* Etat initial du DFPlayer */
  myDFPlayer.pause();
  myDFPlayer.volume(10);

  /* On écoute le décrochage sur le PIN indiqué */
  pinMode(PIN_HANG, INPUT_PULLUP);
  if (EXTRA_HANG) {
    pinMode(EXTRA_HANG_PIN, INPUT_PULLUP);
  }

  randomSeed(analogRead(0));
  audioFilesCount = myDFPlayer.readFileCounts();
}

void loop() {
  /* Si le téléphone est raccroché, on stoppe la lecture du MP3 (il n'a pas de véritable stop() et on passe à l'itération suivante */
  if (isHangedUp() || (EXTRA_HANG && isExtraHangedUp())) {
    myDFPlayer.pause();
    //Serial.println("Raccroche");
    phoneStatus = 0;
    return;
  } else {
    if (phoneStatus == 0) {
      phoneStatus = 1;
    }
  }

  if (timeSinceLastIntroPlay == 0 || (phoneStatus == 1 && needToPlayIntro())) {
    phoneStatus = 2;
    playIntro();
  }
  phoneStatus = 2;

  /* Si un numéro a été composé sur le téléphone, on le stocke */
  if (strcmp(DIALER_TYPE, "FR") == 0) {
    if (dialer.update()) {
      Serial.println("numero");

      numberSpecified = getDialedNumber(dialer);
    }
  }
  if (strcmp(DIALER_TYPE, "UK") == 0) {
    numberSpecified = getUkDialerNumber();
  }


  /* Si un numéro a été composé, alors on joue le MP3 correspondant */
  if (numberSpecified != -1) {
    Serial.print(numberSpecified);
    myDFPlayer.pause();
    if (isFirstPlaySinceHangUp) {
      delay(1000);
      isFirstPlaySinceHangUp = false;
    }
    if (DIAL_RANDOM) {
      myDFPlayer.play(random(1, audioFilesCount + 1)); //We need to add 1 to let the last audio played
    } else {
      Serial.println(numberSpecified);
      myDFPlayer.playMp3Folder(numberSpecified);
    }
    numberSpecified = -1;
  }
}

/* Récupération du numéro composé (le DFPlayer démarre à 1, donc le 0 est converti en 10) */
int getDialedNumber(RotaryDialer dialerObject) {
  int number = dialer.getNextNumber();
  return number == 0 ? 10 : number;
}

/* Récupération de l'état de décroché/raccroché */
bool isHangedUp() {
  return 0 == digitalRead(PIN_HANG);
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
  int numberSpecified = -1;
  if ((millis() - lastStateChangeTime) > dialHasFinishedRotatingAfterMs) {
    // the dial isn't being dialed, or has just finished being dialed.
    if (needToPrint) {
      // if it's only just finished being dialed, we need to send the number down the serial
      // line and reset the count. We mod the count by 10 because '0' will send 10 pulses.
      numberSpecified = count % 10;
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
  return numberSpecified == 0 ? 10 : numberSpecified;
}
