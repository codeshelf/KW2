#include "eeprom.h"

extern byte g_lastChannel;
extern byte g_restartCause;
extern byte g_restartData;
extern byte g_lastLQI;

// ------------------------------------------------------------
// ------------------ Global Values ---------------------------
// ------------------------------------------------------------

uint8_t g_guid[EEPROM_GUID_LEN + 1];		// space for null termination
uint8_t g_hw_ver[EEPROM_HW_VER_LEN + 2];	// space for period and null termination
uint8_t g_aes_key[EEPROM_AES_KEY_LEN];
uint8_t g_trim[EEPROM_TUNING_LEN];


