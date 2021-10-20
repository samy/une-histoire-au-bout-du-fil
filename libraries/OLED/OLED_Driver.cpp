/******************************************************************************
***************************Intermediate driver layer***************************
  | file      	:	OLED_Driver.cpp
  |	version		:	V1.0
  | date		:	2017-11-09
  | function	:	SSD1327 Drive function

  note:
  Image scanning:
  Please use progressive scanning to generate images or fonts

  Because the Arduino cache is not enough, and the screen manufacturers
  did not lead MISO line, so a byte control of the two points is not very
  good display, so joined a SPIRAM (23K256) chipmodule used to do the
  screen cache, so there Conducive to display.
******************************************************************************/
#include "OLED_Driver.h"
#include "SPI_RAM.h"
#include "Debug.h"
#include <stdio.h>

COLOR Buffer[OLED_BUFSIZ];

OLED_DIS sOLED_DIS;
/*******************************************************************************
  function:
			Hardware reset
*******************************************************************************/
static void OLED_Reset(void)
{
  OLED_RST_1;
  Driver_Delay_ms(100);
  OLED_RST_0;
  Driver_Delay_ms(100);
  OLED_RST_1;
  Driver_Delay_ms(100);
}

/*******************************************************************************
  function:
		Write register address and data
*******************************************************************************/
void OLED_WriteReg(uint8_t Reg)
{
#if USE_SPI_4W
  OLED_DC_0;
  OLED_CS_0;
  SPI4W_Write_Byte(Reg);
  OLED_CS_1;
#elif USE_IIC
  I2C_Write_Byte(Reg, IIC_CMD);
#endif
}

void OLED_WriteData(uint8_t Data)
{
#if USE_SPI_4W
  OLED_DC_1;
  OLED_CS_0;
  SPI4W_Write_Byte(Data);
  OLED_CS_1;
#elif USE_IIC
  I2C_Write_Byte(Data, IIC_RAM);
#endif
}

/*******************************************************************************
  function:
		Common register initialization
*******************************************************************************/
static void OLED_InitReg(void)
{
  OLED_WriteReg(0xae);//--turn off oled panel

  OLED_WriteReg(0x15);    //set column address
  OLED_WriteReg(0x00);    //start column   0
  OLED_WriteReg(0x7f);    //end column   127

  OLED_WriteReg(0x75);    //set row address
  OLED_WriteReg(0x00);    //start row   0
  OLED_WriteReg(0x7f);    //end row   127

  OLED_WriteReg(0x81);    //set contrast control
  OLED_WriteReg(0x80);

  OLED_WriteReg(0xa0);    //gment remap
  OLED_WriteReg(0x51);    //51

  OLED_WriteReg(0xa1);    //start line
  OLED_WriteReg(0x00);

  OLED_WriteReg(0xa2);    //display offset
  OLED_WriteReg(0x00);

  OLED_WriteReg(0xa4);    //rmal display
  OLED_WriteReg(0xa8);    //set multiplex ratio
  OLED_WriteReg(0x7f);

  OLED_WriteReg(0xb1);    //set phase leghth
  OLED_WriteReg(0xf1);

  OLED_WriteReg(0xb3);    //set dclk
  OLED_WriteReg(0x00);    //80Hz:0xc1 90Hz:0xe1   100Hz:0x00   110Hz:0x30 120Hz:0x50   130Hz:0x70     01

  OLED_WriteReg(0xab);    //
  OLED_WriteReg(0x01);    //

  OLED_WriteReg(0xb6);    //set phase leghth
  OLED_WriteReg(0x0f);

  OLED_WriteReg(0xbe);
  OLED_WriteReg(0x0f);

  OLED_WriteReg(0xbc);
  OLED_WriteReg(0x08);

  OLED_WriteReg(0xd5);
  OLED_WriteReg(0x62);

  OLED_WriteReg(0xfd);
  OLED_WriteReg(0x12);

}

