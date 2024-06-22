/*
 * QCA.c
 *
 *  Created on: Nov 20, 2023
 *      Author: Admin
 */
#include "QCA.h"
#include "string.h"


void uint64ToUint8Array(uint64_t value, uint8_t *array)
{
    for (int i = 5; i >= 0; --i)
    {
        array[i] = (uint8_t)(value & 0xFF); // Lấy 8 bit LSB
        value >>= 8; // Dịch trái để lấy byte tiếp theo
    }
}


void QCA_Frame_Init(QCA_Frame *myFrame, uint64_t sourceadd,uint64_t destinationadd, uint16_t datalength)
{
    myFrame->SoF[0]=0xAA;
    myFrame->SoF[1]=0xAA;
    myFrame->SoF[2]=0xAA;
    myFrame->SoF[3]=0xAA;

    myFrame->FL[0]=(datalength+20) & 0xFF;
    myFrame->FL[1]=((datalength+20) >> 8) & 0xFF;

    myFrame->RSVD[0]=0x00;
    myFrame->RSVD[1]=0x00;

    uint64ToUint8Array(sourceadd,myFrame->SA);
    uint64ToUint8Array(destinationadd,myFrame->DA);

    myFrame->ET[0]=0x88;
    myFrame->ET[1]=0xE1;

    myFrame->MMEV[0]=0x01;
    myFrame->MMET[0]=0x0B;
    myFrame->MMET[1]=0x00;

    myFrame->OUT[0]=0x00;
    myFrame->OUT[1]=0xB0;
    myFrame->OUT[2]=0x52;

    myFrame->EoF[0]=0x55;
    myFrame->EoF[1]=0x55;
}

void QCA_Frame_Package(QCA_Frame myFrame, uint8_t *data,uint16_t datalength, uint8_t transmitBuffer[])
{
    memcpy(&transmitBuffer[0], myFrame.SoF, 4);
    memcpy(&transmitBuffer[4], myFrame.FL, 2);
    memcpy(&transmitBuffer[6], myFrame.RSVD, 2);
    memcpy(&transmitBuffer[8], myFrame.DA, 6);
    memcpy(&transmitBuffer[14], myFrame.SA, 6);
    memcpy(&transmitBuffer[20], myFrame.ET, 2);
    memcpy(&transmitBuffer[22], myFrame.MMEV, 1);
    memcpy(&transmitBuffer[23], myFrame.MMET, 2);
    memcpy(&transmitBuffer[25], myFrame.OUT, 3);
    memcpy(&transmitBuffer[28], data, datalength);
    memcpy(&transmitBuffer[28 + datalength], myFrame.EoF, 2);
}

void QCA_Transmit(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart, uint16_t cmd, uint16_t loc, uint16_t reg)
{
	uint8_t data[4];
	data[0]=(cmd|loc|reg)>>8;
	data[1]=cmd|loc|reg;
	uint8_t rx_data[4];
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(hspi, data, rx_data, 4, QCA_TIME_OUT);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
	HAL_UART_Transmit(huart,rx_data,4, 10);
}

void QCA_Reset_SPI_Config(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart)
{
	uint8_t tx1[4]={0x44, 0x00, 0x00, 0x40};
	uint8_t rx1[4];

	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(hspi, tx1, rx1, 4, QCA_TIME_OUT);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);

	HAL_UART_Transmit(huart, rx1, 4, 10);
}

void QCA_Check_Signature(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart)
{
	QCA_Transmit(hspi, GPIOx, GPIO_Pin, huart, QCA7K_SPI_READ, QCA7K_SPI_INTERNAL, SPI_REG_SIGNATURE);
}

uint16_t QCA_Get_Signature(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,UART_HandleTypeDef *huart)
{
	uint8_t tx_data[4]={0xDA,0x00,0x00,0x00};
	uint8_t rx_data[4];
		HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
		HAL_SPI_TransmitReceive(hspi, tx_data, rx_data, 4, QCA_TIME_OUT);
		HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);

		HAL_UART_Transmit(huart, rx_data, 4, 10);
	uint16_t signature = ((uint16_t)rx_data[2] << 8) | ((uint16_t)rx_data[3]);
	return signature;
}

uint16_t QCA_Get_Avai_Read_data_length(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	uint8_t tx_data[4]={0xC3,0x00,0x00,0x00};
	uint8_t rx_data[4];
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(hspi, tx_data, rx_data, 4, QCA_TIME_OUT);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);

	return ((rx_data[2]<<8)|rx_data[3]);
}

