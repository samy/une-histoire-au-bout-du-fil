
/* Bibliothèques requises */
#include <SoftwareSerial.h> /* Connexion série */
#include <DFRobotDFPlayerMini.h> /* Lecteur MP3 */

/* Définition des constantes */
#define PIN_HANG A3

/* Déclaration des variables */
SoftwareSerial mySoftwareSerial(9, 10); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
bool currentState = 0;

void setup() {
  /* Connexion série pour la remontée d'informations au PC */
  Serial.begin(9600);

  /* Connexion série pour la communication avec le DFPlayer */
  mySoftwareSerial.begin(9600);

  /* Connexion au DFPlayer */
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to play"));
    while (true);
  }

  /* Etat initial du DFPlayer */
  myDFPlayer.volume(12);
  myDFPlayer.enableLoopAll(); //loop all mp3 files.
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);

  /* On écoute le décrochage sur le PIN indiqué */
  pinMode(PIN_HANG, INPUT_PULLUP);

}

void loop() {
  /* Si le téléphone est raccroché, on stoppe la lecture du MP3 (il n'a pas de véritable stop() et on passe à l'itération suivante */
  if (isHangedUp()) {
    myDFPlayer.pause();
    currentState = 0;

  } else {
    if (0 == currentState) {
      myDFPlayer.next();
      currentState = 1;
    }
  }
}


/** Récupération de l'état de décroché/raccroché
  La composition du numéro ayant tendance à parasiter le circuit
  on détecte ici des valeurs proches de 1023 qui correspondent à un vrai raccrochage)
*/
bool isHangedUp() {
  return 1 == digitalRead(PIN_HANG);
}
