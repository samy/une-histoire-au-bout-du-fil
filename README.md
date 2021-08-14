# Une histoire au bout du fil

Ce projet a pour objectif principal de convertir le vieux téléphone présent sur l'image ci-dessous (de la marque AOIP) en boîte à histoires!

![telephone](https://user-images.githubusercontent.com/1282106/129452034-c55ad1a5-5f9b-4c79-a58a-9e0bbab8d801.jpg)

# Technologie

L'idée principale qui sous-tend ce projet est de ne pas dénaturer le téléphone d'origine : on réutilisera donc la circuiterie d'origine, qui est en bon état.

Comme beaucoup de projets embarqués c'est ici un Arduino (Nano) qui sera employé.

Qui dit histoires dit "son" : en effet, on va devoir jouer des fichiers audio à travers le combiné (d'origine).

Cette tâche va être confiée à un lecteur [DFPlayer Mini](https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299) (de la marque DFRobot)

En résumé vous aurez besoin de:
- Arduino Nano (officiel ou non)
- un DFPlayer Mini (autour de 5€)
- une carte micro-SD
- câbles Dupont ou de fils électriques classiques
- une [breadboard](https://www.robot-maker.com/ouvrages/2-1-utiliser-breadboard/) (carte de prototypage) pour pouvoir brancher les composants et les fils et facilement les replacer
- *ou* une [version à souder](https://www.amazon.fr/prototypage-Breadboard-Protoboard-Prototype-%C3%A9lectroniques/dp/B08F2TB5Y5/)
