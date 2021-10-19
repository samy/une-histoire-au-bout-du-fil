/* Gestion CSV */
char** MP3Authors;
char** MP3Titles;
bool hasReadCSV = false;

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
#include "Debug.h"
bool isScreenInitialized = false;

/* Lecteur carte SD*/
#define USE_FAT_FILE_FLAG_CONTIGUOUS 0
#define USE_LONG_FILE_NAMES 0
#define SDFAT_FILE_TYPE 1
#define SD_FAT_TYPE 0
#define USE_DEDICATED_SPI 0
#include <SdFat.h>
#include <SdFatConfig.h>

/* Boutons */
const int buttonCount = 10;                              //Nombre de boutons
int buttonStates[buttonCount];                           //Etat des boutons
int lastButtonPressedIndex = -1;                         //Numéro du dernier bouton pressé
int buttonsPins[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };  //Liste des ports sur lesquels sont branchés les boutons
int powerPin = 12;
bool isAButtonPressed = false;
bool isADifferentButtonPressed = false;
bool areButtonsInitialized = false;
int MP3_PIN1 = A6;  //Le port de l'Ardunino branché sur le TX du DFPlayer
int MP3_PIN2 = A5;  //Le port de l'Ardunino branché sur le RX du DFPlayer
SoftwareSerial MP3Serial(MP3_PIN1, MP3_PIN2);
DFRobotDFPlayerMini mp3Player;

void setup() {

  //Initialisation du lecteur MP3
  MP3Serial.begin(9600);
  mp3Player.begin(MP3Serial);
  mp3Player.volume(25);
}

void loop() {
  if (!hasReadCSV) {
    readFromSd();
  }
  if (!isScreenInitialized) {
    Ecran();
  }
  if (!checkPower()) {
    return;
  }
  if (!areButtonsInitialized) {
    initializeButtons();
  }
  Serial.println("ok");
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
      Serial.println((String) "Bouton :" + i);
      Serial.println((String) "lastButtonPressedIndex :" + lastButtonPressedIndex);
      isAButtonPressed = true;
      //Inutile de retester 60 fois / seconde si un bouton est enfoncé :)
      if (lastButtonPressedIndex != i || lastButtonPressedIndex == -1) {
        Serial.println("A different button has been pressed1");

        isADifferentButtonPressed = true;
      }
      lastButtonPressedIndex = i;
      delay(1000);

      break;
    }
  }

  //Si un bouton a été pressé
  if (isAButtonPressed) {
    Serial.println("A button has been pressed");
    //Si c'est un autre bouton que celui d'avant, on arrête la lecture seulement si elle est en cours
    //Valable aussi au premier bouton
    if (isADifferentButtonPressed) {
      Serial.println("A different button has been pressed2");
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

void Ecran() {
  System_Init();

  OLED_SCAN_DIR OLED_ScanDir = SCAN_DIR_DFT;
  OLED_Init( OLED_ScanDir );

  //GUI_Show();
  OLED_ClearBuf();
  OLED_ClearScreen(OLED_BACKGROUND);

  GUI_DisString_EN(0 , 2, "I'M ALIVE !!", &Font16, FONT_BACKGROUND, WHITE);

  OLED_Display(0, 65, 128, 65 + 32);
  OLED_ClearBuf();
  isScreenInitialized = true;

}

/* Lit le CSV depuis la carte SD et stocke les données pour chaque colonne/ligne */
void readFromSd() {

  SdFat SD;
  File dir;
  File file;
  int SD_PIN = A0;

#define SD_CONFIG SdSpiConfig(SD_PIN)
#define error(s) SD.errorHalt(&Serial, F(s))

  // Initialize the SD.
  if (!SD.begin(SD_CONFIG)) {
    Serial.println("BAD");

    SD.initErrorHalt(&Serial);
  }
  // Create the file.
  if (!file.open("liste.csv", FILE_READ)) {
    error("open failed");
  }

  // Rewind file for read.
  file.rewind();
  hasReadCSV = true;
}
