
/* Boutons */
const int buttonCount = 10;       //Nombre de boutons
int buttonStates[buttonCount];    //Etat des boutons
int lastButtonPressedIndex;       //Numéro du dernier bouton pressé
int buttonsPins[] = { 2, 3, 4 };  //Liste des ports sur lesquels sont branchés les boutons
bool isAButtonPressed = false;

void setup() {
  initializeButtons();
  checkButtonStates();
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
  for (int i = 0; i < buttonCount; i++) {
    if (digitalRead(i) == LOW) { //Dans le cas de INPUT_PULLUP, c'est la valeur LOW qui indique que le port est alimenté
      lastButtonPressedIndex = i;
      isAButtonPressed = true;
      //Inutile de retester 60 fois / seconde si un bouton est enfoncé :)
      delay(100);
    }
  }

  //Si un bouton a été pressé
  if (isAButtonPressed) {

  }
  
  //Si aucun bouton n'est actuellement appuyé
  if (!isAButtonPressed) {

  }
}