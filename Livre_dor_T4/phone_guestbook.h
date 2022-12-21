#include <Arduino.h>

#ifndef PHONE_GUESTBOOK
#define PHONE_GUESTBOOK

class PhoneGuestBook {
public:
  void enableIntroBeforeRecord();
  void enableIntroBeforePlay();
  bool needToPlayIntro();
  bool IntroRecordEnabled;
  bool IntroPlayEnabled;
  char phoneMode[10];
private:
  bool introHasBeenPlayed;
};


#ifndef RECORDS_NUMBER_FILE_NAME
#define RECORDS_NUMBER_FILE_NAME ".records_file_number"
#endif



#define PIN_HANG 10  //Port lié au fil du décrochage

bool isHangedUp();
bool needToPlayIntro();


#endif