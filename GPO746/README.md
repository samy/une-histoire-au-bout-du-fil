# GPO 746

Ce téléphone provient de Grande-Bretagne (et cet exemplaire date de 1968)
![image](https://user-images.githubusercontent.com/1282106/204911892-fa22c91d-5533-4925-9102-443f3f17eb40.png)

## Adaptation au boitier générique "Une histoire au bout du fil"

Afin de simplifier la conversion de mes téléphones, j'avais créé un boitier générique, avec 4 connecteurs A, B, C et D reliés aux 4 fils du cadran.
Mais cela fonctionnait avec un cadran français, et pas britannique comme celui-ci (le code couleur est différent).

| PIN PCB | Couleur fil UK |
| --- | --- |
| A | Bleu  |
| B | Orange  |
| C | Brun clair (ou rose)  |
| D | Gris  |

## Détection décrochage
Il faudra se brancher sur les connecteurs T1 et T19 du circuit du téléphone (et à défaut, sur T1 et T4)

## Pieds
La hauteur sous l'appareil étant insuffisante pour glisser le lecteur de carte microSD, il est nécessaire d'imprimer des cylindres de **16mm** de diamètre et d'au moins **6mm** de haut.

## Combiné
Un bruit de fond peut survenir dans le combiné, il suffit de rajouter une résistance de 4.7kΩ (pour un volume de 10). Si nécessaire monter jusqu'à 10kΩ (pour un volume de 15).
