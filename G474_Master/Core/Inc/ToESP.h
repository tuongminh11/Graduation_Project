/*
 * ToESP.h
 *
 *  Created on: Jan 18, 2024
 *      Author: Admin
 */

#ifndef INC_TOESP_H_
#define INC_TOESP_H_



#endif /* INC_TOESP_H_ */

#include "main.h"


typedef struct
{
	uint8_t type_ESP_Data;
	uint16_t ESP_Data_voltage;
	uint16_t ESP_Data_current;
	uint8_t check_sum;
} DATA_TO_ESP;


extern DATA_TO_ESP ESP_Data;
extern uint8_t ESP_Payload[8];
extern void ESP_Send(void);

#define TYPE_INTERNET_STATUS 0X11
#define TYPE_CIMS_CHARGE_STATUS 0X12
#define TYPE_HMI_STATUS 0X13
#define TYPE_PLC_STATUS 0X14
#define TYPE_ID_TAG 0X15
#define TYPE_SLAVE_STATUS 0X16
#define TYPE_CONNECTER_STATUS 0X17
#define TYPE_HMI_CONTROL_TRANSACTION 0X18
#define TYPE_BEGIN_TRANSACTION 0X10
#define TYPE_END_TRANSACTION 0X1A
#define TYPE_TRANSACTION_CONFIRM 0X1A
#define TYPE_CURRENT_VALUE 0X20
#define TYPE_VOLTAGE_VALUE 0X21

#define TIMEOUT_100 100

#define STATE_ON 0X01
#define STATE_OFF 0X00


extern void Packet_to_ESP(uint8_t type, uint8_t state);
