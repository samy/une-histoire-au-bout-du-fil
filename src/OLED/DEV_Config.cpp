/******************************************************************************
**************************Hardware interface layer*****************************
  | file      	:	DEV_Config.cpp
  |	version		:	V1.0
  | date		:	2017-12-11
  | function	:
	Provide the hardware underlying interface
******************************************************************************/
#include "DEV_Config.h"

/********************************************************************************
  function:	System Init and exit
  note:
	Initialize the communication method
********************************************************************************/
uint8_t System_Init(void)
{
  //set pin
  pinMode(OLED_CS, OUTPUT);
  pinMode(OLED_RST, OUTPUT);
  pinMode(OLED_DC, OUTPUT);
  pinMode(SPIRAM_CS, OUTPUT);

  //set Serial
  Serial.begin(115200);

#if USE_SPI_4W
  //set OLED SPI
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();

#elif USE_IIC
  //set OLED I2C
  OLED_DC_1;//DC = 1 => Address = 0x3d
  OLED_CS_0;
  Wire.setClock(1000000);
  Wire.begin();

  //SET SPIRAM SPI
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();
#endif
  return 0;
}

void System_Exit(void)
{

}

/********************************************************************************
  function:	Hardware interface
  note:
	SPI4W_Write_Byte(value) :
		hardware SPI
	I2C_Write_Byte(value, cmd):
		hardware I2C
********************************************************************************/
void SPI4W_Write_Byte(uint8_t DATA)
{
  SPI.transfer(DATA);
}

uint8_t SPI4W_Read_Byte(uint8_t DATA)
{
  return SPI.transfer(DATA);
}

void I2C_Write_Byte(uint8_t value, uint8_t Cmd)
{
  uint8_t Addr = 0x3d;
  Wire.beginTransmission(Addr);
  Wire.write(Cmd);
  Wire.write(value);
  Wire.endTransmission();
}

/********************************************************************************
  function:	Delay function
  note:
	Driver_Delay_ms(xms) : Delay x ms
	Driver_Delay_us(xus) : Delay x us
********************************************************************************/
void Driver_Delay_ms(unsigned long xms)
{
  delay(xms);
}

void Driver_Delay_us(int xus)
{
  for (int j = xus; j > 0; j--);
}
