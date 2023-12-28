#include <Arduino.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <PN532_debug.h>
#include <connection.h>
#include <wrappers.h>

#define DBG_SERIAL false
#define SPICEAPI_PASSWORD ""
#define SPICEAPI_WRAPPER_BUFFER_SIZE 256
#define SPICEAPI_WRAPPER_BUFFER_SIZE_STR 256
#define SPICEAPI_INTERFACE Serial

spiceapi::Connection CON(512, SPICEAPI_PASSWORD);

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

uint8_t        _prevIDm[8];
unsigned long  _prevTime;

void setup(void)
{
  #if DBG_SERIAL
  Serial.begin(115200);
  #endif

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata)
  {
    #if DBG_SERIAL
    Serial.print("Didn't find PN53x board");
    #endif
    while (1) {delay(10);};      // halt
  }

  #if DBG_SERIAL
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
  #endif

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();

  memset(_prevIDm, 0, 8);

  SPICEAPI_INTERFACE.begin(115200);
  while (!SPICEAPI_INTERFACE);
}

void loop(void)
{
  uint8_t ret;
  uint16_t systemCode = 0xFFFF;
  uint8_t requestCode = 0x01;
  uint8_t idm[8];
  uint8_t pmm[8];
  uint16_t systemCodeResponse;

  #if DBG_SERIAL
  Serial.print("Waiting for a FeliCa card... ");
  #endif
  ret = nfc.felica_Polling(systemCode, requestCode, idm, pmm, &systemCodeResponse, 5000);

  if (ret != 1)
  {
    #if DBG_SERIAL
    Serial.println("Could not find a card");
    #endif
    delay(500);
    return;
  }

  if ( memcmp(idm, _prevIDm, 8) == 0 ) {
    if ( (millis() - _prevTime) < 3000 ) {
      #if DBG_SERIAL
      Serial.println("Same card");
      #endif
      delay(100);
      return;
    }
  }

  char card_id [17];
  for (int i = 0; i < 8; ++i) {
    sprintf (card_id + (i*2), "%02X", idm[i]); 
  }
  
  #if DEBUG_PRINT
  Serial.print("The card ID is: ");
  Serial.print(card_id);
  Serial.print("\n");
  #endif

  memcpy(_prevIDm, idm, 8);
  _prevTime = millis();
  
  spiceapi::card_insert(CON, 0, card_id);

  delay(100);
}