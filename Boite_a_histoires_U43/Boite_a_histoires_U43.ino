
/* Bibliothèques requises */
#include <SoftwareSerial.h> /* Connexion série */
#include <DFRobotDFPlayerMini.h> /* Lecteur MP3 */
#include "RotaryDialer.h" /* Gestion du cadran rotatif */

/* Définition des constantes */
#define PIN_READY  A2
#define PIN_PULSE 6
#define PIN_HANG A3

/* Fonctionnalités */
#define INTRO_ENABLE true /* Pour activer le message au décrochage */
#define INTRO_DELTA 20 /* Temps minimal en secondes entre deux diffusions du message de décrochage */

/* Déclaration des variables */
SoftwareSerial mySoftwareSerial(9, 10); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
int numberSpecified = -1;
RotaryDialer dialer = RotaryDialer(PIN_READY, PIN_PULSE);
unsigned long timeSinceLastIntroPlay = 0;
int phoneStatus = 0;

void setup() {

  /* Connexion série pour la remontée d'informations au PC */
  Serial.begin(9600);

  /* Connexion série pour la communication avec le DFPlayer */
  mySoftwareSerial.begin(9600);

  /* Initiation de la gestion du cadran rotatif */
  dialer.setup();

  /* Connexion au DFPlayer */
  if (!myDFPlayer.begin(mySoftwareSerial, true, false)) {  //Use softwareSerial to communicate with mp3.
    while (true);
  }

  /* Etat initial du DFPlayer */
  myDFPlayer.pause();
  myDFPlayer.volume(8);

  /* On écoute le décrochage sur le PIN indiqué */
  pinMode(PIN_HANG, INPUT_PULLUP);
}

void loop() {

  /* Si le téléphone est raccroché, on stoppe la lecture du MP3 (il n'a pas de véritable stop() et on passe à l'itération suivante */
  if (isHangedUp()) {
    myDFPlayer.pause();

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
  if (dialer.update()) {
    numberSpecified = getDialedNumber(dialer);
  }

  /* Si un numéro a été composé, alors on joue le MP3 correspondant */
  if (numberSpecified != -1) {
    myDFPlayer.playMp3Folder(numberSpecified);
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
  return 1 == digitalRead(PIN_HANG);
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

  delay(1000); /* On laisse aux utilisateurs le temps de décrocher */
  myDFPlayer.playMp3Folder(0); /* L'intro est stockée dans le fichier commençant par 0000 dans le dossier MP3 */
}
