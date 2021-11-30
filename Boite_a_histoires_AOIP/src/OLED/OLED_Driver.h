/******************************************************************************
***************************Intermediate driver layer***************************
* | file      	:	OLED_Driver.h
* |	version		:	V1.0
* | date		:	2017-11-09
* | function	:	SSD1327 Drive function

note:
Image scanning:
Please use progressive scanning to generate images or fonts

Because the Arduino cache is not enough, and Under serial mode, only 
write operations are allowed,But a byte controls the data of two 
points. is not very good display, so joined a SPIRAM (23K256) 
chipmodule used to do the screen cache, so there Conducive to display.
******************************************************************************/
#ifndef __OLED_DRIVER_H
#define __OLED_DRIVER_H

#include "DEV_Config.h"

#define	COLOR				uint8_t			//The variable type of the color (unsigned short) 
#define	POINT				uint16_t		//The type of coordinate (unsigned short) 
#define	LENGTH				uint16_t		//The type of coordinate (unsigned short) 

/********************************************************************************
function:
		Define the full screen height length of the display
********************************************************************************/
#define OLED_X_MAXPIXEL  128  //OLED width maximum memory 
#define OLED_Y_MAXPIXEL  128 //OLED height maximum memory
#define OLED_X	 0
#define OLED_Y	 0

#define OLED_WIDTH  (OLED_X_MAXPIXEL - 2 * OLED_X)  //OLED width
#define OLED_HEIGHT  OLED_Y_MAXPIXEL //OLED height

/********************************************************************************
function:
		Selection control mode
********************************************************************************/
#define USE_INT_RAM 1
#define USE_EXT_RAM 0
#define USE_OLED_RAM 0

#if USE_INT_RAM
    #define XByte 64 //1 BYTE = 2dot,X max Dot = 128
    #define YByte 16 //1 Byte = 1Dot,y max Dot = 128 
#elif USE_EXT_RAM
    #define XByte (OLED_WIDTH / 2)
    #define YByte OLED_HEIGHT
#elif USE_OLED_RAM
    DEV_DEBUG("Err...");//Under serial mode, only write operations are allowed,But a byte controls the data of two points.
#else
    DEV_DEBUG("Please check which RAM control is used.");
#endif
#define OLED_BUFSIZ XByte * YByte
/********************************************************************************
function:
			scanning method
********************************************************************************/
typedef enum {
    L2R_U2D  = 0,	//The display interface is displayed , left to right, up to down
    L2R_D2U  ,
    R2L_U2D  ,
    R2L_D2U  ,

    U2D_L2R  ,
    U2D_R2L  ,
    D2U_L2R  ,
    D2U_R2L  ,
} OLED_SCAN_DIR;
#define SCAN_DIR_DFT  L2R_U2D  //Default scan direction = L2R_U2D

/********************************************************************************
function:
	Defines the total number of rows in the display area
********************************************************************************/
typedef struct {
    LENGTH OLED_Dis_Column;	//COLUMN
    LENGTH OLED_Dis_Page;	//PAGE
    OLED_SCAN_DIR OLED_Scan_Dir;
    POINT OLED_X_Adjust;		//OLED x actual display position calibration
    POINT OLED_Y_Adjust;		//OLED y actual display position calibration
} OLED_DIS;

/********************************************************************************
function:
			Macro definition variable name
********************************************************************************/
void OLED_Init( OLED_SCAN_DIR OLED_ScanDir);
void OLED_SetGramScanWay(OLED_SCAN_DIR Scan_dir);

void OLED_WriteReg(uint8_t Reg);
void OLED_WriteData(uint8_t Data);

//OLED set cursor + windows + color
void OLED_SetCursor(POINT Xpoint, POINT Ypoint);
void OLED_SetColor(POINT Xpoint, POINT Ypoint, COLOR Color);
void OLED_SetWindow(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend);
void OLED_SetWindowColor(COLOR Color);
//Full screen operation
void OLED_ClearScreen(COLOR  Color);
void OLED_ClearBuf();
void OLED_Display(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend);
//Local screen operation
void OLED_ClearWindow(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend, COLOR Color);
void OLED_DisWindow(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend);

//Combination operation
void OLED_DisPage(unsigned int Column_Num, unsigned int Page_Num);
#endif





