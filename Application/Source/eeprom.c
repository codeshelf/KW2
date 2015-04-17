#include "eeprom.h"
#include "PE_Types.h"
#include "IO_Map.h"
#include "string.h"
#include "SPI_PDD.h"
#include "commands.h"


#define WAIT_SPI0_TRANSMISSON_END(count) {while ((SPI0_SR & SPI_SR_RXCTR_MASK) < (count<<SPI_SR_RXCTR_SHIFT)) {};}

/*
 * Will get one byte from the eeprom.
 * 
 * Assumes pattern: request one byte, read one byte, request one byte...
 * 
 * If a different pattern is desired you'll have to move the WAIT_SPI0_TRANSMISSION_END(n)
 * out and use it to wait for incoming data like the following:
 * 
 * spiSendByte(data)
 * spiSendByte(data)
 * spiSendByte(data)
 * WAIT_SPI0_TRANSMISSON_END(3);
 * rcv1 = spiGetByte();
 * rcv2 = spiGetByte();
 * rcv3 = spiGetByte();
 */
void spiSendByte(uint8_t data) {
	// Wait until TX buffer empty.
	while (SPI_PDD_GetTxFIFOCounter(Eeprom_DEVICE) > 1)
		;
	SPI_PDD_WriteData8Bits(Eeprom_DEVICE, data);
	
	WAIT_SPI0_TRANSMISSON_END(1);
}

uint8_t spiGetByte() {
	uint8_t result;

	// Wait until RX buffer not empty.
	while (SPI_PDD_GetRxFIFOCounter(Eeprom_DEVICE) == 0)
		;
	result = SPI_PDD_ReadData8bits(Eeprom_DEVICE);

	return result;
}

/*
 * Will read as many bytes as requested. Does not loop on page end.
 * 
 */
void readEepromData(uint16_t addr, uint8_t* dataPtr, uint8_t bytesToRead) {
	gwUINT8 ccrHolder;
	
	GW_ENTER_CRITICAL(ccrHolder);
	
	EEPROM_CS_ON
	
	spiSendByte(READ_CMD);
	spiGetByte();
	spiSendByte(addr >> 8);
	spiGetByte();
	spiSendByte( addr & 0xFF);
	spiGetByte();
	
	for(uint8_t i = 0; i < bytesToRead; i++) {
		spiSendByte(0xff);
		dataPtr[i] = spiGetByte();
	}
	
	EEPROM_CS_OFF
	GW_EXIT_CRITICAL(ccrHolder);
}


/*
 * Need to be careful not to write beyond page. Will loop and overwrite
 * data in current page.
 * 
 * Physical page boundaries start at addresses that are integer multiples 
 * of the page buffer size (32 bytes) (or ‘page size’) and, end at 
 * addresses that are integer multiples of page size – 1
 */
void writeEepromData(uint16_t addr, uint8_t* dataPtr, uint8_t bytesToWrite) {
	
	EEPROM_CS_ON
	spiSendByte(WRITE_ENABLE_CMD);
	EEPROM_CS_OFF
	EEPROM_CS_ON

	spiSendByte(WRITE_CMD);
	spiSendByte(addr >> 8);
	spiSendByte(addr & 0xff);
	
	for(uint8_t i = 0; i < bytesToWrite; i++) {
		spiSendByte(dataPtr[i]);
	}
	
	EEPROM_CS_OFF
	
	// Wait for internal write cycle time (TWC)
	Wait_Waitns(EEPROM_TWC);
}

void writeAESKey(uint8_t* dataPtr) {
	writeEepromData((uint16_t)EEPROM_AES_KEY_ADDR, dataPtr, (uint8_t)EEPROM_AES_KEY_LEN);
}

void readAESKey(uint8_t* dataPtr) {
	readEepromData((uint16_t)EEPROM_AES_KEY_ADDR, dataPtr, (uint8_t)EEPROM_AES_KEY_LEN);
}

void writeHWVersion(uint8_t* dataPtr) {
	writeEepromData((uint16_t)EEPROM_HW_VER_ADDR, dataPtr, (uint8_t)EEPROM_HW_VER_LEN);
}

void readHWVersion(uint8_t* dataPtr) {
	readEepromData((uint16_t)EEPROM_HW_VER_ADDR, dataPtr, (uint8_t)EEPROM_HW_VER_LEN);
}

void writeGuid(uint8_t* dataPtr) {
	writeEepromData((uint16_t)EEPROM_GUID_ADDR, dataPtr, (uint8_t)EEPROM_GUID_LEN);
}

void readGuid(uint8_t* dataPtr) {
	readEepromData((uint16_t)EEPROM_GUID_ADDR, dataPtr, (uint8_t)EEPROM_GUID_LEN);
}

void writeTuning(uint8_t* dataPtr) {
	writeEepromData((uint16_t)EEPROM_TUNING_ADDR, dataPtr, (uint8_t)EEPROM_TUNING_LEN);
}

void readTuning(uint8_t* dataPtr) {
	readEepromData((uint16_t)EEPROM_TUNING_ADDR, dataPtr, (uint8_t)EEPROM_TUNING_LEN);
}
