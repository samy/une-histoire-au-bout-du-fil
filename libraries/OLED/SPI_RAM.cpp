/******************************************************************************
***************************Intermediate driver layer***************************
* | file      	:	SPI_RAM.cpp
* |	version		:	V1.0
* | date		:	2017 - 12 - 12
* | function	:	23K256 DRIVER
******************************************************************************/
#include "DEV_Config.h"
#include "SPI_RAM.h"

/*******************************************************************************
function:
		// Mode handling
*******************************************************************************/
static void Set_Mode(BYTE mode)
{
    SPIRAM_CS_0;

    SPI4W_Write_Byte(CMD_WRSR);
    SPI4W_Write_Byte(mode);

    SPIRAM_CS_1;
}

/*******************************************************************************
function:
		// Write and read byte
*******************************************************************************/
BYTE SPIRAM_RD_Byte(WORD Addr)
{
    BYTE RD_Byte;

    // Set byte mode
    Set_Mode(BYTE_MODE);

    SPIRAM_CS_0;
    SPI4W_Write_Byte(CMD_READ);
    SPI4W_Write_Byte((BYTE)(Addr >> 8));
    SPI4W_Write_Byte((BYTE)Addr);
    RD_Byte = SPI4W_Read_Byte(0xFF);
    SPIRAM_CS_1;

    return RD_Byte;
}

void SPIRAM_WR_Byte(WORD Addr, BYTE Data)
{
    // Set byte mode
    Set_Mode(BYTE_MODE);

    // Write Addr,data
    SPIRAM_CS_0;
    SPI4W_Write_Byte(CMD_WRITE);
    SPI4W_Write_Byte((BYTE)(Addr >> 8));
    SPI4W_Write_Byte((BYTE)Addr);
    SPI4W_Write_Byte(Data);
    SPIRAM_CS_1;
}


/*******************************************************************************
function:
		// Page transfer functions. Bound to current page. Passing the boundary
		//  will wrap to the beginning
*******************************************************************************/
void SPIRAM_RD_Page(WORD Addr, BYTE *pBuf)
{
    WORD i;

    // Set byte mode
    Set_Mode(PAGE_MODE);

    // Write Addr, read data
    SPIRAM_CS_0;
    SPI4W_Write_Byte(CMD_READ);
    SPI4W_Write_Byte((BYTE)(Addr >> 8));
    SPI4W_Write_Byte((BYTE)Addr);
    for (i = 0; i < 32; i++) {
        *pBuf = SPI4W_Read_Byte(0xFF);
        pBuf++;
    }
    SPIRAM_CS_1;
}

void SPIRAM_WR_Page(WORD Addr, BYTE *pBuf)
{
    WORD i;

    // Set byte mode
    Set_Mode(PAGE_MODE);

    // Write Addr, read data
    SPIRAM_CS_0;
    SPI4W_Write_Byte(CMD_WRITE);
    SPI4W_Write_Byte((BYTE)(Addr >> 8));
    SPI4W_Write_Byte((BYTE)Addr);
    for (i = 0; i < 32; i++) {
        SPI4W_Write_Byte(*pBuf);
        pBuf++;
    }
    SPIRAM_CS_1;
}

/*******************************************************************************
function:
		// Write and read Len
*******************************************************************************/
void SPIRAM_RD_Stream(WORD Addr, BYTE *pBuf, WORD Len)
{
    WORD i;

    // Set byte mode
    Set_Mode(STREAM_MODE);

    // Write Addr, read data
    SPIRAM_CS_0;
    SPI4W_Write_Byte(CMD_READ);
    SPI4W_Write_Byte((BYTE)(Addr >> 8));
    SPI4W_Write_Byte((BYTE)Addr);
    for (i = 0; i < Len; i++) {
        *pBuf = SPI4W_Read_Byte(0xFF);
        pBuf++;
    }
    SPIRAM_CS_1;
}

void SPIRAM_WR_Stream(WORD Addr, BYTE *pBuf, WORD Len)
{
    WORD i;

    // Set byte mode
    Set_Mode(STREAM_MODE);

    // Write Addr, read data
    SPIRAM_CS_0;
    SPI4W_Write_Byte(CMD_WRITE);
    SPI4W_Write_Byte((BYTE)(Addr >> 8));
    SPI4W_Write_Byte((BYTE)Addr);
    for (i = 0; i < Len; i++) {
        SPI4W_Write_Byte(*pBuf);
        pBuf++;
    }
    SPIRAM_CS_1;
}
