/*
 * QCA.h
 *
 *  Created on: Nov 20, 2023
 *      Author: Admin
 */
#ifndef INC_QCA_H_
#define INC_QCA_H_

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QCA_TIME_OUT 1000

#define QCA_READ_MAX_SIZE 1532
#define QCA_READ_MIN_SIZE 70

#define QCA_WRITE_MAX_SIZE 1532
#define QCA_WRITE_MIN_SIZE 70

#define QCA7K_SPI_READ (1 << 15)
#define QCA7K_SPI_WRITE (0 << 15)
#define QCA7K_SPI_INTERNAL (1 << 14)
#define QCA7K_SPI_EXTERNAL (0 << 14)

#define QCASPI_CMD_LEN 2
#define QCASPI_HW_PKT_LEN 4
#define QCASPI_HW_BUF_LEN 0xC5B

#define QCASPI_GOOD_SIGNATURE 0xAA55

#define TX_QUEUE_LEN 10

/*====================================================================*
 *   SPI registers;
 *--------------------------------------------------------------------*/

#define	SPI_REG_BFR_SIZE        0x0100
#define SPI_REG_WRBUF_SPC_AVA   0x0200
#define SPI_REG_RDBUF_BYTE_AVA  0x0300
#define SPI_REG_SPI_CONFIG      0x0400
#define SPI_REG_INTR_CAUSE      0x0C00
#define SPI_REG_INTR_ENABLE     0x0D00
#define SPI_REG_RDBUF_WATERMARK 0x1200
#define SPI_REG_WRBUF_WATERMARK 0x1300
#define SPI_REG_SIGNATURE       0x1A00
#define SPI_REG_ACTION_CTRL     0x1B00

/*====================================================================*
 *   SPI_CONFIG register definition;
 *--------------------------------------------------------------------*/

#define QCASPI_SLAVE_RESET_BIT (1 << 6)

/*====================================================================*
 *   INTR_CAUSE/ENABLE register definition.
 *--------------------------------------------------------------------*/

#define SPI_INT_WRBUF_BELOW_WM (1 << 10)		//Interrupt for Write Buffer Below Watermark.
#define SPI_INT_CPU_ON         (1 << 6)			//Interrupt for CPU Status On.
#define SPI_INT_ADDR_ERR       (1 << 3)			//Interrupt for Address Error.
#define SPI_INT_WRBUF_ERR      (1 << 2)			//Interrupt for Write Buffer Error.
#define SPI_INT_RDBUF_ERR      (1 << 1)			//Interrupt for Read Buffer Error.
#define SPI_INT_PKT_AVLBL      (1 << 0)			//Interrupt for Packet Available.

/*====================================================================*
 *   ACTION_CTRL register definition.
 *--------------------------------------------------------------------*/

#define SPI_ACTRL_PKT_AVA_SPLIT_MODE (1 << 8)
#define SPI_ACTRL_PKT_AVA_INTR_MODE (1 << 0)




typedef struct
{
    uint8_t SoF[4];  //Start Of Frame
    uint8_t FL[2];   //The Ethernet frame length, min 60 max 1522 or 1518 if VLAN is omitted
    uint8_t RSVD[2]; //Reserved to ensure 4-byte frame alignment
    uint8_t DA[6];   //Destination MAC Address
    uint8_t SA[6];   //Source MAC Address
    uint8_t ET[2];   //Ethertype
    uint8_t MMEV[1];    //MME Version
    uint8_t MMET[2]; //MME Type
    uint8_t OUT[3];  //Intellon OUT
    uint8_t *data;   //actual data
    uint8_t EoF[2];  //end of frame
} QCA_Frame;


void uint64ToUint8Array(uint64_t value, uint8_t *array);

void QCA_Frame_Init(QCA_Frame *myFrame, uint64_t sourceadd,uint64_t destinationadd, uint16_t datalength);

void QCA_Frame_Package(QCA_Frame myFrame, uint8_t *data,uint16_t datalength, uint8_t transmitBuffer[]);

void QCA_Transmit(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart, uint16_t cmd, uint16_t loc, uint16_t reg);

void QCA_Reset_SPI_Config(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart);

void QCA_Check_Signature(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart);

uint16_t QCA_Get_Signature(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,UART_HandleTypeDef *huart);

uint16_t QCA_Get_Avai_Read_data_length(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

uint16_t QCA_Get_Avai_Write_data_length(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

void QCA_Write_Reg_Buffer_Size(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint16_t size);

uint16_t QCA_Read_Reg_INT_Cause(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

void QCA_Write_Reg_INT_Cause(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint16_t interupt);

void QCA_Write_Reg_SPI_Config(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint16_t Config);

void QCA_Enalbe_All_IT(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart);

void QCA_Disalbe_All_IT(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart);

void QCA_Init_Setup(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,UART_HandleTypeDef *huart);

void QCA_Handling_IT(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart);

void QCA_External_Write(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart, uint8_t* str);

void QCA_External_Read(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UART_HandleTypeDef *huart);

#endif /* INC_QCA_H_ */