uint16_t QCA_Get_Avai_Write_data_length(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	uint8_t tx_data[4]={0xC2,0x00,0x00,0x00};
	uint8_t rx_data[4];
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(hspi, tx_data, rx_data, 4, QCA_TIME_OUT);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);

	return ((rx_data[2]<<8)|rx_data[3]);
}

void QCA_Write_Reg_Buffer_Size(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint16_t size)
{
	uint8_t tx_data[4]={0x41,0x00,0x00,0x00};
	uint8_t rx_data[4];
	tx_data[2]=(uint8_t)(size>>8);
	tx_data[3]=(uint8_t)(size);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(hspi, tx_data, rx_data, 4, QCA_TIME_OUT);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);

}

uint16_t QCA_Read_Reg_INT_Cause(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	uint8_t tx1[4]={0xCC, 0x00, 0x00, 0x00};
	uint8_t rx1[4];

	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(hspi, tx1, rx1, 4, QCA_TIME_OUT);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);

	return ((rx1[2]<<8)|rx1[3]);
}

void QCA_Write_Reg_INT_Cause(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint16_t interupt)
{
	uint8_t tx1[4]={0x4C, 0x00, 0x00, 0x00};
	uint8_t rx1[4];

	tx1[2]=(uint8_t)(interupt>>8);
	tx1[3]=(uint8_t)(interupt);

	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(hspi, tx1, rx1, 4, QCA_TIME_OUT);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}

void QCA_Write_Reg_SPI_Config(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint16_t Config)
{
	uint8_t tx1[4]={0x44, 0x00, 0x00, 0x00};
	uint8_t rx1[4];

	tx1[2]=(uint8_t)(Config>>8);
	tx1[3]=(uint8_t)(Config);

	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(hspi, tx1, rx1, 4, QCA_TIME_OUT);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}

void QCA_Enalbe_All_IT(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart)
{
	uint8_t tx1[4]={0x4D, 0x00, 0x00, 0x47};
	uint8_t rx1[4];

	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(hspi, tx1, rx1, 4, QCA_TIME_OUT);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}

void QCA_Disalbe_All_IT(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart)
{
	uint8_t tx1[4]={0x4D, 0x00, 0x00, 0x00};
	uint8_t rx1[4];

	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(hspi, tx1, rx1, 4, QCA_TIME_OUT);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}


void QCA_Init_Setup(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,UART_HandleTypeDef *huart)
{
	QCA_Transmit(hspi, GPIOx, GPIO_Pin, huart, QCA7K_SPI_READ, QCA7K_SPI_INTERNAL, SPI_REG_SIGNATURE);
	uint16_t signature=QCA_Get_Signature(hspi, GPIOx, GPIO_Pin,huart);
	if(signature==QCASPI_GOOD_SIGNATURE)
	{
		QCA_Enalbe_All_IT(hspi, GPIOx, GPIO_Pin,huart);
	}
}

