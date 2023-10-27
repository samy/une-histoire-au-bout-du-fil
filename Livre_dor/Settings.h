#define PIN_HANG 14         //Port lié au fil du décrochage
#define PIN_READY 1         //Correspond au A du cadran
#define PIN_PULSE 2         //Correspond au C du cadran
#define PIN_MODE_CHANGE 17  //Pour indiquer qu'on doit changer de mode
#define PIN_LED 4           //Pour indiquer que le mode enregistrement est actif

/* Optionnels */
#define PIN_RESET 4   //Pour annuler l'enregistrement courant - Nécessite RESET_ENABLE activé
#define PIN_REPLAY 2  //Pour réécouter le dernier enregistrement - Nécessite REPLAY_ENABLE activé

#define MTP_ENABLE false           //Activation du mode MTP
#define AUTO_PLAY false            //Déclenchement automatique dès qu'on est en mode lecture
#define REVERSE_MODE_CHANGE false  //A ajuster selon l'interrupteur choisi pour le changement de mode
#define RECORD_ON_DIAL true        //Utiliser le cadran pour déclencher un enregistrement (rangement dans le dossier lié au numéro composé)







/* Réglages par défaut */
#ifndef MTP_ENABLE
#define MTP_ENABLE false
#endif

#ifndef RESET_ENABLE
#define RESET_ENABLE false
#endif

#ifndef REPLAY_ENABLE
#define REPLAY_ENABLE false
#endif

#ifndef REVERSE_MODE_CHANGE
#define REVERSE_MODE_CHANGE false
#endif

#ifndef AUTO_PLAY
#define AUTO_PLAY false
#endif

#ifndef RECORD_ON_DIAL
#define RECORD_ON_DIAL false
#endif