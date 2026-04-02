/*
  Simple PN5180 Reader Motion Testing
  Used on ESP32 WROOM 38pin
  Geoff Franklin
  March 27, 2026

  Use library: https://github.com/tueddy/PN5180-Library
*/

#include <PN5180.h>
#include <PN5180ISO15693.h>

// Define PN5180 NFC Readers
#define RST 25
#define NSS 21
#define BUSY 22

PN5180ISO15693 reader(NSS, BUSY, RST);
PN5180 pn5180(NSS, BUSY, RST);

void setup() {
  // Serial interface for debugging only
  Serial.begin(115200);
  while (!Serial);        // Wait for serial
  delay(2000);            // Wait more for serial
  
  Serial.println("\nPN5180 Speed Test " );
  Serial.println("Geoff Franklin" );
  Serial.println(__FILE__);
  Serial.println(__DATE__);    
  Serial.println();

  pn5180.begin();
  delay(100);

  // get version
  Serial.print("PN5180 Version: ");
  uint8_t productVersion[2];
  pn5180.readEEprom(PRODUCT_VERSION, productVersion, sizeof(productVersion));
  uint8_t firmwareVersion[2];
  pn5180.readEEprom(FIRMWARE_VERSION, firmwareVersion, sizeof(firmwareVersion));
  uint8_t eepromVersion[2];
  pn5180.readEEprom(EEPROM_VERSION, eepromVersion, sizeof(eepromVersion));
  Serial.printf("Product: %d.%d   Firmware: %d.%d   EEPROM: %d.%d\n", productVersion[1], productVersion[0], firmwareVersion[1], firmwareVersion[0], eepromVersion[1], eepromVersion[0]);

  Serial.println("Setup Reader");
  reader.begin();
  reader.reset();
  reader.setupRF();
  Serial.println("Ready");
}

void showIRQStatus(uint32_t irqStatus) {
  Serial.print(F("IRQ-Status 0x"));
  Serial.print(irqStatus, HEX);
  Serial.print(": [ ");
  if (irqStatus & (1<< 0)) Serial.print(F("RQ "));
  if (irqStatus & (1<< 1)) Serial.print(F("TX "));
  if (irqStatus & (1<< 2)) Serial.print(F("IDLE "));
  if (irqStatus & (1<< 3)) Serial.print(F("MODE_DETECTED "));
  if (irqStatus & (1<< 4)) Serial.print(F("CARD_ACTIVATED "));
  if (irqStatus & (1<< 5)) Serial.print(F("STATE_CHANGE "));
  if (irqStatus & (1<< 6)) Serial.print(F("RFOFF_DET "));
  if (irqStatus & (1<< 7)) Serial.print(F("RFON_DET "));
  if (irqStatus & (1<< 8)) Serial.print(F("TX_RFOFF "));
  if (irqStatus & (1<< 9)) Serial.print(F("TX_RFON "));
  if (irqStatus & (1<<10)) Serial.print(F("RF_ACTIVE_ERROR "));
  if (irqStatus & (1<<11)) Serial.print(F("TIMER0 "));
  if (irqStatus & (1<<12)) Serial.print(F("TIMER1 "));
  if (irqStatus & (1<<13)) Serial.print(F("TIMER2 "));
  if (irqStatus & (1<<14)) Serial.print(F("RX_SOF_DET "));
  if (irqStatus & (1<<15)) Serial.print(F("RX_SC_DET "));
  if (irqStatus & (1<<16)) Serial.print(F("TEMPSENS_ERROR "));
  if (irqStatus & (1<<17)) Serial.print(F("GENERAL_ERROR "));
  if (irqStatus & (1<<18)) Serial.print(F("HV_ERROR "));
  if (irqStatus & (1<<19)) Serial.print(F("LPCD "));
  Serial.print("]");
}

void loop() {
  static int readCount = 0;           // number of consecutive valid reads
  static int sequence = 1;        
  static unsigned long firstSeenTime; // start time of a series of good reads  
  uint8_t uid[10];                    // 8-byte integer array to store UID of detected tag

  unsigned long commandStartTime = millis();
  reader.commandTimeout = 20;         // reduce the timeout
  int code =reader.getInventory(uid); // tag read function
  reader.commandTimeout = 500;        // restore the timeout
  unsigned long ElapsedTime = (millis() - commandStartTime);  

  if ( (ElapsedTime > 40)) {          // time out, failed read
      Serial.printf("Failed Read, code: %d, ", code);
      showIRQStatus(reader.getIRQStatus());
      Serial.printf(", Read Time %d mSec.\n", ElapsedTime);
      commandStartTime = millis();
      reader.reset();                 // reset the reader after failed read
      reader.setupRF();
      ElapsedTime = (millis() - commandStartTime);    
      Serial.printf("Reset Time, %d mSec.\n", ElapsedTime);
      return;
  }
  
  if (code == ISO15693_EC_OK) {       // successful read
    readCount++;  
    if (readCount==1) {               // first successful read?
      firstSeenTime = millis();
      static char UID[50];
      sprintf(UID, "%02X %02X %02X %02X %02X %02X %02X %02X", uid[7],uid[6],uid[5],uid[4],uid[3],uid[2],uid[1],uid[0]);
      Serial.printf("Sequence: %d, UID: %s, Read Time: %d mSec.\n", sequence++, UID, ElapsedTime);
    }  
  } 
  
  else {                            // No tag in range
    if (readCount>0) {
      unsigned long timeSeen = (millis() - firstSeenTime);    
      Serial.printf("Total Time Seen: %d mSec. reads: %d\n\n", timeSeen, readCount);                 
      readCount = 0;                // reset
    }
  }  

  delay(1);
}
