/******************************************************************************
****************************Upper application layer****************************
  | file      	:	OLED_GUI.c
  |	version		:	V1.0
  | date		:	2017-11-09
  | function	:
  Achieve drawing: draw points, lines, boxes, circles and their size,
				solid dotted line, solid rectangle hollow rectangle,
				solid circle hollow circle.
  Achieve display characters: Display a single character, string, number
  Achieve time display: adaptive size display time minutes and seconds
******************************************************************************/
#include "OLED_GUI.h"
#include "DEV_Config.h"
#include "Debug.h"
#include <stdio.h>

extern OLED_DIS sOLED_DIS;
/********************************************************************************
  function:	Coordinate conversion
********************************************************************************/
void GUI_Swop(POINT Point1, POINT Point2)
{
  POINT Temp;
  Temp = Point1;
  Point1 = Point2;
  Point2 = Temp;
}

/********************************************************************************
  function:	Draw Point(Xpoint, Ypoint) Fill the color
  parameter:
	Xpoint		:   The x coordinate of the point
	Ypoint		:   The y coordinate of the point
	Color		:   Set color
	Dot_Pixel	:	point size
********************************************************************************/
void GUI_DrawPoint(POINT Xpoint, POINT Ypoint, COLOR Color,
                   DOT_PIXEL Dot_Pixel, DOT_STYLE DOT_STYLE)
{
  if (Xpoint > sOLED_DIS.OLED_Dis_Column || Ypoint > sOLED_DIS.OLED_Dis_Page) {
    return;
  }
  uint8_t XDir_Num , YDir_Num;
  if (DOT_STYLE == DOT_STYLE_DFT) {
    for (XDir_Num = 0; XDir_Num < 2 * Dot_Pixel - 1; XDir_Num++) {
      for (YDir_Num = 0; YDir_Num < 2 * Dot_Pixel - 1; YDir_Num++) {
        OLED_SetColor(Xpoint + XDir_Num - Dot_Pixel, Ypoint + YDir_Num - Dot_Pixel, Color);
      }
    }
  } else {
    for (XDir_Num = 0; XDir_Num <  Dot_Pixel; XDir_Num++) {
      for (YDir_Num = 0; YDir_Num <  Dot_Pixel; YDir_Num++) {
        OLED_SetColor(Xpoint + XDir_Num, Ypoint + YDir_Num, Color);
      }
    }
  }
}



/********************************************************************************
  function:	Show English characters
  parameter:
	Xpoint           ：X coordinate
	Ypoint           ：Y coordinate
	Acsii_Char       ：To display the English characters
	Font             ：A structure pointer that displays a character size
	Color_Background : Select the background color of the English character
	Color_Foreground : Select the foreground color of the English character
********************************************************************************/
void GUI_DisChar(POINT Xpoint, POINT Ypoint, const char Acsii_Char,
                 sFONT* Font, COLOR Color_Background, COLOR Color_Foreground)
{
  POINT Page, Column;

  if (Xpoint > sOLED_DIS.OLED_Dis_Column || Ypoint > sOLED_DIS.OLED_Dis_Page) {
    return;
  }

  uint32_t Char_Offset = (Acsii_Char - ' ') * Font->Height * (Font->Width / 8 + (Font->Width % 8 ? 1 : 0));
  const unsigned char *ptr = &Font->table[Char_Offset];

  for (Page = 0; Page < Font->Height; Page ++ ) {
    for (Column = 0; Column < Font->Width; Column ++ ) {

      //To determine whether the font background color and screen background color is consistent
      if (FONT_BACKGROUND == Color_Background) { //this process is to speed up the scan
        if (pgm_read_byte(ptr) & (0x80 >> (Column % 8)))
          GUI_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
      } else {
        if (pgm_read_byte(ptr) & (0x80 >> (Column % 8))) {
          GUI_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
        } else {
          GUI_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Background, DOT_PIXEL_DFT, DOT_STYLE_DFT);
        }
      }
      //One pixel is 8 bits
      if (Column % 8 == 7)
        ptr++;
    }/* Write a line */
    if (Font->Width % 8 != 0)
      ptr++;
  }/* Write all */
}

/********************************************************************************
  function:	Display the string
  parameter:
	Xstart           ：X coordinate
	Ystart           ：Y coordinate
	pString          ：The first address of the English string to be displayed
	Font             ：A structure pointer that displays a character size
	Color_Background : Select the background color of the English character
	Color_Foreground : Select the foreground color of the English character
********************************************************************************/
void GUI_DisString_EN(POINT Xstart, POINT Ystart, const char * pString,
                      sFONT* Font, COLOR Color_Background, COLOR Color_Foreground )
{
  POINT Xpoint = Xstart;
  POINT Ypoint = Ystart;

  if (Xstart > sOLED_DIS.OLED_Dis_Column || Ystart > sOLED_DIS.OLED_Dis_Page) {
    return;
  }

  while (* pString != '\0') {
    //if X direction filled , reposition to(Xstart,Ypoint),Ypoint is Y direction plus the height of the character
    if ((Xpoint + Font->Width ) > sOLED_DIS.OLED_Dis_Column ) {
      Xpoint = Xstart;
      Ypoint += Font->Height;
    }

    // If the Y direction is full, reposition to(Xstart, Ystart)
    if ((Ypoint  + Font->Height ) > sOLED_DIS.OLED_Dis_Page ) {
      Xpoint = Xstart;
      Ypoint = Ystart;
    }
    GUI_DisChar(Xpoint, Ypoint, * pString, Font, Color_Background, Color_Foreground);

    //The next character of the address
    pString ++;

    //The next word of the abscissa increases the font of the broadband
    Xpoint += Font->Width;
  }
}


