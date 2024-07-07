// #include <WiFiManager.h> // wifi manager
#include <WiFi.h>
#include <MicroOcpp.h> //OCPP library
#include <SPI.h>       //SPI communication
#include <MFRC522.h>   // RC522 module
#include "header.h"
//-----------------------------------------------------------
MFRC522 mfrc522(SS_PIN, RST); // class RFID reader
MFRC522::MIFARE_Key key;
//-----------------------------------------------------------
// Declare task
TaskHandle_t OCPP_Server;
TaskHandle_t CIMS;
//-----------------------------------------------------------
// Variable save temporary state of system
// uint8_t internetStatus = 0x00;
// uint8_t cimsChargeStatus = 0x00;
// uint8_t hmiStatus = 0x00;
// uint8_t plcStatus = 0x00;
// uint8_t slaveStatus[5];
uint8_t connectorStatus[2];
struct User
{
  char idTag[IDTAG_LEN_MAX + 1];
  uint8_t connectorID;
};
struct User userDB[2] = {{"", 255}, {"", 255}};
uint8_t isAuth = 0x00;
char idTag[16];
float buf_SoH = 0;
float buf_SoKM = 0;
float buf_SoC = 0;
float buf_InitSoC = 0;
float buf_InitU = 0;
float buf_InitI = 0;
float buf_U = 0;
float buf_I = 0;
float buf_SoT = 0;
//-----------------------------------------------------------

void OCPP_Server_handle(void *pvParameters);
void CIMS_handle(void *pvParameters);
void sendData(uint8_t data[5]);
void setup()
{
  xTaskCreatePinnedToCore(
      OCPP_Server_handle, /* Task function. */
      "OCPP_Server",      /* name of task. */
      10000,              /* Stack size of task */
      NULL,               /* parameter of the task */
      1,                  /* priority of the task */
      &OCPP_Server,       /* Task handle to keep track of created task */
      0);                 /* pin task to core 0 */
  delay(500);
  xTaskCreatePinnedToCore(
      CIMS_handle, /* Task function. */
      "CIMS",      /* name of task. */
      10000,       /* Stack size of task */
      NULL,        /* parameter of the task */
      1,           /* priority of the task */
      &CIMS,       /* Task handle to keep track of created task */
      1);          /* pin task to core 1 */
  delay(500);
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
  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD prepare to next read
  mfrc522.PCD_StopCrypto1();

  if (!isAuth && status == MFRC522::STATUS_OK)
  {
    memcpy(idTag, buffer, 16);
    // buffer
    uint8_t data[] = {uint8_t(ID_TAG), 0x00, 0x00, 0x00, 0x01};
    // authorize
    authorize(idTag, [](JsonObject payload) -> void
              {
      JsonObject idTagInfo = payload["idTagInfo"];
      if (strcmp("Accepted", idTagInfo["status"] | "UNDEFINED")) { //strcmp == 0 mean equal
        isAuth = 0;
        Serial.println(F("authorize reject"));
      }
      else {
        isAuth = 1;
        Serial.println(F("authorize success"));
      } }, nullptr, nullptr, nullptr);

    Serial.println(idTag);
    // check if this is exist user
    if (strcmp(idTag, userDB[0].idTag) == 0)
    {
      // hanlde
      Serial.print(F("exist user 1, connectID: "));
      Serial.println(userDB[0].connectorID);
      data[3] = userDB[0].connectorID;
      data[4] = 1;
      sendData(data);
    }
    else if (strcmp(idTag, userDB[1].idTag) == 0)
    {
      // hanlde
      Serial.print(F("exist user 2, connectID: "));
      Serial.println(userDB[1].connectorID);
      data[3] = userDB[1].connectorID;
      data[4] = 1;
      sendData(data);
    }
    // add new user
    else
    {
      if (userDB[0].connectorID == 255)
      {
        // hanlde
        Serial.println(F("new user 1"));
        memcpy(userDB[0].idTag, idTag, 16);
        data[3] = userDB[0].connectorID;
        data[4] = 1;
        sendData(data);
      }
      else if (userDB[1].connectorID == 255)
      {
        // hanlde
        Serial.println(F("new user 2"));
        memcpy(userDB[1].idTag, idTag, 16);
        data[3] = userDB[1].connectorID;
        data[4] = 1;
        sendData(data);
      }
      else
      {
        Serial.println(F("All connectors is full"));
        return;
      }
    }
  }
  else
  {
    Serial.println(F("Already logged in"));
  }
}

