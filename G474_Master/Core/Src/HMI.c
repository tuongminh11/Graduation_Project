/*
 * HMI_V3.c
 *
 *  Created on: Jun 11, 2023
 *      Author: PC
 */

#include "HMI.h"
#include "string.h"

/* @brift This function use to transmit current data to HMI
 * @param huart pointer to a UART_HandleTypeDef structure that contains the configuration information for UART module
 * @param HMI variable
 * @param current data in float
 */
//void HMI_Transmit(UART_HandleTypeDef* huart, uint8_t var, float data)
//{
//	uint8_t END_BYTE = 0xFF;
//	int x = data*10;
//	uint8_t str[]="x.val=0000";
//	switch(var)
//	{
//	case 0x00:
//		str[0]='v';
//	break;
//	case 0x01:
//		str[0]='i';
//	break;
//	case 0x02:
//		str[0]='t';
//	break;
//	}
//	for(int i=9; i>5; i--)
//	{
//		str[i] = x%10 + 48;
//		x = x/10;
//	}
//	HAL_UART_Transmit(huart, str, 10, 10);
//	HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//	HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//	HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//}
//
///* @brift This function use to transmit current data to HMI
// * @param huart pointer to a UART_HandleTypeDef structure that contains the configuration information for UART module
// * @param HMI status
// */
//void HMI_Status(UART_HandleTypeDef* huart, uint8_t status)
//{
//	uint8_t END_BYTE = 0xFF;
//	//uint8_t str1[]="state.txt=\"Connected\"";
//	uint8_t str1[]={0x73, 0x74, 0x61, 0x74, 0x65, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x43, 0x6F, 0x6E, 0x6E, 0x65, 0x63, 0x74, 0x65, 0x64, 0x22};
//	//uint8_t str2[]="bug.val=1";
//	uint8_t str2[]={0x62, 0x75, 0x67, 0x2E, 0x76, 0x61, 0x6C, 0x3D, 0x31};
//	//uint8_t str3[]="bug.val=2";
//	uint8_t str3[]={0x62, 0x75, 0x67, 0x2E, 0x76, 0x61, 0x6C, 0x3D ,0x32};
//	//uint8_t str3[]="bug.val=0";
//	uint8_t str4[]={0x62, 0x75, 0x67, 0x2E, 0x76, 0x61, 0x6C, 0x3D ,0x30};
//
//	switch(status)
//	{
//		case HMI_CONNECT:
//		//	uint8_t str[]={0x73, 0x74, 0x61, 0x74, 0x65, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x43, 0x6F, 0x6E, 0x6E, 0x65, 0x63, 0x74, 0x65, 0x64, 0x22};
//			HAL_UART_Transmit(huart, str1, sizeof(str1), 10);
//			HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//			HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//			HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//			break;
//		case HMI_WARNING:
//			HAL_UART_Transmit(huart, str2, sizeof(str2), 10);
//			HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//			HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//			HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//			break;
//		case HMI_ERROR:
//			HAL_UART_Transmit(huart, str3, sizeof(str3), 10);
//			HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//			HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//			HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//			break;
//		case HMI_STOP:
//			HAL_UART_Transmit(huart, str4, sizeof(str4), 10);
//			HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//			HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//			HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//			break;
//		case HMI_READY:
//			char str[2100];
//					sprintf(str,"state.txt=\"Ready\"");
//					HAL_UART_Transmit(huart, (uint8_t*)str, strlen(str), 100);
//					HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//					HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//					HAL_UART_Transmit(huart, &END_BYTE, 1, 1);
//			break;
//	}
//}
//
///* @brift This function use to filter data from hmi
// * @param HMI_data is struct that include all data
// * @param data from HMI
// */
//void HMI_Fillter(HMI_Data* data, uint8_t str[8])
//{
//
//	data->plug=str[0];
//	data->mode=str[1];
//	switch(str[1])
//	{
//	case '0':			//300V, 20A
//		data->vout[0]=0x33;
//		data->vout[1]=0x30;
//		data->vout[2]=0x30;
//
//		data->iout[0]=0x30;
//		data->iout[1]=0x32;
//		data->iout[2]=0x30;
//	break;
//	case '1':			//200V,10A
//		data->vout[0]=0x32;
//		data->vout[1]=0x30;
//		data->vout[2]=0x30;
//
//		data->iout[0]=0x30;
//		data->iout[1]=0x31;
//		data->iout[2]=0x30;
//
//	break;
//	case '2':			//TIMER set by HMI -100v 20a
//		data->vout[0]=0x31;
//		data->vout[1]=0x30;
//		data->vout[2]=0x30;
//
//		data->iout[0]=0x30;
//		data->iout[1]=0x32;
//		data->iout[2]=0x30;
//
//		data->time[0]=str[2];
//		data->time[1]=str[3];
//		data->time[2]=str[4];
//
//	break;
//	case '3':			//set by HMI
//		data->vout[0]=str[2];
//		data->vout[1]=str[3];
//		data->vout[2]=str[4];
//
//		data->iout[0]=str[5];
//		data->iout[1]=str[6];
//		data->iout[2]=str[7];
//
//	break;
//	}
//}
//
///* @brift This function use to convert uint8_t to float
// * @param data in uint8_t*
// * @param x is the multiplier
// */
//float uint8_to_float(uint8_t* data, float x)
//{
//	uint16_t a;
//	a = (data[0]&0xFF)|((data[1]&0xFF)<<8);
//	return a*x;
//}
//
//
///* @brift This function use to convert uint16_t to float
// * @param data in uint16_t
// * @param x is the multiplier
// */
//float uint16_to_float(uint16_t data, float x)
//{
//	return data*x;
//}
//
///* @brift This function use to convert data to uint_16
// * @param data in uint8_t
// */
//uint16_t HMI_to_uint16(uint8_t* str) {
//    int result = 0;
//    for (int i=0; i<3; i++)
//    {
//        if (str[i] >= '0' && str[i] <= '9')
//        {
//            result = result * 10 + (str[i] - '0');
//        }
//        else
//        {
//            return 0;
//        }
//    }
//    return (uint16_t)result;
//}
//
//void HMI_Timer_Transmit_data(UART_HandleTypeDef* huart,uint32_t event_time,float current,float voltage, float temperature, uint32_t *previoustick)
//{
//	uint32_t currenttick = HAL_GetTick(); // Lưu thời điểm bắt đầu
//  	if ((currenttick - *previoustick) >event_time)
//  	{
//  		*previoustick=HAL_GetTick();
//  		//
//  		uint8_t str[39]="i.val=1000   v.val=2000   t.val=3000";
//  		str[10]=0xFF;str[11]=0xFF;str[12]=0xFF;str[23]=0xFF;str[24]=0xFF;str[25]=0xFF;str[36]=0xFF;str[37]=0xFF;str[38]=0xFF;
//  			int x1 = current*10;
//  			int x2 = voltage*10;
//  			int x3 = temperature*10;
//  			for(int i=9; i>5; i--)
//  			{
//  				str[i] = x1%10 + 48;
//  				x1 = x1/10;
//  			}
//
//  			for(int i=22; i>18; i--)
//  			{
//  				str[i] = x2%10 + 48;
//  				x2 = x2/10;
//  			}
//
//  			for(int i=35; i>31; i--)
//  			{
//  				str[i] = x3%10 + 48;
//  				x3 = x3/10;
//  			}
//  		//
//  			HAL_UART_Transmit(huart, str, sizeof(str), 10);
//  	}
//}
//
//void HMI_Timer_Transmit_status(UART_HandleTypeDef* huart,uint32_t event_time,uint8_t status,uint32_t *previoustick)
//{
//	uint32_t currenttick = HAL_GetTick(); // Lưu thời điểm bắt đầu
//  	if ((currenttick - *previoustick) >event_time)
//  	{
//  		*previoustick=HAL_GetTick();
//  		if(status==CHARGING_ERROR) HMI_Status(huart, HMI_ERROR);
//  		if(status==CHARGING_ARLAM) HMI_Status(huart, HMI_WARNING);
//  		if(status==CHARGING_STOP)  HMI_Status(huart, HMI_STOP);
//  	}
//}
//
//void HMI_IREF_Transmit(UART_HandleTypeDef* huart,uint16_t iref)
//{
//	uint8_t str[16]="iref.val=200   ";
//	str[11]=0xFF;str[12]=0xFF;str[13]=0xFF;
//	int x1=iref*10;
//	for(int i=11; i>8; i--)
//	{
//		str[i] = x1%10 + 48;
//		x1 = x1/10;
//	}
//	HAL_UART_Transmit(huart, str, sizeof(str), 10);
//}
//void HMI_EV_Data_Transmit(UART_HandleTypeDef* huart,uint16_t batt, uint16_t vbat,uint16_t time)
//{
//	//com.txt="Compatible"
//	uint8_t c[23] = {0x63, 0x6F, 0x6D, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x43, 0x6F, 0x6D, 0x70, 0x61, 0x74, 0x69, 0x62, 0x6C, 0x65, 0x22, 0xFF, 0xFF, 0xFF};
//	//batt.txt="100%"
//	uint8_t b[18] = {0x62, 0x61, 0x74, 0x74, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x31, 0x30, 0x30, 0x25, 0x22, 0xFF, 0xFF, 0xFF};
//    b[12]='0'+(batt%10);
//    b[11]='0'+((batt/10)%10);
//    b[10]='0'+((batt/100)%10);
//    //vbat.txt="456V"
//    uint8_t v[18] = {0x76, 0x62, 0x61, 0x74, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x34, 0x35, 0x36, 0x56, 0x22, 0xFF, 0xFF, 0xFF};
//    v[12]='0'+(vbat%10);
//    v[11]='0'+((vbat/10)%10);
//    v[10]='0'+((vbat/100)%10);
//    //time.txt="123p"
//    uint8_t t[18] = {0x74, 0x69, 0x6D, 0x65, 0x2E, 0x74, 0x78, 0x74, 0x3D, 0x22, 0x31, 0x32, 0x33, 0x70, 0x22, 0xFF, 0xFF, 0xFF};
//    t[12]='0'+(time%10);
//    t[11]='0'+((time/10)%10);
//    t[10]='0'+((time/100)%10);
//
//    HAL_UART_Transmit(huart, c, sizeof(b), 100);
//	HAL_UART_Transmit(huart, b, sizeof(b), 100);
//	HAL_UART_Transmit(huart, v, sizeof(b), 100);
//	HAL_UART_Transmit(huart, t, sizeof(b), 100);
//}
