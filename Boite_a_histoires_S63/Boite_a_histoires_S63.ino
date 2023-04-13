/* Bibliothèques requises */
#include <SoftwareSerial.h> /* Connexion série */
#include <DFRobotDFPlayerMini.h> /* Lecteur MP3 */
#include "RotaryDialer.h" /* Gestion du cadran rotatif */

/* Définition des constantes */
#define PIN_READY A2
#define PIN_PULSE 6
#define PIN_HANG A3

/* Déclaration des variables */
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
int numberSpecified = -1;
RotaryDialer dialer = RotaryDialer(PIN_READY, PIN_PULSE);

void setup() {
  /* Connexion série pour la remontée d'informations au PC */
  Serial.begin(9600);

  /* Connexion série pour la communication avec le DFPlayer */
  mySoftwareSerial.begin(9600);

  /* Initiation de la gestion du cadran rotatif */
  dialer.setup();

  /* Connexion au DFPlayer */
  if (!myDFPlayer.begin(mySoftwareSerial, true, false)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to play"));
    while (true);
  }

  /* Etat initial du DFPlayer */
  myDFPlayer.pause();
  myDFPlayer.volume(12);

  /* On écoute le décrochage sur le PIN indiqué */
  pinMode(PIN_HANG, INPUT_PULLUP);
}

void loop() {
  /* Si le téléphone est raccroché, on stoppe la lecture du MP3 (il n'a pas de véritable stop() et on passe à l'itération suivante */
  if (isHangedUp()) {
    myDFPlayer.pause();
    return;
  }

  /* Si un numéro a été composé sur le téléphone, on le stocke */
  if (dialer.update()) {
    numberSpecified = getDialedNumber(dialer);
        Serial.println(numberSpecified);

  }

  /* Si un numéro a été composé, alors on joue le MP3 correspondant */
  if (numberSpecified != -1) {
    // les mp3 doivent se trouver un dossier /MP3 et numérotés 0001.mp3, 0002.mp3, 0003.mp3 ...
    myDFPlayer.playMp3Folder(numberSpecified);
    numberSpecified = -1;
  }
}

/* Récupération du numéro composé (le DFPlayer démarre à 1, donc le 0 est converti en 10) */
int getDialedNumber(RotaryDialer dialerObject) {
  int number = dialer.getNextNumber();
  return number == 0 ? 10 : number;
}

/** Récupération de l'état de décroché/raccroché
  La composition du numéro ayant tendance à parasiter le circuit
  on détecte ici des valeurs proches de 1023 qui correspondent à un vrai raccrochage)
*/
bool isHangedUp() {
  int sensorValue = analogRead(PIN_HANG);
  return sensorValue > 1000;
}