// handle request start/end transaction from hmi 0x18
void hmiTranControl(uint8_t buffer[8])
{
  uint8_t data[] = {uint8_t(TRANSACTION_CONFIRMATION), 0x00, 0x00, buffer[5], 0x01};
  if (!buffer[6])
  {
    if (getTransaction(buffer[5]))
    {
      // memcpy(userDB[1].idTag, idTag, 16);
      if (userDB[0].connectorID == buffer[5])
      {
        memcpy(idTag, userDB[0].idTag, 16);
      }
      else if (userDB[0].connectorID == buffer[5])
      {
        memcpy(idTag, userDB[1].idTag, 16);
      }

      if (endTransaction(idTag, nullptr, buffer[5]))
      {
        sendData(data); // ack
      }
      else
      {
        data[2] = 0xFF;
        sendData(data); // ack
      }
      Serial.println(F("Logout"));
    }
  }
  else
  {
    if (!getTransaction(buffer[5]))
    {
      sendRequest("DataTransfer", [buffer]() -> std::unique_ptr<DynamicJsonDocument>
                  {
      auto res = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(150)); 
      String mid = "{\"vendorId\":\"EVSE-iPAC-New\",\"messageId\":\"InitValue\",\"data\": \"[" + String(buffer[5]) + "," + String(buf_InitSoC, 1) + "," + String(buf_SoH, 1) + "," + String(buf_SoC, 1) + "," + String(buf_InitU, 1) + "," + String(buf_InitI, 1) + "," + String(buf_SoKM, 1) + "]\"}";
      deserializeJson(*res, mid);
      serializeJson(*res, Serial);
      Serial.println();

      return res; }, [](JsonObject response) -> void
                  {
        if (!strcmp(response["status"], "Accepted")) {
          Serial.println(F("[main] Server get init value"));
        } });
      auto ret = beginTransaction(idTag, buffer[5]);

      if (ret)
      {
        Serial.println(F("[main] Transaction begin accepted"));
        data[1] = 0x01;
        sendData(data); // ack
      }
      else
      {
        Serial.println(F("[main] No transaction initiated"));
        data[2] = 0xFF;
        sendData(data); // ack
      }
    }
  }
}

// handle connector status change
void connectorStHandle(uint8_t buffer[8])
{
  if (buffer[6])
  {
    setConnectorPluggedInput([]()
                             { return true; }, buffer[5]);

    if (strcmp(idTag, userDB[0].idTag) == 0)
    {
      Serial.println(F("Connector 1 plugged in"));
      userDB[0].connectorID = buffer[5];
    }
    else if (strcmp(idTag, userDB[1].idTag) == 0)
    {
      Serial.println(F("Connector 2 plugged in"));
      userDB[1].connectorID = buffer[5];
    }
  }
  else
  {
    setConnectorPluggedInput([]()
                             { return false; }, buffer[5]);
  }
}

