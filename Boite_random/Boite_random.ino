
/* Bibliothèques requises */
#include <SoftwareSerial.h> /* Connexion série */
#include <DFRobotDFPlayerMini.h> /* Lecteur MP3 */

/* Définition des constantes */
#define PIN_READY  A2
#define PIN_PULSE 6
#define PIN_HANG A3

/* Déclaration des variables */
SoftwareSerial mySoftwareSerial(9, 10); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

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
  myDFPlayer.pause();
  myDFPlayer.volume(15);

  /* On écoute le décrochage sur le PIN indiqué */
  pinMode(PIN_HANG, INPUT_PULLUP);
}

void loop() {
  /* Si le téléphone est raccroché, on stoppe la lecture du MP3 (il n'a pas de véritable stop() et on passe à l'itération suivante */
  if (isHangedUp()) {
    myDFPlayer.pause();
    return;
  } else {
    myDFPlayer.randomAll();
  }
}


/** Récupération de l'état de décroché/raccroché
  La composition du numéro ayant tendance à parasiter le circuit
  on détecte ici des valeurs proches de 1023 qui correspondent à un vrai raccrochage)
*/
bool isHangedUp() {
  return 1 == digitalRead(PIN_HANG);
}
