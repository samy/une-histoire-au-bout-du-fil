/******************************************************************************
***************************Intermediate driver layer***************************
* | file      	:	SPI_RAM.h
* |	version		:	V1.0
* | date		:	2017 - 12 - 12
* | function	:	23K256 DRIVER
******************************************************************************/
#ifndef __SPI_RAM_H_
#define __SPI_RAM_H_

#include "DEV_Config.h"

#define	BYTE				uint8_t
#define	WORD				uint16_t

// SRAM opcodes
//#define CMD_WREN  0x06
//#define CMD_WRDI  0x04
#define CMD_RDSR  0x05
#define CMD_WRSR  0x01
#define CMD_READ  0x03
#define CMD_WRITE 0x02

// SRAM Hold line override
#define HOLD 1

// SRAM modes
#define BYTE_MODE (0x00 | HOLD)
#define PAGE_MODE (0x80 | HOLD)
#define STREAM_MODE (0x40 | HOLD)

BYTE SPIRAM_RD_Byte(WORD Addr);
void SPIRAM_WR_Byte(WORD Addr, BYTE Data);
void SPIRAM_RD_Page(WORD Addr, BYTE *pBuf);
void SPIRAM_WR_Page(WORD Addr, BYTE *pBuf);
void SPIRAM_RD_Stream(WORD Addr, BYTE *pBuf, WORD Len);
void SPIRAM_WR_Stream(WORD Addr, BYTE *pBuf, WORD Len);
#endif
