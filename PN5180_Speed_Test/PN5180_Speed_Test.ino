/*
  Simple PN5180 Reader Motion Testing
  Used on ESP32 WROOM 38pin
  Geoff Franklin
  March 27, 2026

  Use library: https://github.com/tueddy/PN5180-Library
*/

#include <PN5180ISO15693.h>

// Define PN5180 NFC Readers
#define RST 25
#define NSS 21
#define BUSY 22

PN5180ISO15693 pn5180(NSS, BUSY, RST);

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

  Serial.println("Setup Reader");
  pn5180.begin();
  pn5180.reset();
  pn5180.setupRF();
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
  unsigned long commandTime = millis();
  static int readCount = 0;       // number of consecutive valid reads
  static int sequence = 0;
  static unsigned long startTime; // start time of a series of good reads
    
  uint8_t uid[10];                // 8-byte integer array to store UID of detected tag

  pn5180.commandTimeout = 50;     // reduce the timeout
  int code =pn5180.getInventory(uid);
  pn5180.commandTimeout = 500;    // restore the timeout

  unsigned long ElapsedTime = (millis() - commandTime);  

  if ( (ElapsedTime > 49)) {      // time out, failed read
      Serial.printf("Failed Read, %d mSec. code: %d, ", ElapsedTime, code);
      showIRQStatus(pn5180.getIRQStatus());
      Serial.println();
      pn5180.reset();             // reset the reader after failed read
      pn5180.setupRF();
  }
  if (code == ISO15693_EC_OK) {   // successful read
    readCount++;  
    if (readCount==1) {           // first successful read?
      startTime = millis();

      // If tag was found, print its ID to the serial monitor
      sequence++;
      static char UID[50];
      sprintf(UID, "%02X %02X %02X %02X %02X %02X %02X %02X", uid[7],uid[6],uid[5],uid[4],uid[3],uid[2],uid[1],uid[0]);
      Serial.printf("Sequence: %d, UID: %s\n", sequence, UID);
    }  
  } 
  
  else {                        // No tag in range
    if (readCount>0) {
      unsigned long ElapsedTime = (millis() - startTime);    
      Serial.printf("%d mSec. reads: %d\n\n", ElapsedTime, readCount);                 
      readCount = 0;            // reset
    }
  }  

  delay(1);
}
