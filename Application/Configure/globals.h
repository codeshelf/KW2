#include "eeprom.h"


// ------------------------------------------------------------
// ------------------ Global Values ---------------------------
// ------------------------------------------------------------

uint8_t guid[EEPROM_GUID_LEN + 1];		// space for null termination
uint8_t hw_ver[EEPROM_HW_VER_LEN + 2];	// space for period and null termination
uint8_t aes_key[EEPROM_AES_KEY_LEN];
uint8_t trim[EEPROM_TUNING_LEN ];
