
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId = "None";
String newTagId = "None";
byte nuidPICC[4];

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("System initialized");
  nfc.begin();
}

void loop() {
  readNFC();
}

void readNFC() {
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    //tag.print();
    newTagId = tag.getUidString();
    if (tagId != newTagId) {
      tagId = newTagId;

      Serial.print("Tag RFID trouvé : ");
      Serial.println(tagId);
      Serial.println(' ');
    }
  }
  delay(100);
}