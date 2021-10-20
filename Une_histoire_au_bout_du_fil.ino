const char string_0[] PROGMEM = "Le petit princeLe petit princeLe petit princeLe petit prince"; // "String 0" etc are strings to store - change to suit.
const char string_1[] PROGMEM = "Le petit princeLe petit princeLe petit princeLe petit prince";
const char string_2[] PROGMEM = "Le petit princeLe petit princeLe petit princeLe petit prince";
const char string_3[] PROGMEM = "Le petit princeLe petit princeLe petit princeLe petit prince";
const char string_4[] PROGMEM = "Le petit princeLe petit princeLe petit princeLe petit prince";
const char string_5[] PROGMEM = "Le petit princeLe petit princeLe petit princeLe petit prince";
const char *const titles[] PROGMEM = {string_0, string_1, string_2, string_3, string_4, string_5};


/* Lecteur MP3 */
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

/* Ecran OLED */
#define OLED_CS A3
#define OLED_DC A2
#include "OLED_Driver.h"
#include "OLED_GUI.h"
#include "DEV_Config.h"
#include "Show_Lib.h"
bool isScreenInitialized = false;

/* Boutons */
const int buttonCount PROGMEM = 10;                              //Nombre de boutons
int buttonStates[buttonCount];                           //Etat des boutons
int lastButtonPressedIndex = -1;                         //Numéro du dernier bouton pressé
const int buttonsPins[] PROGMEM = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };  //Liste des ports sur lesquels sont branchés les boutons
const int powerPin PROGMEM = 12;
bool isAButtonPressed = false;
bool isADifferentButtonPressed = false;
bool areButtonsInitialized = false;
const int MP3_PIN1 PROGMEM = A6;  //Le port de l'Ardunino branché sur le TX du DFPlayer
const int MP3_PIN2 PROGMEM = A5;  //Le port de l'Ardunino branché sur le RX du DFPlayer
SoftwareSerial MP3Serial(MP3_PIN1, MP3_PIN2);
DFRobotDFPlayerMini mp3Player;

void setup() {

  //Initialisation du lecteur MP3
  MP3Serial.begin(9600);
  mp3Player.begin(MP3Serial);
  mp3Player.volume(25);
  char valeur[64]; // Attention : La variable doit être suffisamment grande pour stocker le message
  strcpy_P(valeur, (char*) pgm_read_word(&(titles[2])));
}

void loop() {
  if (!isScreenInitialized) {
    initScreen();
  }
  if (!checkPower()) {
    return;
  }
  if (!areButtonsInitialized) {
    initializeButtons();
  }
  handleButtons();
}

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
  pinMode(powerPin, INPUT_PULLUP);
  areButtonsInitialized = true;
}

/*
  Vérification de l'alimentation du port déclenché par le fait de décrocher le téléphone
*/

bool checkPower() {
  if (HIGH == digitalRead(powerPin)) {  //Si le téléphone est raccroché, on réinitialise tout
    initializeButtons();
    lastButtonPressedIndex = -1;
    mp3Player.pause();
    return false;
  }
  return true;
}

/*
  Fonction principale de gestion des boutons/lecture
*/
void handleButtons() {
  isAButtonPressed = false;
  isADifferentButtonPressed = false;
  for (int i = 0; i < buttonCount; i++) {
    if (digitalRead(buttonsPins[i]) == LOW) {  //Dans le cas de INPUT_PULLUP, c'est la valeur LOW qui indique que le port est alimenté
      isAButtonPressed = true;
      //Inutile de retester 60 fois / seconde si un bouton est enfoncé :)
      if (lastButtonPressedIndex != i || lastButtonPressedIndex == -1) {

        isADifferentButtonPressed = true;
      }
      lastButtonPressedIndex = i;
      delay(1000);

      break;
    }
  }

  //Si un bouton a été pressé
  if (isAButtonPressed) {
    //Si c'est un autre bouton que celui d'avant, on arrête la lecture seulement si elle est en cours
    //Valable aussi au premier bouton
    if (isADifferentButtonPressed) {
      if (-1 != mp3Player.readState()) {
        mp3Player.pause();
      }
      mp3Player.play(lastButtonPressedIndex);  //On joue le morçeau associé (entre 0 et 9)
      delay(3000);
    }
  }

  //Si aucun bouton n'est actuellement appuyé
  if (!isAButtonPressed) {
    //On stocke toute lecture de MP3
    mp3Player.pause();
  }
}

void initScreen() {
  System_Init();

  OLED_SCAN_DIR OLED_ScanDir = SCAN_DIR_DFT;
  OLED_Init( OLED_ScanDir );

  //GUI_Show();
  OLED_ClearBuf();
  OLED_ClearScreen(OLED_BACKGROUND);

  GUI_DisString_EN(0 , 2, "Boite a histoires", &Font16, FONT_BACKGROUND, WHITE);

  OLED_Display(0, 65, 128, 65 + 32);
  OLED_ClearBuf();
  isScreenInitialized = true;

}