void QCA_Handling_IT(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart)
{
	QCA_Disalbe_All_IT(hspi, GPIOx, GPIO_Pin, huart);

	uint16_t interupt=QCA_Read_Reg_INT_Cause(hspi, GPIOx, GPIO_Pin);


	//handling Interupt
	if(interupt & SPI_INT_WRBUF_BELOW_WM)
		{
			char tx_buffer[60];
			sprintf(tx_buffer, "Hello, This is SPI_INT_WRBUF_BELOW_WM interupt handle \r\n ");
			HAL_UART_Transmit(huart, (uint8_t *)tx_buffer, strlen(tx_buffer), QCA_TIME_OUT);
		}
	if(interupt & SPI_INT_CPU_ON)
		{
			char tx_buffer[60];
			sprintf(tx_buffer, "Hello, This is SPI_INT_CPU_ON interupt handle \r\n ");
			HAL_UART_Transmit(huart, (uint8_t *)tx_buffer, strlen(tx_buffer), QCA_TIME_OUT);
			QCA_Init_Setup(hspi, GPIOx, GPIO_Pin, huart);
		}
	if(interupt & SPI_INT_ADDR_ERR)
		{
			char tx_buffer[60];
			sprintf(tx_buffer, "Hello, This is SPI_INT_ADDR_ERR interupt handle \r\n ");
			HAL_UART_Transmit(huart, (uint8_t *)tx_buffer, strlen(tx_buffer), QCA_TIME_OUT);
		}
	if(interupt & SPI_INT_WRBUF_ERR)
		{
			char tx_buffer[60];
			sprintf(tx_buffer, "Hello, This is SPI_INT_WRBUF_ERR interupt handle \r\n ");
			HAL_UART_Transmit(huart, (uint8_t *)tx_buffer, strlen(tx_buffer), QCA_TIME_OUT);
			QCA_Write_Reg_SPI_Config(hspi, GPIOx, GPIO_Pin, QCASPI_SLAVE_RESET_BIT);
		}
	if(interupt & SPI_INT_RDBUF_ERR)
		{
			char tx_buffer[60];
			sprintf(tx_buffer, "Hello, This is SPI_INT_RDBUF_ERR interupt handle \r\n ");
			HAL_UART_Transmit(huart, (uint8_t *)tx_buffer, strlen(tx_buffer), QCA_TIME_OUT);
			QCA_Write_Reg_SPI_Config(hspi, GPIOx, GPIO_Pin, QCASPI_SLAVE_RESET_BIT);
		}
	if(interupt & SPI_INT_PKT_AVLBL)
		{
			char tx_buffer[60];
			sprintf(tx_buffer, "Hello, This is SPI_INT_PKT_AVLBL interupt handle \r\n ");
			HAL_UART_Transmit(huart, (uint8_t *)tx_buffer, strlen(tx_buffer), QCA_TIME_OUT);
			QCA_External_Read(hspi, GPIOx, GPIO_Pin, huart);
		}

	QCA_Write_Reg_INT_Cause(hspi, GPIOx, GPIO_Pin, interupt);
	QCA_Enalbe_All_IT(hspi, GPIOx, GPIO_Pin,huart);
}

void QCA_External_Write(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart, uint8_t* str)
{
	uint8_t tx3[]={0x00, 0x00};

	uint16_t avai_data=QCA_Get_Avai_Write_data_length(hspi, GPIOx, GPIO_Pin);
	char tx_buffer1[50];
	sprintf(tx_buffer1, "Hello, Available writen Data= %u\r\n", avai_data);
	HAL_UART_Transmit(huart, (uint8_t *)tx_buffer1, strlen(tx_buffer1), QCA_TIME_OUT);

	QCA_Write_Reg_Buffer_Size(hspi, GPIOx, GPIO_Pin, 70); //ghi 70 byte qua internal write

	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, tx3, 2, QCA_TIME_OUT);
	HAL_SPI_Transmit(hspi, str, 0x46, QCA_TIME_OUT);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);

    char tx_buffer2[50 + 70 * 3];  // Độ dài của buffer có thể được điều chỉnh

    sprintf(tx_buffer2, "Hello, Writen data is: \r\n");
    for (int i = 0; i < 70; ++i)
    {
    	sprintf(tx_buffer2 + strlen(tx_buffer2), "%02X ", str[i]);
    }

    strcat(tx_buffer2, "\r\n");
    HAL_UART_Transmit(huart, (uint8_t *)tx_buffer2, strlen(tx_buffer2), QCA_TIME_OUT);

}

void QCA_External_Read(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart)
{
	uint8_t tx3[]={0x80, 0x00};
	uint8_t rx4[QCA_READ_MIN_SIZE];

	uint16_t avai_data=QCA_Get_Avai_Read_data_length(hspi, GPIOx, GPIO_Pin);
	QCA_Write_Reg_Buffer_Size(hspi, GPIOx, GPIO_Pin, avai_data);
	//QCA_Write_Reg_Buffer_Size(hspi, GPIOx, GPIO_Pin, 80);
	char tx_buffer1[50];
	sprintf(tx_buffer1, "Hello, Available readed Data= %u\r\n", avai_data);
	HAL_UART_Transmit(huart, (uint8_t *)tx_buffer1, strlen(tx_buffer1), QCA_TIME_OUT);


	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, tx3, 2, QCA_TIME_OUT);
	HAL_SPI_Receive(hspi, rx4, 74, QCA_TIME_OUT);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);



	char tx_buffer2[50 + 74 * 3];
	sprintf(tx_buffer2, "Hello, readed Data is \r\n ");

	    // Append each element of rx4 to tx_buffer2
	    for (int i = 0; i < 74; ++i) {
	        sprintf(tx_buffer2 + strlen(tx_buffer2), "%02X ", rx4[i]);
	    }

	    strcat(tx_buffer2, "\r\n");

	HAL_UART_Transmit(huart, (uint8_t *)tx_buffer2, strlen(tx_buffer2), QCA_TIME_OUT);
}



