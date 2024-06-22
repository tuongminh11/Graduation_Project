/*
 * mycanopen.c
 *
 *  Created on: May 28, 2023
 *      Author: Admin
 */
#include "mycanopen.h"


/**
  * @brief  Add a PDO message to the first free Tx mailbox and activate the
  *         corresponding transmission request.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  pHeader pointer to a CAN_TxHeaderTypeDef structure.
  * @param  aData array containing the payload of the Tx frame.
  * @param  event_time represents the time interval between two consecutive PDO transmissions.
  * @param  previoustick is the previous time that transmit PDO, must be a global variable and setting=0 at first
  */
//void sendPDO(FDCAN_HandleTypeDef *hfdcan, FDCAN_TxHeaderTypeDef *pHeader,uint8_t aData[],uint32_t event_time, uint32_t *previoustick)
//{
//	uint32_t currenttick = HAL_GetTick(); // Lưu thời điểm bắt đầu
//  	if ((currenttick - *previoustick) >event_time)
//  	{
//  		*previoustick=HAL_GetTick();
//			 if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, pHeader, aData)!= HAL_OK)
//			 {
//			  Error_Handler();
//			 }
//  	}
//}


/**
  * @brief  Insert all data needed into Transmit 8 byte Frame
  * @param  aData is Transmit_data to transmit
  * @param  My_PDO_Data is struct that include all data
  */
void PDO_set_data_frame(uint8_t* aData,SPDO_Data My_PDO_Data)
{
	aData[1]=My_PDO_Data.state;
	aData[2]=My_PDO_Data.function;
	aData[3]=My_PDO_Data.plug |My_PDO_Data.mode;
	aData[4]=My_PDO_Data.slaveID;
	aData[5]=My_PDO_Data.type_of_value;

	uint16_t value;
	switch (My_PDO_Data.type_of_value)
	{
	case CHARGING_DATA_CURRENT:
		value=My_PDO_Data.current_value;
		break;
	case CHARGING_DATA_VOLTAGE:
		value=My_PDO_Data.voltage_value;
		break;
	case CHARGING_DATA_TEMPERATURE:
		value=My_PDO_Data.temperature_value;
		break;
	case CHARGING_DATA_TIME:
		value=My_PDO_Data.time_value;
		break;
	}
	aData[6]=(value >> 8) & 0xFF;
	aData[7]=value & 0xFF;
}

/**
  * @brief  Insert all data transmited into data Struct
  * @param  aData is Receive_data
  * @param  My_PDO_Data is struct that include all data
  */
void deframe_PDO(uint8_t* aData, SPDO_Data* SPDO_Data)
{
	SPDO_Data->state=aData[1];
	SPDO_Data->function=aData[2];
	SPDO_Data->plug=aData[3]&0xF0;
	SPDO_Data->mode=aData[3]&0x0F;
	SPDO_Data->slaveID=aData[4];
	SPDO_Data->type_of_value=aData[5];

	switch (aData[5])
	{
	case CHARGING_DATA_CURRENT:
		SPDO_Data->current_value=(uint16_t)aData[6]<< 8 | aData[7];
		break;
	case CHARGING_DATA_VOLTAGE:
		SPDO_Data->voltage_value=(uint16_t)aData[6]<< 8 | aData[7];
		break;
	case CHARGING_DATA_TEMPERATURE:
		SPDO_Data->temperature_value=(uint16_t)aData[6]<< 8 | aData[7];
		break;
	case CHARGING_DATA_TIME:
		SPDO_Data->time_value=(uint16_t)aData[6]<< 8 | aData[7];
		break;
	}
}



void changeTypeOfValue(uint8_t* type_of_value)
{
    switch (*type_of_value)
    {
        case CHARGING_DATA_CURRENT:
            *type_of_value = CHARGING_DATA_VOLTAGE;
            break;
        case CHARGING_DATA_VOLTAGE:
            *type_of_value = CHARGING_DATA_TEMPERATURE;
            break;
        case CHARGING_DATA_TEMPERATURE:
            *type_of_value = CHARGING_DATA_TIME;
            break;
        case CHARGING_DATA_TIME:
            *type_of_value = CHARGING_DATA_CURRENT;
            break;
    }
}

void UpdateCANMasterTxDesID(uint8_t firstID, uint8_t lastID, SPDO_Data *Tx_SPDO_Data,uint8_t aData[])
{
	if (Tx_SPDO_Data->type_of_value == CHARGING_DATA_VOLTAGE)
	{
		aData[0]+=1;
		if(aData[0]>lastID) aData[0]=firstID;
	}
}

void Reset_Data(SPDO_Data *Tx_SPDO_Data)
{
	Tx_SPDO_Data->current_value=0;
	Tx_SPDO_Data->temperature_value=0;
	Tx_SPDO_Data->time_value=0;
	Tx_SPDO_Data->voltage_value=0;
}
