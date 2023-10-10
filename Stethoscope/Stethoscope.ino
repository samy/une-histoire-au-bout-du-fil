
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId = "None";
String newTagId = "None";
const char* tags[3] = { "uno", "duo", "tri" };


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
    Serial.println(findNumberForTag(newTagId));
    if (tagId != newTagId) {
      tagId = newTagId;
      Serial.print("Tag RFID trouvé : ");
      Serial.println(tagId);
      Serial.println(' ');
    }
  }
  delay(100);
}

int findNumberForTag(String tagKey) {
  int tagIndex = 0;
  for (tagIndex = 0; tagIndex < sizeof(tags) / sizeof(tags[0]); tagIndex++) {
    if (strcmp(tags[tagIndex], tagKey.c_str()) == 0) {
      break;
    }
  }
  return tagIndex;
}
