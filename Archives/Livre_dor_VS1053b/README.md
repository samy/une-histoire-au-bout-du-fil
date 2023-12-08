## Correspondance des pins XIAO - VS1053

| VS1053 | XIAO |
| ------------- | ------------- |
| CS  | D7  |
| MISO  | D9  |
| (MO)SI  | D10  |
| SCK | D8  |
| XCS | D6  |
| XRESET | D5  |
| XDCS | D4  |
| DREQ | D3  |

Seul l'emplacement des MISO, MOSI et SCK est réellement important (pour les autres il vous suffit de changer les valeurs dans le code).
Correction dans la bibliothèque Adafruit VS1053

#define VS1053_CONTROL_SPI_SETTING                                             \
  SPISettings(2000000, MSBFIRST, SPI_MODE0) //!< VS1053 SPI control settings
#define VS1053_DATA_SPI_SETTING                                                \
  SPISettings(12000000, MSBFIRST, SPI_MODE0) //!< VS1053 SPI data settings
  
A rajouter dans le code Arduino:

#define SPI_HAS_TRANSACTION 1

