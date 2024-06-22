#define TRIGGER_PIN 23
// #define STASSID "IPAC_LAB_Member"
// #define STAPSK "12345678"
#define STASSID "W403"
#define STAPSK "kamekamehahaha"
//-----------------------------------------------------------
#define RST 22    // RST pin in ESP32 module 30 pin
#define SS_PIN 21 // SS  pin
//-----------------------------------------------------------
#define OCPP_CHARGE_BOX_ID "esp32-charger-new" // device ID in server, need declare in server before, unique
// String OCPP_BACKEND_URL;
#define OCPP_BACKEND_URL "ws://58.186.92.182:34589/steve/websocket/CentralSystemService/" // URL
//-----------------------------------------------------------
/**
 * Frame structure
 * byte |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7     |
 * ------------------------------------------------------------------
 * mean |  HH   |   HL  |  ID   |  D0   |   D1  |   D2  |   D3  | checksum|
 *
 * HH: HEADER_HIGH
 * HL: HEADER_LOW
 * ID: ID of frame (meaning of frame)
 *
 */
#define HEADER_HIGH 0xAB // header byte of communication frame
#define HEADER_LOW 0xCD
//-----------------------------------------------------------
// Frame ID
#define INTERNET_STATUS 0x11
#define CIMS_CHARGE_STATUS 0x12
#define HMI_STATUS 0x13
#define PLC_STATUS 0x14
#define ID_TAG 0x15
#define SLAVE_STATUS 0x16
#define CONNECTOR_STATUS 0x17
#define HMI_CONTROL_TRANSACTION 0x18
#define BEGIN_TRANSACTION 0x19
#define END_TRANSACTION 0x1A
#define TRANSACTION_CONFIRMATION 0x1B
#define TRIGGER_MESSAGE 0x1C
#define SYNC_CLOCK_TIME 0x1D
#define SYNC_CLOCK_DATE 0x1E
#define CURRENT_VALUE 0x20
#define VOLT_VALUE 0x21
#define SOT_SOC_VALUE 0x22
#define CURRENT_INIT 0x23
#define VOLT_INIT 0x24
#define SOC_NEW_INIT 0x25
#define SOC_INIT 0x26
#define SOH_INIT 0x27
#define SoKM 0x28
//-----------------------------------------------------------
// UART 2 comunicatino to CIMS
#define RXD2 16
#define TXD2 17