void process(uint8_t buffer[8])
{
  int data;
  char measurand[10];
  char unit[4];

  switch (buffer[2])
  {
  case CIMS_CHARGE_STATUS:
    break;
  case HMI_STATUS:
    break;
  case PLC_STATUS:
    break;
  case SLAVE_STATUS:
    break;
  case CONNECTOR_STATUS:
    connectorStHandle(buffer);
    break;
  case HMI_CONTROL_TRANSACTION:
    // code relate OCPP library
    hmiTranControl(buffer);
    break;
  case CURRENT_VALUE:
    data = 0;
    data = buffer[3] | (buffer[4] << 8) | (buffer[5] << 16);
    buf_I = float(data);
    Serial.printf("I: %f\n", buf_I);
    break;
  case VOLT_VALUE:
    data = 0;
    data = buffer[3] | (buffer[4] << 8) | (buffer[5] << 16);
    buf_U = float(data);
    Serial.printf("U: %f\n", buf_U);
    break;
  case ID_TAG:
    isAuth = buffer[6];
    if (!isAuth)
    {
      idTag[0] = '\0'; // reset current ID tag in station
      isAuth = 0;      // set state no authentication
      if (/*check if have in a transaction*/ !getTransaction(buffer[5]))
      { // clear user db if not in a transaction and have logout signal
        if (userDB[0].connectorID == buffer[5])
        {
          userDB[0].idTag[0] = '\0';
          userDB[0].connectorID = 255;
        }
        else if (userDB[1].connectorID == buffer[5])
        {
          userDB[1].idTag[0] = '\0';
          userDB[1].connectorID = 255;
        }
        else
        {
          Serial.println(F("Logout error..."));
        }
      }
      Serial.println(F("Logout..."));
    }
    break;
  case SOT_SOC_VALUE:
    data = 0;
    data = buffer[4] | (buffer[5] << 8);
    buf_SoC = float(data);
    Serial.printf("SoC: %f\n", buf_SoC);
    data = 0;
    data = buffer[3];
    buf_SoT = float(data);
    Serial.printf("SoT: %f\n", buf_SoT);

    sendRequest("DataTransfer", [buffer]() -> std::unique_ptr<DynamicJsonDocument>
                {
      //will be called to create the request once this operation is being sent out
      auto res = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(130)); 
      String mid = "{\"vendorId\":\"EVSE-iPAC-New\",\"messageId\":\"RealValue\",\"data\": \"[" + String(buffer[6]) + "," + String(buf_SoC,1) + "," + String(buf_SoT,1) + "," + String(buf_U,1) + "," + String(buf_I,1) + "," + String(buf_SoKM,1) + "]\"}";
      deserializeJson(*res, mid);
      serializeJson(*res, Serial);
      Serial.println();

      return res; }, [](JsonObject response) -> void
                {
        if (!strcmp(response["status"], "Accepted")) {
          Serial.println(F("[main] Server get real value"));
        } });
    break;
  case CURRENT_INIT:
    data = 0;
    data = buffer[3] | (buffer[4] << 8) | (buffer[5] << 16);
    buf_InitI = float(data);
    Serial.printf("II: %f\n", buf_InitI);
    // addMeterValueInput([]()
    //                    { return buf_InitI; }, "Current.Offered", "A", nullptr, nullptr, buffer[6]);

    break;
  case VOLT_INIT:
    data = 0;
    data = buffer[3] | (buffer[4] << 8) | (buffer[5] << 16);
    buf_InitU = float(data);
    Serial.printf("IU: %f\n", buf_InitU);
    // addMeterValueInput([]()
    //                    { return buf_InitU; }, "Voltage", "V", "Cable", nullptr, buffer[6]);
    break;
  case SOC_NEW_INIT:
    data = 0;
    data = buffer[4] | (buffer[5] << 8);
    buf_InitSoC = float(data);
    Serial.printf("ISoC: %f\n", buf_InitSoC);
    // addMeterValueInput([]()
    //                    { return buf_InitSoC; }, "Energy.Active.Import.Register", "kWh", "Cable", nullptr, buffer[6]);
    break;
  case SOC_INIT:
    data = 0;
    data = buffer[4] | (buffer[5] << 8);
    buf_SoC = float(data);
    Serial.printf("SoC: %f\n", buf_SoC);
    // addMeterValueInput([]()
    //                    { return buf_SoC; }, "SoC", "Percent", nullptr, nullptr, buffer[6]);
    break;
  case SOH_INIT:
    data = 0;
    data = buffer[4] | (buffer[5] << 8);
    buf_SoH = float(data);
    Serial.printf("SoH: %f\n", buf_SoH);
    // addMeterValueInput([]()
    //                    { return buf_SoH; }, "Energy.Active.Import.Register", "kWh", "EV", nullptr, buffer[6]);
    break;
  case SoKM:
    data = 0;
    data = buffer[4] | (buffer[5] << 8);
    buf_SoKM = float(data);
    Serial.printf("SoKM: %f\n", buf_SoKM);
    // addMeterValueInput([]()
    //                    { return buf_SoKM; }, "RPM", nullptr, nullptr, nullptr, buffer[6]);
    break;
  default:
    break;
  }
}

