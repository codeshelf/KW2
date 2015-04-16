/*
 * Display.h
 *
 *  Created on: Jun 12, 2014
 *      Author: jeffw
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include "PE_Types.h"
#include "EepromCS.h"
#include "Wait.h"
#include "SPI_PDD.h"

#define Eeprom_DEVICE 		SPI0_BASE_PTR

#define READ_CMD			0b00000011
#define WRITE_CMD			0b00000010
#define READSRS_CMD			0b00000101
#define WRITESRS_CMD		0b00000001
#define WRITE_ENABLE_CMD	0b00000110
#define WRITE_DISABLE_CMD	0b00000100

// Read 1 byte from RX FIFO and clear RXFIFO to fix RXFIFO pointer error
#define EEPROM_CS_ON     	EepromCS_ClrVal(); SPI_PDD_ReadData8bits(Eeprom_DEVICE); SPI_PDD_ClearRxFIFO(Eeprom_DEVICE);
#define EEPROM_CS_OFF    	Wait_Waitus(5); EepromCS_SetVal();

#define EEPROM_PAGE_SIZE	32			// bytes
#define EEPROM_TWC			50000000	// ns
#define SPI_READ_DELAY		10000000	// ns

// Length of values stored in EEPROM in bytes
#define EEPROM_AES_KEY_LEN		16
#define EEPROM_GUID_LEN			8
#define EEPROM_HW_VER_LEN		8
#define EEPROM_TUNING_LEN		1

// EEPROM memory locations
#define EEPROM_AES_KEY_ADDR		0
#define EEPROM_GUID_ADDR		EEPROM_AES_KEY_ADDR + EEPROM_AES_KEY_LEN
#define EEPROM_HW_VER_ADDR		EEPROM_GUID_ADDR + EEPROM_GUID_LEN
#define EEPROM_TUNING_ADDR		EEPROM_HW_VER_ADDR + EEPROM_HW_VER_LEN

void spiSendByte(uint8_t data);
uint8_t spiGetByte(void);
void readEepromData(uint16_t addr, uint8_t* dataPtr, uint8_t bytesToRead);
void writeEepromData(uint16_t addr, uint8_t* dataPtr, uint8_t bytesToWrite);

void writeAESKey(uint8_t* dataPtr);
void readAESKey(uint8_t* dataPtr);
void writeHWVersion(uint8_t* dataPtr);
void readHWVersion(uint8_t* dataPtr);
void writeGuid(uint8_t* dataPtr);
void readGuid(uint8_t* dataPtr);

#endif /* EEPROM_H_ */
