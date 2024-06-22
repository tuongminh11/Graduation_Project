/*
 * ToESP.c
 *
 *  Created on: Jan 18, 2024
 *      Author: Admin
 */

#include "ToESP.h"


DATA_TO_ESP ESP_Data;

uint8_t ESP_Payload[8];

void Packet_to_ESP(uint8_t type, uint8_t state)
{
	ESP_Payload[0]=0xAB;
	ESP_Payload[1]=0xCD;
	ESP_Payload[2]=type;
	ESP_Data.type_ESP_Data=type;
	if((ESP_Data.type_ESP_Data == TYPE_INTERNET_STATUS) ||
	   (ESP_Data.type_ESP_Data == TYPE_CIMS_CHARGE_STATUS) ||
	   (ESP_Data.type_ESP_Data == TYPE_HMI_STATUS) ||
	   (ESP_Data.type_ESP_Data == TYPE_PLC_STATUS) ||
	   (ESP_Data.type_ESP_Data == TYPE_ID_TAG))
	{
		ESP_Payload[3]=0x00;
		ESP_Payload[4]=0x00;
		ESP_Payload[5]=0x00;
		ESP_Payload[6]=state;
	}
	else if(ESP_Data.type_ESP_Data==TYPE_HMI_CONTROL_TRANSACTION)
	{
		ESP_Payload[3]=0x00;
		ESP_Payload[4]=0x00;
		ESP_Payload[5]=0x01;
		ESP_Payload[6]=state;

	}
	else if(ESP_Data.type_ESP_Data==TYPE_VOLTAGE_VALUE)
	{
		ESP_Payload[5]=0;
		ESP_Payload[3]=(uint8_t)(ESP_Data.ESP_Data_voltage & 0xFF);
		ESP_Payload[4]=(uint8_t)((ESP_Data.ESP_Data_voltage>>8) & 0xFF);
		ESP_Payload[6]=1;
	}
	else if(ESP_Data.type_ESP_Data==TYPE_CURRENT_VALUE)
	{
		ESP_Payload[5]=0;
		ESP_Payload[3]=(uint8_t)(ESP_Data.ESP_Data_current & 0xFF);
		ESP_Payload[4]=(uint8_t)((ESP_Data.ESP_Data_current>>8) & 0xFF);
		ESP_Payload[6]=1;
	}
	ESP_Payload[7]=ESP_Payload[0]+ESP_Payload[1]+ESP_Payload[2]+ESP_Payload[3]+ESP_Payload[4]+ESP_Payload[5]+ESP_Payload[6];
	ESP_Send();
}
