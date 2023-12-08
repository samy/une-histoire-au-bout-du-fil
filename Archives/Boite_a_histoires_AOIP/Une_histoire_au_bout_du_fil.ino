/* Liste des textes */
#include "ListeHistoires.h"

/* Lecteur MP3 */
#include <SoftwareSerial.h>
#include "src/DFRobotDFPlayerMini/DFRobotDFPlayerMini.h"

/* Ecran OLED */
#define OLED_CS A3
#define OLED_DC A2
#include "src/OLED/OLED_Driver.h"
#include "src/OLED/OLED_GUI.h"
#include "src/OLED/DEV_Config.h"
#include "src/OLED/Show_Lib.h"
bool isScreenInitialized = false;

/* Boutons */
const int buttonCount  = 10;                              //Nombre de boutons
int buttonStates[buttonCount];                           //Etat des boutons
int lastButtonPressedIndex = -1;                         //Numéro du dernier bouton pressé
const int buttonsPins[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };  //Liste des ports sur lesquels sont branchés les boutons
const int powerPin = 12;
bool isAButtonPressed = false;
bool isADifferentButtonPressed = false;
bool areButtonsInitialized = false;
const int MP3_PIN1  = A6;  //Le port de l'Ardunino branché sur le TX du DFPlayer
const int MP3_PIN2  = A5;  //Le port de l'Ardunino branché sur le RX du DFPlayer
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

  if (isScreenInitialized == false) {
    initScreen();
  }
  if (!checkPower()) {
    Serial.println("power off");
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

  Serial.println("handleButtons");
  isAButtonPressed = false;
  isADifferentButtonPressed = false;
  for (int i = 0; i < buttonCount; i++) {
    if (digitalRead(buttonsPins[i]) == LOW) {  //Dans le cas de INPUT_PULLUP, c'est la valeur LOW qui indique que le port est alimenté
      char message[32];  // max length you’ll need +1
      sprintf(message, "Bouton appuye : %d" , i);
      Serial.println(message);
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
extern OLED_DIS sOLED_DIS;

void initScreen() {
  Serial.println("ecran");
  System_Init();
  OLED_SCAN_DIR OLED_ScanDir = SCAN_DIR_DFT;
  OLED_Init( OLED_ScanDir );

  displayHeader();
  displayHomePage();

  isScreenInitialized = true;

}

void displayHeader()
{
  //GUI_Show();
  OLED_ClearBuf();
  OLED_ClearScreen(OLED_BACKGROUND);

  GUI_DrawLine(0, 8, sOLED_DIS.OLED_Dis_Column - 1, 8, WHITE, LINE_SOLID , DOT_PIXEL_DFT);
  OLED_DisPage(0, 0);
  OLED_DisPage(0, 7);
  OLED_ClearBuf();

  GUI_DisString_EN(6, 0, "BOITE A HISTOIRES", &Font12, FONT_BACKGROUND, WHITE);
  GUI_DrawLine(0, 15, sOLED_DIS.OLED_Dis_Column - 1, 15, WHITE, LINE_SOLID , DOT_PIXEL_DFT);

  OLED_DisPage(0, 1);
  OLED_ClearBuf();

  OLED_DisPage(0, 2);
  OLED_ClearBuf();
}

void displayHomePage()
{
  GUI_DisString_EN(12, 2, "DECROCHE ET", &Font12, FONT_BACKGROUND, WHITE);
  OLED_DisPage(0, 3);
  OLED_ClearBuf();
  GUI_DisString_EN(12, 2, "APPUIE SUR UN", &Font12, FONT_BACKGROUND, WHITE);
  OLED_DisPage(0, 4);
  OLED_ClearBuf();
  GUI_DisString_EN(12, 2, "DES BOUTONS =>", &Font12, FONT_BACKGROUND, WHITE);
  OLED_DisPage(0, 5);
  OLED_ClearBuf();
}
