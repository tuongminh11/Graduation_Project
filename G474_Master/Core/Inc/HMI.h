/*
 * HMI_V3.h
 *
 *  Created on: Jun 11, 2023
 *      Author: PC
 */

#ifndef INC_HMI_H_
#define INC_HMI_H_

#include <stdio.h>
#include "mycanopen.h"
#include "main.h" //Change chip's library

//HMI Variable
//#define HMI_VOLTAGE 0x00
//#define HMI_CURRENT 0X01
//#define HMI_TEMP 0x02

////HMI Status
//#define HMI_CONNECT 0x10
//#define HMI_WARNING 0x11
//#define HMI_ERROR 0x12
//#define HMI_STOP 0x13
//#define HMI_READY 0X14
//
//#define HMI_CHARGING_ON 0x01
//#define HMI_CHARGING_OFF 0x00
////Struct
//typedef struct
//{
//	uint8_t plug;
//	uint8_t mode;
//	uint8_t vout[3];
//	uint8_t iout[3];
//	uint8_t time[3];
//}HMI_Data;
//
////HMI Function
//void HMI_Transmit(UART_HandleTypeDef* huart, uint8_t var, float data);
//void HMI_Status(UART_HandleTypeDef* huart, uint8_t status);
//void HMI_Fillter(HMI_Data* data, uint8_t str[8]);
//void HMI_Timer_Transmit_data(UART_HandleTypeDef* huart,uint32_t event_time,float current,float voltage, float temperature, uint32_t *previoustick);
//void HMI_Timer_Transmit_status(UART_HandleTypeDef* huart,uint32_t event_time,uint8_t status,uint32_t *previoustick);
//void HMI_EV_Data_Transmit(UART_HandleTypeDef* huart,uint16_t batt, uint16_t vbat,uint16_t time);
//void HMI_IREF_Transmit(UART_HandleTypeDef* huart,uint16_t iref);
////Convert Function
//float uint8_to_float(uint8_t* data, float x);
//float uint16_to_float(uint16_t data, float x);
//uint16_t int_to_uint16(float x);
//uint16_t HMI_to_uint16(uint8_t* str);

#endif /* INC_HMI_H_ */
