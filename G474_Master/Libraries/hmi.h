/*
 * hmi.h
 *
 *  Created on: Jan 3, 2024
 *      Author: PC
 */
#define HMI_TX_BUFFER_SIZE 50

//HMI Realtime Data
#define HMI_VOLTAGE 0x00
#define HMI_CURRENT 0X01
#define HMI_TEMP 0x02
#define HMI_IREF 0x03
#define HMI_VOLTAGE2 0x04
#define HMI_CURRENT2 0X05
#define HMI_TEMP2 0x06


//HMI Status
#define HMI_STOP 0x10
#define HMI_CONNECT 0x11
#define HMI_READY 0x12
#define HMI_PAGE1 0x13
#define HMI_CONNECT2 0x14

#define HMI_WARNING_1 0x20
#define HMI_WARNING_2 0x21
#define HMI_WARNING_3 0x22
#define HMI_WARNING_4 0x23
#define HMI_WARNING_5 0x24

#define HMI_ERROR_1 0x25
#define HMI_ERROR_2 0x26
#define HMI_ERROR_3 0x27
#define HMI_ERROR_4 0x28
#define HMI_ERROR_5 0x2A

//#define HMI_CONNECT 0x10
//#define HMI_WARNING 0x11
//#define HMI_ERROR 0x12
//#define HMI_STOP 0x13

//HMI Pre charge parm
#define HMI_COMPATIBLE 0x30
#define HMI_BATTERY_CAPACITY 0X31
#define HMI_InitSoC 0x32
#define HMI_SoH 0X33
#define HMI_CHARGING_TIME_HOUR 0x34
#define HMI_CHARGING_TIME_MIN 0x36
#define HMI_CAR_MODEL 0x35
#define HMI_SoC 0x37
#define HMI_InitSoC2 0x38
#define HMI_SoH2 0X39
#define HMI_CHARGING_TIME_HOUR2 0x40
#define HMI_CHARGING_TIME_MIN2 0x41
#define HMI_SoC2 0x42
#define HMI_TRAN_STATUS1 0x43
#define HMI_TRAN_STATUS2 0x44
typedef struct hmi_setting_parameter{
	uint8_t cable;
	uint8_t mode;
	uint16_t voltage;
	uint16_t current;
	uint16_t time;
}hmi_data;

extern hmi_data myHMI;
extern char hmi_buffer[HMI_TX_BUFFER_SIZE];

extern void HMI_Compose_Pre_Charge_Parm(uint8_t type, uint16_t value);
extern void HMI_Compose_Realtime_Data(uint8_t type, uint16_t value);
extern void HMI_Compose_Status(uint8_t status);
extern void HMI_Evaluate_Setting_Data(uint8_t str[8]);
