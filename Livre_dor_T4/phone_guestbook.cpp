#include "phone_guestbook.h"

/* Récupération de l'état de décroché/raccroché */
bool isHangedUp() {
  return 1 == digitalRead(PIN_HANG);
}


bool PhoneGuestBook::needToPlayIntro() {
  if (this->introHasBeenPlayed) {
    return false;
  }
  if ((strcmp(this->phoneMode, "record") == 0 && this->IntroRecordEnabled)
      || (strcmp(this->phoneMode, "play") == 0 && this->IntroPlayEnabled)) {
    introHasBeenPlayed = true;
    return true;
  }
  return false;
}

void PhoneGuestBook::enableIntroBeforeRecord() {
  this->IntroRecordEnabled = true;
}

void PhoneGuestBook::enableIntroBeforePlay() {
  this->IntroPlayEnabled = true;
}