/********************************************************************************
  function:	Set the display scan and color transfer modes
  parameter:
		Scan_dir   :   Scan direction
		Colorchose :   RGB or GBR color format
********************************************************************************/
void OLED_SetGramScanWay(OLED_SCAN_DIR Scan_dir)
{
  //Get the screen scan direction
  sOLED_DIS.OLED_Scan_Dir = Scan_dir;

  if (Scan_dir == L2R_U2D) {
    OLED_WriteReg(0xa0);    //gment remap
    OLED_WriteReg(0x51);    //51
    //OLED_WriteReg(0xa7);
  } else if (Scan_dir == L2R_D2U) { //Y
    OLED_WriteReg(0xa0);    //gment remap
    OLED_WriteReg(0x41);    //51
  } else if (Scan_dir == R2L_U2D) {
    OLED_WriteReg(0xa0);    //gment remap
    OLED_WriteReg(0x52);    //51
  } else if (Scan_dir == R2L_D2U) {
    OLED_WriteReg(0xa0);    //gment remap
    OLED_WriteReg(0x42);    //51
  }/*else if(Scan_dir == U2D_L2R){
		OLED_WriteReg(0xa0);    //gment remap
		OLED_WriteReg(0x51);    //51
	}else if(Scan_dir == U2D_R2L){
		OLED_WriteReg(0xa0);    //gment remap
		OLED_WriteReg(0x51);    //51
	}else if(Scan_dir == D2U_L2R){
		OLED_WriteReg(0xa0);    //gment remap
		OLED_WriteReg(0x41);    //51
	}else if(Scan_dir == D2U_R2L){//Y
		OLED_WriteReg(0xa0);    //gment remap
		OLED_WriteReg(0x42);    //51
	}*/else {
    return;
  }

  //Get GRAM and OLED width and height
  if (Scan_dir == L2R_U2D || Scan_dir == L2R_D2U || Scan_dir == R2L_U2D || Scan_dir == R2L_D2U) {
    sOLED_DIS.OLED_Dis_Column	= OLED_WIDTH;
    sOLED_DIS.OLED_Dis_Page = OLED_HEIGHT;
    sOLED_DIS.OLED_X_Adjust = OLED_X;
    sOLED_DIS.OLED_Y_Adjust = OLED_Y;
  } else {
    sOLED_DIS.OLED_Dis_Column	= OLED_HEIGHT;
    sOLED_DIS.OLED_Dis_Page = OLED_WIDTH;
    sOLED_DIS.OLED_X_Adjust = OLED_Y;
    sOLED_DIS.OLED_Y_Adjust = OLED_X;
  }
}

/********************************************************************************
  function:
			initialization
********************************************************************************/
void OLED_Init(OLED_SCAN_DIR OLED_ScanDir)
{
  //Hardware reset
  OLED_Reset();

  //Set the initialization register
  OLED_InitReg();

  //Set the display scan and color transfer modes
  OLED_SetGramScanWay(OLED_ScanDir );
  Driver_Delay_ms(200);

  //Turn on the OLED display
  OLED_WriteReg(0xAF);
}

/********************************************************************************
  function:	Set the display point(Xpoint, Ypoint)
  parameter:
		xStart :   X direction Start coordinates
		xEnd   :   X direction end coordinates
********************************************************************************/
void OLED_SetCursor(POINT Xpoint, POINT Ypoint)
{
  if ((Xpoint > sOLED_DIS.OLED_Dis_Column) || (Ypoint > sOLED_DIS.OLED_Dis_Page))
    return;

  OLED_WriteReg(0x15);
  OLED_WriteReg(Xpoint);
  OLED_WriteReg(Xpoint);

  OLED_WriteReg(0x75);
  OLED_WriteReg(Ypoint);
  OLED_WriteReg(Ypoint);
}

/********************************************************************************
  function:	Set the display Window(Xstart, Ystart, Xend, Yend)
  parameter:
		xStart :   X direction Start coordinates
		Ystart :   Y direction Start coordinates
		Xend   :   X direction end coordinates
		Yend   :   Y direction end coordinates
********************************************************************************/
void OLED_SetWindow(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend)
{
  if ((Xstart > sOLED_DIS.OLED_Dis_Column) || (Ystart > sOLED_DIS.OLED_Dis_Page) ||
      (Xend > sOLED_DIS.OLED_Dis_Column) || (Yend > sOLED_DIS.OLED_Dis_Page))
    return;

  OLED_WriteReg(0x15);
  OLED_WriteReg(Xstart);
  OLED_WriteReg(Xend - 1);

  OLED_WriteReg(0x75);
  OLED_WriteReg(Ystart);
  OLED_WriteReg(Yend - 1);
}

/********************************************************************************
  function:	Set show color
  parameter:
		Color  :   Set show color,16-bit depth
********************************************************************************/
void OLED_SetColor(POINT Xpoint, POINT Ypoint, COLOR Color)
{
  if (Xpoint > sOLED_DIS.OLED_Dis_Column || Ypoint > sOLED_DIS.OLED_Dis_Page) {
    return;
  }
#if USE_INT_RAM
  if (Xpoint % 2 == 0) {
    Buffer[Xpoint / 2 + Ypoint * 64] = (Color << 4) | Buffer[Xpoint / 2 + Ypoint * 64];
  } else {
    Buffer[Xpoint / 2 + Ypoint * 64] = (Color & 0x0f) | Buffer[Xpoint / 2 + Ypoint * 64];
  }

#elif USE_EXT_RAM
  uint8_t R_Buf;
  
  //1 byte control two points
  R_Buf = SPIRAM_RD_Byte(Xpoint / 2 + Ypoint * 64);
  if (Xpoint % 2 == 0) {
    SPIRAM_WR_Byte(Xpoint / 2 + Ypoint * 64, (Color << 4) | R_Buf);
  } else {
    SPIRAM_WR_Byte(Xpoint / 2 + Ypoint * 64, (Color & 0x0f) | R_Buf);
  }
#endif
}