// package data and send to CIMS
void sendData(uint8_t data[5])
{
  uint8_t buffer[8] = {};
  buffer[0] = HEADER_HIGH;
  buffer[1] = HEADER_LOW;

  for (uint8_t i = 0; i < 5; i++)
  {
    buffer[i + 2] = data[i];
  }
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < 7; i++)
  {
    checksum += buffer[i];
  }
  buffer[7] = checksum;
  Serial2.write(buffer, 8);
}

void OCPP_Server_handle(void *pvParameters)
{
  Serial.begin(115200);
  // init RFID reader
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  SPI.begin();                       // Init SPI bus
  mfrc522.PCD_Init();                // Init MFRC522 (PCD is terminology)
  mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
                                     // wait for WiFi connection
  Serial.print(F("[main] Wait for WiFi: "));

  WiFi.begin(STASSID, STAPSK);

  while (!WiFi.isConnected())
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(F("Connected!"));
  // init and config OCPP
  mocpp_initialize(OCPP_BACKEND_URL, OCPP_CHARGE_BOX_ID, "Wallnut Charging Station New", "EVSE-iPAC-New");

  // handle json object confirm from EVSE to server
  setOnSendConf("RemoteStopTransaction", [](JsonObject payload) -> void
                {
    int connectorID = payload["connectorId"];
    if (!strcmp(payload["status"], "Accepted")) { //send to CIMS
      uint8_t data[] = {END_TRANSACTION, 0x00, 0x00, 0x00, (uint8_t) connectorID};
      sendData(data);
    } });

  setOnSendConf("RemoteStartTransaction", [](JsonObject payload) -> void
                {
    int connectorID = payload["connectorId"];
    if (!strcmp(payload["status"], "Accepted")) { //send to CIMS
      uint8_t data[] = {BEGIN_TRANSACTION, 0x00, 0x00, 0x00, (uint8_t) connectorID};
      sendData(data);
    } });
  for (;;)
  {
    readPICC();
    mocpp_loop();
    vTaskDelay(1);
  }
}

void CIMS_handle(void *pvParameters)
{
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  uint8_t uartData = 0;
  uint8_t buffer[8] = {};
  uint8_t count = 0;
  for (;;)
  {
    while (Serial2.available())
    {
      uartData = Serial2.read();
      // check header
      if (count == 0)
      {
        if (uartData == HEADER_HIGH)
        {
          buffer[count] = uartData;
          count++;
          continue;
        }
        else
        {
          count = 0;
        }
      }
      // check header
      if (count == 1)
      {
        if (uartData == HEADER_LOW)
        {
          buffer[count] = uartData;
          count++;
          continue;
        }
        else
        {
          count = 0;
        }
      }
      // save data
      if (count > 1 && count < 8)
      {
        buffer[count] = uartData;
        count++;
      }
      // check checksum & process
      if (count == 8)
      {
        count = 0;
        uint8_t checksum = 0;
        for (uint8_t i = 0; i < 7; i++)
        {
          checksum += buffer[i];
        }
        if (buffer[7] != checksum)
        {
          Serial2.println("error data"); // DEBUG
          continue;
        }
        process(buffer);
      }
    }
    vTaskDelay(1);
  }
}

void loop()
{
}