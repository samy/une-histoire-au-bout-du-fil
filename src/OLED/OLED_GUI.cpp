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
    Serial.print("GUI_DrawPoint Input exceeds the normal display range\r\n");
    return;
  }
  uint8_t XDir_Num , YDir_Num;
  if (DOT_STYLE == DOT_STYLE_DFT) {
    for (XDir_Num = 0; XDir_Num < 2 * Dot_Pixel - 1; XDir_Num++) {
      for (YDir_Num = 0; YDir_Num < 2 * Dot_Pixel - 1; YDir_Num++) {
        //				Serial.print("YDir_Num = %d\r\n",YDir_Num);
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
  function:	Draw a line of arbitrary slope
  parameter:
	Xstart ：Starting x point coordinates
	Ystart ：Starting x point coordinates
	Xend   ：End point x coordinate
	Yend   ：End point y coordinate
	Color  ：The color of the line segment
********************************************************************************/
void GUI_DrawLine(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
                  COLOR Color, LINE_STYLE Line_Style, DOT_PIXEL Dot_Pixel)
{
  if (Xstart > sOLED_DIS.OLED_Dis_Column || Ystart > sOLED_DIS.OLED_Dis_Page ||
      Xend > sOLED_DIS.OLED_Dis_Column || Yend > sOLED_DIS.OLED_Dis_Page) {
    Serial.print("GUI_DrawLine Input exceeds the normal display range\r\n");
    return;
  }
  if (Xstart > Xend)
    GUI_Swop(Xstart, Xend);
  if (Ystart > Yend)
    GUI_Swop(Ystart, Yend);

  POINT Xpoint = Xstart;
  POINT Ypoint = Ystart;
  int dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
  int dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;

  // Increment direction, 1 is positive, -1 is counter;
  int XAddway = Xstart < Xend ? 1 : -1;
  int YAddway = Ystart < Yend ? 1 : -1;

  //Cumulative error
  int Esp = dx + dy;
  char Line_Style_Temp = 0;

  for (;;) {
    Line_Style_Temp++;
    //Painted dotted line, 2 point is really virtual
    if (Line_Style == LINE_DOTTED && Line_Style_Temp % 3 == 0) {
      GUI_DrawPoint(Xpoint, Ypoint, OLED_BACKGROUND, Dot_Pixel, DOT_STYLE_DFT);
      Line_Style_Temp = 0;
    } else {
      GUI_DrawPoint(Xpoint, Ypoint, Color, Dot_Pixel, DOT_STYLE_DFT);
    }
    if (2 * Esp >= dy) {
      if (Xpoint == Xend) break;
      Esp += dy;
      Xpoint += XAddway;
    }
    if (2 * Esp <= dx) {
      if (Ypoint == Yend) break;
      Esp += dx;
      Ypoint += YAddway;
    }
  }
}

/********************************************************************************
  function:	Draw a rectangle
  parameter:
	Xstart ：Rectangular  Starting x point coordinates
	Ystart ：Rectangular  Starting x point coordinates
	Xend   ：Rectangular  End point x coordinate
	Yend   ：Rectangular  End point y coordinate
	Color  ：The color of the Rectangular segment
	Filled : Whether it is filled--- 1 solid 0：empty
********************************************************************************/
void GUI_DrawRectangle(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
                       COLOR Color, DRAW_FILL Filled, DOT_PIXEL Dot_Pixel )
{
  if (Xstart > sOLED_DIS.OLED_Dis_Column || Ystart > sOLED_DIS.OLED_Dis_Page ||
      Xend > sOLED_DIS.OLED_Dis_Column || Yend > sOLED_DIS.OLED_Dis_Page) {
    Serial.print("Input exceeds the normal display range\r\n");
    return;
  }
  if (Xstart > Xend)
    GUI_Swop(Xstart, Xend);
  if (Ystart > Yend)
    GUI_Swop(Ystart, Yend);

  POINT Ypoint;
  if (Filled ) {
    for (Ypoint = Ystart; Ypoint < Yend; Ypoint++) {
      GUI_DrawLine(Xstart, Ypoint, Xend, Ypoint, Color , LINE_SOLID, Dot_Pixel);
    }
  } else {
    GUI_DrawLine(Xstart, Ystart, Xend, Ystart, Color , LINE_SOLID, Dot_Pixel);
    GUI_DrawLine(Xstart, Ystart, Xstart, Yend, Color , LINE_SOLID, Dot_Pixel);
    GUI_DrawLine(Xend, Yend, Xend, Ystart, Color , LINE_SOLID, Dot_Pixel);
    GUI_DrawLine(Xend, Yend, Xstart, Yend, Color , LINE_SOLID, Dot_Pixel);
  }
}

/********************************************************************************
  function:	Use the 8-point method to draw a circle of the
				specified size at the specified position.
  parameter:
	X_Center  ：Center X coordinate
	Y_Center  ：Center Y coordinate
	Radius    ：circle Radius
	Color     ：The color of the ：circle segment
	Filled    : Whether it is filled: 1 filling 0：Do not
********************************************************************************/
void GUI_DrawCircle(POINT X_Center, POINT Y_Center, LENGTH Radius,
                    COLOR Color, DRAW_FILL  Draw_Fill , DOT_PIXEL Dot_Pixel)
{
  if (X_Center > sOLED_DIS.OLED_Dis_Column || Y_Center >= sOLED_DIS.OLED_Dis_Page) {
    Serial.print("GUI_DrawCircle Input exceeds the normal display range\r\n");
    return;
  }

  //Draw a circle from(0, R) as a starting point
  int16_t XCurrent, YCurrent;
  XCurrent = 0;
  YCurrent = Radius;

  //Cumulative error,judge the next point of the logo
  int16_t Esp = 3 - (Radius << 1 );

  int16_t sCountY;
  if (Draw_Fill == DRAW_FULL) {
    while (XCurrent <= YCurrent ) { //Realistic circles
      for (sCountY = XCurrent; sCountY <= YCurrent; sCountY ++ ) {
        GUI_DrawPoint(X_Center + XCurrent, Y_Center + sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );             //1
        GUI_DrawPoint(X_Center - XCurrent, Y_Center + sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );             //2
        GUI_DrawPoint(X_Center - sCountY, Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );             //3
        GUI_DrawPoint(X_Center - sCountY, Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );             //4
        GUI_DrawPoint(X_Center - XCurrent, Y_Center - sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );             //5
        GUI_DrawPoint(X_Center + XCurrent, Y_Center - sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );             //6
        GUI_DrawPoint(X_Center + sCountY, Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );             //7
        GUI_DrawPoint(X_Center + sCountY, Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT );
      }
      if (Esp < 0 )
        Esp += 4 * XCurrent + 6;
      else {
        Esp += 10 + 4 * (XCurrent - YCurrent );
        YCurrent --;
      }
      XCurrent ++;
    }
  } else { //Draw a hollow circle
    while (XCurrent <= YCurrent ) {
      GUI_DrawPoint(X_Center + XCurrent, Y_Center + YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );             //1
      GUI_DrawPoint(X_Center - XCurrent, Y_Center + YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );             //2
      GUI_DrawPoint(X_Center - YCurrent, Y_Center + XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );             //3
      GUI_DrawPoint(X_Center - YCurrent, Y_Center - XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );             //4
      GUI_DrawPoint(X_Center - XCurrent, Y_Center - YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );             //5
      GUI_DrawPoint(X_Center + XCurrent, Y_Center - YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );             //6
      GUI_DrawPoint(X_Center + YCurrent, Y_Center - XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );             //7
      GUI_DrawPoint(X_Center + YCurrent, Y_Center + XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );             //0

      if (Esp < 0 )
        Esp += 4 * XCurrent + 6;
      else {
        Esp += 10 + 4 * (XCurrent - YCurrent );
        YCurrent --;
      }
      XCurrent ++;
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
    Serial.print("GUI_DisChar Input exceeds the normal display range\r\n");
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
    Serial.print("GUI_DisString_EN Input exceeds the normal display range\r\n");
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

/********************************************************************************
  function:	Display the string
  parameter:
	Xstart           ：X coordinate
	Ystart           : Y coordinate
	Nummber          : The number displayed
	Font             ：A structure pointer that displays a character size
	Color_Background : Select the background color of the English character
	Color_Foreground : Select the foreground color of the English character
********************************************************************************/
#define  ARRAY_LEN 255
void GUI_DisNum(POINT Xpoint, POINT Ypoint, int Nummber,
                sFONT* Font, COLOR Color_Background, COLOR Color_Foreground )
{

  int16_t Num_Bit = 0, Str_Bit = 0;
  uint8_t Str_Array[ARRAY_LEN] = {0}, Num_Array[ARRAY_LEN] = {0};
  uint8_t *pStr = Str_Array;

  if (Xpoint > sOLED_DIS.OLED_Dis_Column || Ypoint > sOLED_DIS.OLED_Dis_Page) {
    Serial.print("GUI_DisNum Input exceeds the normal display range\r\n");
    return;
  }

  //Converts a number to a string
  while (Nummber) {
    Num_Array[Num_Bit] = Nummber % 10 + '0';
    Num_Bit++;
    Nummber /= 10;
  }

  //The string is inverted
  while (Num_Bit > 0) {
    Str_Array[Str_Bit] = Num_Array[Num_Bit - 1];
    Str_Bit ++;
    Num_Bit --;
  }

  //show
  GUI_DisString_EN(Xpoint, Ypoint, (const char*)pStr, Font, Color_Background, Color_Foreground );
}

/********************************************************************************
  function:	Display the bit map,1 byte = 8bit = 8 points
  parameter:
	Xpoint ：X coordinate
	Ypoint : Y coordinate
	pMap   : Pointing to the picture
	Width  ：Bitmap Width
	Height : Bitmap Height
  note:
	This function is suitable for bitmap, because a 16-bit data accounted for 16 points
********************************************************************************/
void GUI_Disbitmap(POINT Xpoint, POINT Ypoint, const unsigned char *pMap,
                   POINT Width, POINT Height)
{
  POINT i, j, byteWidth = (Width + 7) / 8;
  for (j = 0; j < Height; j++) {
    for (i = 0; i < Width; i ++) {
      if (pgm_read_byte(pMap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
        GUI_DrawPoint(Xpoint + i, Ypoint + j, WHITE, DOT_PIXEL_DFT, DOT_STYLE_DFT);
      }
    }
  }
}

/********************************************************************************
  function:	Display the Gray map,1 byte = 8bit = 2 points
  parameter:
	Xpoint ：X coordinate
	Ypoint : Y coordinate
	pMap   : Pointing to the picture
	Width  ：Bitmap Width
	Height : Bitmap Height
  note:
	This function is suitable for bitmap, because a 4-bit data accounted for 1 points
	Please use the Image2lcd generated array
********************************************************************************/
void GUI_DisGrayMap(POINT Xpoint, POINT Ypoint, const unsigned char *pBmp)
{
  //Get the Map header Gray, width, height
  char Gray;
  Gray = pgm_read_byte(pBmp + 1);
  POINT Height, Width;
  Width = (pgm_read_byte(pBmp + 3) << 8) | (pgm_read_byte(pBmp + 2));
  Height = (pgm_read_byte(pBmp + 5) << 8) | (pgm_read_byte(pBmp + 4));

  POINT i, j;
  if (Gray == 0x04) { //Sixteen gray levels
    pBmp = pBmp + 6;
    for (j = 0; j < Height; j++)
      for (i = 0; i < Width / 2; i++) {
        GUI_DrawPoint(Xpoint + i * 2, Ypoint + j, ~(pgm_read_byte(pBmp) >> 4), DOT_PIXEL_DFT, DOT_STYLE_DFT);
        GUI_DrawPoint(Xpoint + i * 2 + 1, Ypoint + j, ~(pgm_read_byte(pBmp)) , DOT_PIXEL_DFT, DOT_STYLE_DFT);
        pBmp++;
      }
  } else {
    Serial.print("Does not support type\r\n");
    return;
  }
}

/********************************************************************************
  function:	According to the display area adaptive display time
  parameter:
		xStart :   X direction Start coordinates
		Ystart :   Y direction Start coordinates
		Xend   :   X direction end coordinates
		Yend   :   Y direction end coordinates
		pTime  :   Pointer to the definition of the structure
		Color  :   Set show color
  note:
********************************************************************************/
void GUI_Showtime(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend, DEV_TIME *pTime, COLOR Color)
{
  uint8_t value[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
  sFONT *New_Font = NULL;

  //According to the display area adaptive font size
  POINT Dx = (Xend - Xstart) / 7;//Determine the spacing between characters
  POINT Dy = Yend - Ystart;      //determine the font size
  
  if (Dy > Font24.Height) {
    New_Font = &Font24;
  } else if (Dy > Font20.Height && Dy <= Font24.Height) {
    New_Font = &Font20;
  } else if (Dy > Font16.Height && Dy <= Font20.Height) {
    New_Font = &Font16;
  } else if (Dy > Font12.Height && Dy <= Font16.Height) {
    New_Font = &Font12;
  } else if (Dy > Font8.Height && Dy <= Font12.Height) {
    New_Font = &Font8;
  } else {
    DEBUG("Dy =  ");
    DEBUG(Dy + "\r\n");
    DEBUG("Please change the display area size, or add a larger font to modify\r\n");
  }

  //Write data into the cache
  GUI_DisChar(Xstart                           , Ystart, value[pTime->Hour / 10], New_Font, FONT_BACKGROUND, Color);
  GUI_DisChar(Xstart + Dx                      , Ystart, value[pTime->Hour % 10], New_Font, FONT_BACKGROUND, Color);
  GUI_DisChar(Xstart + Dx  + Dx / 4 + Dx / 2   , Ystart, ':'                    , New_Font, FONT_BACKGROUND, Color);
  GUI_DisChar(Xstart + Dx * 2 + Dx / 2         , Ystart, value[pTime->Min / 10] , New_Font, FONT_BACKGROUND, Color);
  GUI_DisChar(Xstart + Dx * 3 + Dx / 2         , Ystart, value[pTime->Min % 10] , New_Font, FONT_BACKGROUND, Color);
  GUI_DisChar(Xstart + Dx * 4 + Dx / 2 - Dx / 4, Ystart, ':'                    , New_Font, FONT_BACKGROUND, Color);
  GUI_DisChar(Xstart + Dx * 5                  , Ystart, value[pTime->Sec / 10] , New_Font, FONT_BACKGROUND, Color);
  GUI_DisChar(Xstart + Dx * 6                  , Ystart, value[pTime->Sec % 10] , New_Font, FONT_BACKGROUND, Color);
}

/********************************************************************************
  function:	OLED_Show
  note:
	OLED Clear,
	Draw Line,
	Draw Rectangle,
	Draw Rings,
	Draw Olympic Rings,
	Display String,
	Show Pic
********************************************************************************/
void GUI_Show(void)
{
  Serial.print("Clear... \r\n");
  OLED_ClearScreen(OLED_BACKGROUND);//OLED_BACKGROUND
  OLED_Display(0, 0, sOLED_DIS.OLED_Dis_Column, sOLED_DIS.OLED_Dis_Page);

  Serial.print("Draw Line \r\n");
  GUI_DrawLine(0, 1, sOLED_DIS.OLED_Dis_Column - 1, 1, WHITE, LINE_SOLID , DOT_PIXEL_DFT);
  GUI_DrawLine(0, 4, sOLED_DIS.OLED_Dis_Column - 1, 4, WHITE, LINE_DOTTED, DOT_PIXEL_DFT);
  GUI_DrawLine(0, sOLED_DIS.OLED_Dis_Page - 5, sOLED_DIS.OLED_Dis_Column - 1, sOLED_DIS.OLED_Dis_Page - 5, WHITE, LINE_DOTTED, DOT_PIXEL_DFT);
  GUI_DrawLine(0, sOLED_DIS.OLED_Dis_Page - 1, sOLED_DIS.OLED_Dis_Column - 1, sOLED_DIS.OLED_Dis_Page - 1, WHITE, LINE_SOLID , DOT_PIXEL_2X2);

  Serial.print("Draw Rectangle \r\n");
  GUI_DrawRectangle(5 , 7 , sOLED_DIS.OLED_Dis_Column - 5 , sOLED_DIS.OLED_Dis_Page - 7, WHITE, DRAW_EMPTY, DOT_PIXEL_DFT);
  GUI_DrawRectangle(10, 10, sOLED_DIS.OLED_Dis_Column - 10, 20                         , WHITE, DRAW_FULL , DOT_PIXEL_DFT);

  Serial.print("Draw Rings\r\n");
  GUI_DrawCircle(10, 30, 3, WHITE, DRAW_FULL , DOT_PIXEL_DFT);
  GUI_DrawCircle(10, 40, 3, WHITE, DRAW_EMPTY, DOT_PIXEL_DFT);
  GUI_DrawCircle(10, 50, 3, WHITE, DRAW_FULL , DOT_PIXEL_DFT);
  GUI_DrawCircle(sOLED_DIS.OLED_Dis_Column - 10, 30, 3, WHITE, DRAW_FULL , DOT_PIXEL_DFT);
  GUI_DrawCircle(sOLED_DIS.OLED_Dis_Column - 10, 40, 3, WHITE, DRAW_EMPTY, DOT_PIXEL_DFT);
  GUI_DrawCircle(sOLED_DIS.OLED_Dis_Column - 10, 50, 3, WHITE, DRAW_FULL , DOT_PIXEL_DFT);

  Serial.print("Draw Olympic Rings\r\n");
  uint16_t Cx1 = 35, Cy1 = 85, Cr = 12;
  uint16_t Cx2 = Cx1 + (2.5 * Cr), Cy2 = Cy1;
  uint16_t Cx3 = Cx1 + (5 * Cr), Cy3 = Cy1;
  uint16_t Cx4 = (Cx1 + Cx2) / 2, Cy4 = Cy1 + Cr;
  uint16_t Cx5 = (Cx2 + Cx3) / 2, Cy5 = Cy1 + Cr;

  GUI_DrawCircle(Cx1, Cy1, Cr, WHITE, DRAW_EMPTY, DOT_PIXEL_DFT);
  GUI_DrawCircle(Cx2, Cy2, Cr, WHITE, DRAW_EMPTY, DOT_PIXEL_DFT);
  GUI_DrawCircle(Cx3, Cy3, Cr, WHITE, DRAW_EMPTY, DOT_PIXEL_DFT);
  GUI_DrawCircle(Cx4, Cy4, Cr, WHITE, DRAW_EMPTY, DOT_PIXEL_DFT);
  GUI_DrawCircle(Cx5, Cy5, Cr, WHITE, DRAW_EMPTY, DOT_PIXEL_DFT);

  Serial.print("Display String \r\n");
  GUI_DisString_EN(30, 25, "WaveShare"   , &Font12, FONT_BACKGROUND, WHITE);
  GUI_DisString_EN(28, 35, "Electronic"  , &Font12, FONT_BACKGROUND, WHITE);
  GUI_DisString_EN(18, 45, "1.5inch OLED", &Font12, FONT_BACKGROUND, WHITE);

  OLED_Display(0, 0, sOLED_DIS.OLED_Dis_Column, sOLED_DIS.OLED_Dis_Page);
  Driver_Delay_ms(2000);
}

