/*
 * hmi.c
 *
 *  Created on: Jan 3, 2024
 *      Author: PC
 */

#include "cims.h"

char hmi_buffer[HMI_TX_BUFFER_SIZE];

extern void HMI_Print(void);

//void HMI_Print(void) {
//  HAL_UART_Transmit(&huart4, (uint8_t*)hmi_buffer, strlen(hmi_buffer), TIMEOUT_100_MS);
//  uint8_t END_BYTE = 0xFF;
//  HAL_UART_Transmit(&huart2, &END_BYTE, 1, 10);
//  HAL_UART_Transmit(&huart2, &END_BYTE, 1, 10);
//  HAL_UART_Transmit(&huart2, &END_BYTE, 1, 10);
//}

hmi_data myHMI;

void HMI_Compose_Pre_Charge_Parm(uint8_t type, uint16_t value){
	switch(type){
	case HMI_COMPATIBLE:
		if(value == 0x00AA) sprintf(hmi_buffer,"com.txt=\"Compatible\"");
		else sprintf(hmi_buffer,"com.txt=\"Incompatible\"");
		break;
	case HMI_BATTERY_CAPACITY:
		sprintf(hmi_buffer,"bat.val=%u", value);
		break;
	case HMI_CURRENT_BATTERY:
		sprintf(hmi_buffer,"Interface.bat.val=%u", value);
		break;
	case HMI_BATTERY_VOLTAGE:
		sprintf(hmi_buffer,"vbatt.txt=\"%u V\"", value);
		break;
	case HMI_CHARGING_TIME:
		sprintf(hmi_buffer,"time.txt=\"%u mins\"", value);
		break;
	case HMI_CAR_MODEL:
		sprintf(hmi_buffer,"model.txt=\"Ioniq 5\"");
		break;
	}
}

void HMI_Compose_Realtime_Data(uint8_t type, uint16_t value){
	value = value*10;
	switch(type){
	case HMI_VOLTAGE:
		sprintf(hmi_buffer,"v.val=%u", value);
		break;
	case HMI_CURRENT:
		sprintf(hmi_buffer,"i.val=%u", value);
		break;
	case HMI_TEMP:
		sprintf(hmi_buffer,"t.val=%u", value);
		break;
	case HMI_IREF:
		sprintf(hmi_buffer,"iref.val=%u", value);
		break;
	}
}

void HMI_Compose_Status(uint8_t status){
	switch(status){
	case HMI_PAGE1:
		sprintf(hmi_buffer,"page 1");
		break;
	case HMI_STOP:
		sprintf(hmi_buffer,"bug.val=0");
		break;
	case HMI_CONNECT:
		sprintf(hmi_buffer,"state.txt=\"Connected\"");
		break;
	case HMI_READY:
		sprintf(hmi_buffer,"state.txt=\"Ready\"");
		break;
	case HMI_WARNING_1:
		sprintf(hmi_buffer,"bug.val=1");
		break;
	case HMI_ERROR_1:
		sprintf(hmi_buffer,"bug.val=2");
		break;
	}
}

void HMI_Evaluate_Setting_Data(uint8_t str[8])
{
	myHMI.cable = str[0];
	myHMI.mode = str[1];
	switch(myHMI.mode)
	{
	case '0':
		myHMI.voltage = 200;
		myHMI.current = 15;
	break;
	case '1':
		myHMI.voltage = 200;
		myHMI.current = 15;
	break;
	case '2':
		myHMI.time = (str[2]-48)*100 + (str[3]-48)*10 + (str[4]-48);
	break;
	case '3':
		myHMI.voltage = (str[2]-48)*100 + (str[3]-48)*10 + (str[4]-48);
		myHMI.current = (str[5]-48)*100 + (str[6]-48)*10 + (str[7]-48);
	break;
	}
}

