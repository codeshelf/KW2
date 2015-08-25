#include "eeprom.h"
#include "gwTypes.h"
#include "commandTypes.h"

extern byte 		gLastChannel;
extern byte			gLastLQI;

// --------------- Restart Debug Values ------------------------
extern byte 		gRestartCause;
extern byte 		gRestartData[2];
extern uint32_t		gProgramCounter;

// ------------------- Global Values ---------------------------

uint8_t gGuid[EEPROM_GUID_LEN + 1];		// space for null termination
uint8_t gHwVer[EEPROM_HW_VER_LEN + 2];	// space for period and null termination
uint8_t gAesKey[EEPROM_AES_KEY_LEN];
uint8_t gTrim[EEPROM_TUNING_LEN];
