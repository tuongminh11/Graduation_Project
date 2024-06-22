/*
 * mycanopen.h
 *
 *  Created on: May 28, 2023
 *      Author: Admin
 */

#ifndef INC_MYCANOPEN_H_
#define INC_MYCANOPEN_H_

#include "main.h"



// my define

#define CHARGING_OFF 0x00
#define CHARGING_ON 0x01
#define CHARGING_ERROR 0x02
#define CHARGING_ARLAM 0x03
#define CHARGING_STOP 0x04
#define CHARGING_INIT 0x05

#define CHARGING_REQUEST 0x01
#define CHARGING_RESPOND 0x02
#define CHARGING_SETTING 0x03
#define CHARGING_CONFIRM 0x04

#define CHARGING_PLUG_1 0x10
#define CHARGING_PLUG_2 0x20

#define CHARGING_MODE_1 0x01
#define CHARGING_MODE_2 0x02
#define CHARGING_MODE_3 0x03
#define CHARGING_MODE_4 0x04


#define CHARGING_DATA_CURRENT 0x01
#define CHARGING_DATA_VOLTAGE 0x02
#define CHARGING_DATA_TEMPERATURE 0x03
#define CHARGING_DATA_TIME 0x04

#define CHARGING_MASTER_ID 1
#define CHARGING_SLAVE_ID 0


#define ID_IN_USED 0xAA
#define ID_NOT_USED 0
#define ID_DISCONNECTED 0xBB
// end of my define

//my struct
typedef struct
{
	uint8_t state;
	uint8_t function;
	uint8_t plug;
	uint8_t mode;
	uint8_t slaveID;
	uint8_t type_of_value;
	uint16_t voltage_value;
	uint16_t current_value;
	uint16_t temperature_value;
	uint16_t time_value;
}SPDO_Data;


//end of my struct



//void sendPDO(FDCAN_HandleTypeDef *hfdcan, FDCAN_TxHeaderTypeDef *pHeader,uint8_t aData[], uint32_t event_time, uint32_t *previoustick);
void PDO_set_data_frame(uint8_t* aData,SPDO_Data My_PDO_Data);
void deframe_PDO(uint8_t aData[], SPDO_Data* My_Receive_SPDO_Data);

void changeTypeOfValue(uint8_t* type_of_value);
void UpdateCANMasterTxDesID(uint8_t firstID, uint8_t lastID, SPDO_Data *Tx_SPDO_Data,uint8_t aData[]);
void Reset_Data(SPDO_Data *Tx_SPDO_Data);
#endif /* INC_MYCANOPEN_H_ */
