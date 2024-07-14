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
		if(value == 0x00AA) sprintf(hmi_buffer,"com.val=\"Compatible\"");
		else sprintf(hmi_buffer,"com.val=\"Incompatible\"");
		break;
	case HMI_InitSoC:
		sprintf(hmi_buffer,"InitSoC1.val=%u", value);
		break;
	case HMI_SoC:
		sprintf(hmi_buffer,"SoC1.val=%u", value);
		break;
	case HMI_SoH:
		sprintf(hmi_buffer,"SoH1.val=%u", value);
		break;
	case HMI_CHARGING_TIME_MIN:
		sprintf(hmi_buffer,"min1.val=%u", value);
		break;
	case HMI_CHARGING_TIME_HOUR:
		sprintf(hmi_buffer,"hour1.val=%u", value);
		break;
	case HMI_CAR_MODEL:
		sprintf(hmi_buffer,"model.val=\"Ioniq 5\"");
		break;

	//
	case HMI_InitSoC2:
		sprintf(hmi_buffer,"InitSoC2.val=%u", value);
		break;
	case HMI_SoC2:
		sprintf(hmi_buffer,"SoC2.val=%u", value);
		break;
	case HMI_SoH2:
		sprintf(hmi_buffer,"SoH2.val=%u", value);
		break;
	case HMI_CHARGING_TIME_MIN2:
		sprintf(hmi_buffer,"min2.val=%u", value);
		break;
	case HMI_CHARGING_TIME_HOUR2:
		sprintf(hmi_buffer,"hour2.val=%u", value);
		break;
	}
}

void HMI_Compose_Realtime_Data(uint8_t type, uint16_t value){
	switch(type){
	case HMI_TRAN_STATUS1:
		sprintf(hmi_buffer,"Authorize.plug_s1.val=%u", value);
		break;
	case HMI_TRAN_STATUS2:
		sprintf(hmi_buffer,"Authorize.plug_s2.val=%u", value);
		break;
	case HMI_VOLTAGE2:
		sprintf(hmi_buffer,"v2.val=%u", value);
		break;
	case HMI_CURRENT2:
		sprintf(hmi_buffer,"i2.val=%u", value);
		break;
	case HMI_TEMP2:
		sprintf(hmi_buffer,"t2.val=%u", value);
		break;
	case HMI_VOLTAGE:
		sprintf(hmi_buffer,"v1.val=%u", value);
		break;
	case HMI_CURRENT:
		sprintf(hmi_buffer,"i1.val=%u", value);
		break;
	case HMI_TEMP:
		sprintf(hmi_buffer,"t1.val=%u", value);
		break;
	case HMI_IREF:
		sprintf(hmi_buffer,"iref.val=%u", value);
		break;
	case HMI_SoC:
		sprintf(hmi_buffer,"SoC1.val=%u", value);
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
		sprintf(hmi_buffer,"state1.txt=\"Connected\"");
		break;
	case HMI_CONNECT2:
		sprintf(hmi_buffer,"state2.txt=\"Connected\"");
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

