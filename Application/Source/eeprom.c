#include "eeprom.h"
#include "PE_Types.h"
#include "IO_Map.h"
#include "string.h"
#include "SPI_PDD.h"

void spiSendByte(uint8_t data) {

	// Wait until TX buffer empty.
	while (SPI_PDD_GetTxFIFOCounter(Eeprom_DEVICE) > 1)
		;
	SPI_PDD_WriteData8Bits(Eeprom_DEVICE, data);
	Wait_Waitns(700);

}

uint8_t spiGetByte() {
	uint8_t result = 0;

	// Wait until RX buffer not empty.
	while (SPI_PDD_GetRxFIFOCounter(Eeprom_DEVICE) == 0)
		;
	result = SPI_PDD_ReadData8bits(Eeprom_DEVICE);
	Wait_Waitns(700);

	return result;
}

void readEepromData(uint16_t addr, uint8_t* dataPtr, uint8_t bytesToRead) {
	EEPROM_CS_ON

	spiSendByte(READ_CMD);
	spiSendByte(addr >> 8);
	spiSendByte(addr & 0xFF);
	for(uint8_t i = 0; i < bytesToRead; i++) {
	//	spiSendByte(0xff);
		dataPtr[i] = spiGetByte();
	}
	
	//spiSendByte(0xff);
	//spiSendByte(0xff);
	//spiSendByte(0xff);
	//spiSendByte(0xff);
	//spiSendByte(0xff);
	//dataPtr[0] = spiGetByte();
	
	EEPROM_CS_OFF
}

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
}
