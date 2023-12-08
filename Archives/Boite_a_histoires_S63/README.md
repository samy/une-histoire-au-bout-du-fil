# Boite à histoires "Socotel S63"

![image](https://user-images.githubusercontent.com/1282106/144014466-de22c6db-30d0-470b-b444-1885433b99f5.png)

(source : [x0r.fr](https://x0r.fr/blog/53) )

## Schema du circuit
<img src="https://user-images.githubusercontent.com/1282106/159553098-f76d16e0-87f6-4340-8629-fd2b48387afb.png" align="right" />

<table border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td width="50%"><img src="https://user-images.githubusercontent.com/1282106/159449549-88aea507-3547-4f7f-97e7-b094df9191a9.png" align="right" /></td>
    <td><img src="https://user-images.githubusercontent.com/1282106/159546687-9603c016-0289-4233-be95-e127fce51251.png"></td>
  </tr>
</table>

## Liaison cadran - Arduino

| Couleur       | Destination |
| ------------- | ----------- |
| Blanc-bleu    | Port A      |
| Blanc-rouge   | Port B      |
| Rouge         | Port C      |
| Bleu          | Port D      |

# Matériel à rassembler
- Un [Arduino Nano](https://www.gotronic.fr/art-carte-arduino-nano-12422.htm) (mais vous pouvez utiliser d'autres modèles sans problème, selon votre boitier)
- Un [DFPlayer Mini](https://www.gotronic.fr/art-module-mp3-dfr0299-22404.htm)
- Une résistance de 1kΩ

# Pré-requis avant utilisation
Sur la carte micro-SD, créez un dossier nommé _MP3_ et stockez-y vos fichiers à faire jouer par le téléphone. Ils doivent être prefixés par un numéro à 4 chiffres, qui les fera correspondre au chiffre composé sur le cadran :
- **0001.mp3** ou **0001 - cequevousvoulez.mp3** pour le morceau 1,
- **0002.mp3** ou **0002 - cequevousvoulez.mp3** pour le morceau 2,
- etc

# Code Arduino
On utilise ici deux bibliothèques non natives :
- [*DFRobotDFPlayerMini*](https://github.com/DFRobot/DFRobotDFPlayerMini) (installable via le gestionnaire de bibliothèques)
- [*RotaryDial*](https://github.com/markfickett/Rotary-Dial), (téléchargeable sur Github et installable au format ZIP)

Le [code source](./Boite_a_histoires_S63.ino) est abondamment commenté en français.

# Etiquette du cadran
Celle-ci est constituée d'un disque de 51mm de diamètre, et la police utilisée semble être du Univers Condensed Medium avec en 7.5pt un crénage de 46 (merci à [Jean-François Porchez](https://typofonderie.com) pour la suggestion de la police Univers)

![image](https://user-images.githubusercontent.com/1282106/144504833-abf780c6-383c-48c3-8791-4e733949bcb3.png)

# Licence
<a rel="license" href="https://creativecommons.org/licenses/by-nc-sa/4.0/deed.fr"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br />Ce projet est partagé sous la licence <a rel="license" href="https://creativecommons.org/licenses/by-nc-sa/4.0/deed.fr">Attribution - Pas d’Utilisation Commerciale - Partage dans les Mêmes Conditions 4.0 International</a>.
