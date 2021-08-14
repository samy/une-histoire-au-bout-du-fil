#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

/* Boutons */
const int buttonCount = 10;       //Nombre de boutons
int buttonStates[buttonCount];    //Etat des boutons
int lastButtonPressedIndex;       //Numéro du dernier bouton pressé
int buttonsPins[] = { 2, 3, 4 };  //Liste des ports sur lesquels sont branchés les boutons
bool isAButtonPressed = false;
bool isADifferentButtonPressed = false;
int MP3_PIN1 = DX; //Le port de l'Ardunino branché sur le TX du DFPlayer
int MP3_PIN2 = DY; //Le port de l'Ardunino branché sur le RX du DFPlayer
SoftwareSerial mySoftwareSerial(MP3_PIN1, MP3_PIN2);
DFRobotDFPlayerMini mp3Player;

void setup() {
  initializeButtons();
  checkButtonStates();

  //Initialisation du lecteur MP3
  mySoftwareSerial.begin(9600) ;
  mp3Player.begin(mySoftwareSerial) ;
}

void loop() {}

/* 
- On initialise tous les états de boutons à "LOW" (non enfoncés)
- On écoute désormais tous les 10 ports reliés aux boutons
*/

void initializeButtons() {
  for (int i = 0; i < buttonCount; i++) {
    buttonStates[i] = LOW;
    // On utilise ici INPUT_PULLUP car on est dans une configuration 2 fils, et pas 3 comme un bouton poussoir classique
    // https://www.arduino.cc/en/Tutorial/Foundations/DigitalPins
    pinMode(buttonsPins[i], INPUT_PULLUP);
  }
}

void checkButtonStates() {
  isAButtonPressed = false;
  isADifferentButtonPressed = false;
  for (int i = 0; i < buttonCount; i++) {
    if (digitalRead(i) == LOW) { //Dans le cas de INPUT_PULLUP, c'est la valeur LOW qui indique que le port est alimenté
      lastButtonPressedIndex = i;
      isAButtonPressed = true;
      //Inutile de retester 60 fois / seconde si un bouton est enfoncé :)
      delay(100);
      if (lastButtonPressedIndex != i || lastButtonPressedIndex == 0) {
        isADifferentButtonPressed = true;
      }
    }
  }

  //Si un bouton a été pressé
  if (isAButtonPressed) {

      //Si c'est un autre bouton que celui d'avant, on arrête la lecture seulement si elle est en cours
      //Valable aussi au premier bouton
      if (isADifferentButtonPressed) {
          if (-1 != mp3player.readState()) {
            mp3Player.pause();
          }
          mp3Player.play(i); //On joue le morçeau associé (entre 0 et 9)
      }
  }
  
  //Si aucun bouton n'est actuellement appuyé
  if (!isAButtonPressed) {
    //On stocke toute lecture de MP3
    mp3Player.pause();
  }
}