/********************************************************************************
  function:
			Clear screen
********************************************************************************/
void OLED_ClearScreen(COLOR Color)
{
  unsigned int i, m;
#if USE_INT_RAM
  OLED_SetWindow(0, 0, sOLED_DIS.OLED_Dis_Column, sOLED_DIS.OLED_Dis_Page);
  for (i = 0; i < sOLED_DIS.OLED_Dis_Page; i++) {
    for (m = 0; m < (sOLED_DIS.OLED_Dis_Column / 2); m++) {
      OLED_WriteData(Color);     
    }
  }
#elif USE_EXT_RAM
  for (i = 0; i < sOLED_DIS.OLED_Dis_Page; i++) {
    for (m = 0; m < (sOLED_DIS.OLED_Dis_Column / 2); m++) {
      SPIRAM_WR_Byte(i * 64 + m, Color | (Color << 4));
    }
  }
#endif
}

void OLED_ClearBuf()
{
  unsigned int i, m;
  for (i = 0; i < BUFSIZ / (sOLED_DIS.OLED_Dis_Column / 2); i++) {
    for (m = 0; m < (sOLED_DIS.OLED_Dis_Column / 2); m++) {
      Buffer[i * (sOLED_DIS.OLED_Dis_Column / 2) + m] = 0x00;
    }
  }
}
/********************************************************************************
  function:	Update all memory to LCD
********************************************************************************/
void OLED_Display(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend)
{
  uint16_t page, Column;

  OLED_SetWindow(Xstart, Ystart, Xend, Yend);
#if USE_INT_RAM
  COLOR *pBuf = (COLOR *)Buffer;
  //write data
  for (page = 0; page < BUFSIZ / (sOLED_DIS.OLED_Dis_Column / 2); page++) {
    for (Column = 0; Column < sOLED_DIS.OLED_Dis_Column / 2; Column++ ) {
      OLED_WriteData(*pBuf);
      pBuf++;
    }
  }
#elif USE_EXT_RAM
  //write data
  uint8_t R_Buf;
  for (page = 0; page < sOLED_DIS.OLED_Dis_Page; page++) {
    for (Column = 0; Column < sOLED_DIS.OLED_Dis_Column / 2; Column++ ) {
      R_Buf = SPIRAM_RD_Byte(page * 64 + Column);
      OLED_WriteData(R_Buf);
    }
  }
#endif
}

/********************************************************************************
  function:
			Clear Window
********************************************************************************/
void OLED_ClearWindow(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend, COLOR Color)
{
  uint16_t i, m, Xlen, Ylen;
  Xlen = (Xend - Xstart) / 2;
  Ylen = Yend - Ystart;

  uint16_t Offset = Xstart + Ystart * (sOLED_DIS.OLED_Dis_Column / 2);
#if USE_INT_RAM
  for (i = 0; i < Ylen; i++) {
    for (m = 0; m < Xlen; m++) {
      Buffer[Offset + m] = Color;
    }
    Offset = Xstart + (Ystart + i + 1) * (sOLED_DIS.OLED_Dis_Column / 2);
  }
#elif USE_EXT_RAM
  for (i = 0; i < Ylen; i++) {
    for (m = 0; m < Xlen; m++) {
      SPIRAM_WR_Byte(Offset + m, Color | (Color << 4));
    }
    Offset = Xstart + (Ystart + i) * (sOLED_DIS.OLED_Dis_Column / 2);
  }
#endif
}

/********************************************************************************
  function:	Update Window memory to LCD
********************************************************************************/
void OLED_DisWindow(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend)
{
  uint16_t page, Column, Xlen, Ylen;
  Xlen = (Xend - Xstart) / 2;
  Ylen = Yend - Ystart;
  OLED_SetWindow(Xstart, Ystart, Xend, Yend);
#if USE_INT_RAM
  //write data
  COLOR *pBuf = (COLOR *)Buffer + Xstart + Ystart * (sOLED_DIS.OLED_Dis_Column / 2);
  for (page = 0; page < Ylen; page++) {
    for (Column = 0; Column < Xlen; Column++ ) {
      OLED_WriteData(*pBuf);
      pBuf++;
    }
    pBuf = (COLOR *)Buffer + Xstart + (Ystart + page + 1) * (sOLED_DIS.OLED_Dis_Column / 2);
  }
#elif USE_EXT_RAM
  //write data
  uint16_t Offset = Xstart + Ystart * (sOLED_DIS.OLED_Dis_Column / 2);
  uint8_t	R_Buf;
  for (page = 0; page < Ylen; page++) {
    for (Column = 0; Column < Xlen; Column++ ) {
      R_Buf = SPIRAM_RD_Byte(Offset + Column);
      OLED_WriteData(R_Buf);
    }
    Offset = Xstart + (Ystart + page) * (sOLED_DIS.OLED_Dis_Column / 2);
  }
#endif
}

void OLED_DisPage(unsigned int Column_Num, unsigned int Page_Num)
{
  unsigned int column_max = OLED_WIDTH / XByte;
  unsigned int page_max = OLED_HEIGHT / YByte;
  if (Page_Num > page_max || Column_Num > column_max) {
    DEBUG("Display error\r\n");
    return;
  }
  unsigned int Dx = Column_Num % column_max;
  unsigned int Dy = Page_Num % page_max;
  OLED_Display(Dx * XByte, Dy * YByte, (Dx + 1) * XByte, (Dy + 1) * YByte);
}

