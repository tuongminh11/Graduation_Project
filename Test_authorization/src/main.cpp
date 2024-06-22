// #include <WiFiManager.h> // wifi manager
#include <WiFi.h>
#include <MicroOcpp.h> //OCPP library
#include <SPI.h>       //SPI communication
#include <MFRC522.h>   // RC522 module
#include "header.h"
//-----------------------------------------------------------

//-----------------------------------------------------------
MFRC522 mfrc522(SS_PIN, RST); // class RFID reader
MFRC522::MIFARE_Key key;
//-----------------------------------------------------------
uint8_t connectorStatus[2];
float measure_buffer;
//-----------------------------------------------------------
String idTag;
//-----------------------------------------------------------
void setup()
{
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.print(F("[main] Wait for WiFi: "));

  // init RFID reader
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  SPI.begin();                       // Init SPI bus
  mfrc522.PCD_Init();                // Init MFRC522 (PCD is terminology)
  mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details

  // wait for WiFi connection
  WiFi.begin(STASSID, STAPSK);
  while (!WiFi.isConnected())
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(F("Connected!"));
  // init and config OCPP
  mocpp_initialize(OCPP_BACKEND_URL, OCPP_CHARGE_BOX_ID, "Wallnut Charging Station New", "EVSE-iPAC-New");
}

/*
 * Handle RFID card
 * If EVSE is not logged in -> authorization to OCPP and response to user
 * Else -> notify logged in
 */
void readPICC()
{
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  byte sector = 1;
  byte blockAddr = 4;
  byte trailerBlock = 7;
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(F(mfrc522.GetStatusCodeName(status)));
    return;
  }

  status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(F(mfrc522.GetStatusCodeName(status)));
  }

  // check if user is full
  idTag = (char *)buffer;
  for(int i = 0; i < 18; i++) {
    Serial.printf("%02x ", buffer[i]);
  }
  Serial.println();
  // check if this is exist user
  Serial.println(idTag);
  Serial.println(sizeof(idTag));  
  authorize(idTag.c_str(), [](JsonObject payload) -> void
            {
      JsonObject idTagInfo = payload["idTagInfo"];
      uint8_t data[] = {uint8_t(ID_TAG), 0x00, 0x00, 0x00, 0x01};
      if (strcmp("Accepted", idTagInfo["status"] | "UNDEFINED")) { //strcmp == 0 mean equal
        
        Serial.println(F("authorize 1 reject"));
      }
      else {
        Serial.println(F("authorize 1 success"));
      } }, nullptr, nullptr, nullptr);

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD prepare to next read
  mfrc522.PCD_StopCrypto1();
}

void loop()
{
  readPICC();
  mocpp_loop();
}