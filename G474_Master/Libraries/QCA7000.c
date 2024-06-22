/*
 * QCA7000.c
 *
 *  Created on: Nov 20, 2023
 *      Author: PC
 */

#include "cims.h"

char serial_output_buffer[SERIAL_BUFFER_SIZE]={'\0'};
char serial_input_buffer[SERIAL_BUFFER_SIZE];
uint16_t serial_data_size;

uint8_t spi_tx_buffer[SPI_BUFFER_SIZE];
uint8_t spi_rx_buffer[SPI_BUFFER_SIZE];
uint16_t spi_data_size;

uint8_t eth_tx_buffer[ETH_TRANSMIT_BUFFER_SIZE];
uint16_t eth_tx_size;

uint8_t eth_rx_buffer[ETH_RECEIVE_BUFFER_SIZE];
uint16_t eth_rx_size;

uint8_t eth_2nd_rx_buffer[ETH_RECEIVE_BUFFER_SIZE];
uint16_t eth_2nd_rx_size;

uint16_t debugCounter_cutted_myethreceivebufferLen;

void SPI_Transmit_Receive();
void Serial_Print();


uint16_t SPI_QCA7000_Read_Signature(void){
    uint16_t sig;
    uint8_t i;
    i = 0;
    spi_tx_buffer[i++] = 0xDA;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_data_size = i;
    SPI_Transmit_Receive();

    sig = spi_rx_buffer[2];
    sig <<= 8;
    sig += spi_rx_buffer[3];

    sprintf(serial_output_buffer, "Hello, sig is %X ", sig);
    Serial_Print();

    return sig;
}

uint16_t SPI_QCA7000_Read_WRBUF_SPC_AVA(void){
    uint16_t len;
    uint8_t i;
    i = 0;
    spi_tx_buffer[i++] = 0xC2;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_data_size = i;
    SPI_Transmit_Receive();

    len = spi_rx_buffer[2];
    len <<= 8;
    len += spi_rx_buffer[3];

//    sprintf(serial_output_buffer,"WRBUF_SPC_AVA is %X ", len);
//    Serial_Print();

    return len;
}

uint16_t SPI_QCA7000_Read_RDBUF_SPC_AVA(void){
    uint16_t len;
    uint8_t i;
    i = 0;
    spi_tx_buffer[i++] = 0xC3;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_data_size = i;
    SPI_Transmit_Receive();

    len = spi_rx_buffer[2];
    len <<= 8;
    len += spi_rx_buffer[3];

//    sprintf(serial_output_buffer,"RDBUF_SPC_AVA is %X ", len);
//    Serial_Print();

    return len;
}

void SPI_QCA7000_Write_BFR_SIZE(uint16_t len){
    uint8_t i;
    i = 0;
    spi_tx_buffer[i++] = 0x41;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = len >> 8;
    spi_tx_buffer[i++] = (uint8_t)len;
    spi_data_size = i;
    SPI_Transmit_Receive();
}

void SPI_QCA7000_Disable_REG_INTR_DISABLE(void){
    uint8_t i;
    i=0;
    spi_tx_buffer[i++] = 0x4D;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_data_size = i;
    SPI_Transmit_Receive();

//    sprintf(serial_output_buffer, "REG_INTR_ENABLE is disabled ");
//    Serial_Print();
}

void SPI_QCA7000_Enable_REG_INTR_ENABLE(void){
    uint8_t i;
    i=0;
    spi_tx_buffer[i++] = 0x4D;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x47;
    spi_data_size = i;
    SPI_Transmit_Receive();

//    sprintf(serial_output_buffer, "REG_INTR_ENABLE is enabled ");
//    Serial_Print();
}

uint16_t SPI_QCA7000_Read_REG_INTR_CAUSE(void){
    uint16_t cause;
    uint8_t i;
    i=0;
    spi_tx_buffer[i++] = 0xCC;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_data_size = i;
    SPI_Transmit_Receive();

    cause = spi_rx_buffer[2];
    cause <<= 8;
    cause += spi_rx_buffer[3];

    if(cause & 0x0400){
    	sprintf(serial_output_buffer, "REG_INTR_CAUSE is %X, SPI_INT_WRBUF_BELOW_WM ", cause);
//    	Serial_Print();
    }
    if(cause & 0x0040){
    	sprintf(serial_output_buffer, "REG_INTR_CAUSE is %X, SPI_INT_CPU_ON ", cause);
    	SPI_QCA7000_Init_Setup();
//    	Serial_Print();
    }
    if(cause & 0x0008){
    	sprintf(serial_output_buffer, "REG_INTR_CAUSE is %X, SPI_INT_ADDR_ERR ", cause);
//    	Serial_Print();
    }
    if(cause & 0x0004){
    	sprintf(serial_output_buffer, "REG_INTR_CAUSE is %X, SPI_INT_WRBUF_ERR ", cause);
    	SPI_QCA7000_Write_Config();
//    	Serial_Print();
    }
    if(cause & 0x0002){
//    	sprintf(serial_output_buffer, "REG_INTR_CAUSE is %X, SPI_INT_RDBUF_ERR ", cause);
    	SPI_QCA7000_Write_Config();
//    	Serial_Print();
    }
    if(cause & 0x0001){
//    	sprintf(serial_output_buffer, "REG_INTR_CAUSE is %X, SPI_INT_PKT_AVLBL ", cause);
//    	Serial_Print();
    	SPI_QCA7000_Read_Eth_Frame();
    }

    return cause;
}

void SPI_QCA7000_Confirm_REG_INTR_CAUSE(uint16_t cause){
    uint8_t i;
    i=0;
    spi_tx_buffer[i++] = 0x4C;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = cause >> 8;
    spi_tx_buffer[i++] = (uint8_t)cause;
    spi_data_size = i;
    SPI_Transmit_Receive();
}

void SPI_QCA7000_Reset_Config(void){
    uint8_t i;
    i = 0;
    spi_tx_buffer[i++] = 0x44;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x40;
    spi_data_size = i;
    SPI_Transmit_Receive();

    sprintf(serial_output_buffer, "Reset Configuration ");
    Serial_Print();
}

void SPI_QCA7000_Write_Config(void){
    uint8_t i;
    i = 0;
    spi_tx_buffer[i++] = 0x44;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_tx_buffer[i++] = 0x00;
    spi_data_size = i;
    SPI_Transmit_Receive();

    sprintf(serial_output_buffer, "Set Configuration ");
    Serial_Print();
}

void SPI_QCA7000_Init_Setup(void){
	uint16_t sig;
	sig = SPI_QCA7000_Read_Signature();
	sig = 0x0000;
	sig = SPI_QCA7000_Read_Signature();
	if(sig == 0xAA55) SPI_QCA7000_Enable_REG_INTR_ENABLE();
}

void SPI_QCA7000_Check_Rx_Data(uint16_t available_bytes){
	 uint16_t  spi_len, eth_len;
	 uint8_t *p;
	 uint8_t  blDone = 0;
//	 uint8_t counterOfEthFramesInSpiFrame;
//	 counterOfEthFramesInSpiFrame = 0;
	 p = spi_rx_buffer;

	 while (!blDone) {  /* The SPI receive buffer may contain multiple Ethernet frames. Run through all. */
	       /* the SpiRxBuffer contains more than the ethernet frame:
	         4 byte length
	         4 byte start of frame AA AA AA AA
	         2 byte frame length, little endian
	         2 byte reserved 00 00
	         payload
	         2 byte End of frame, 55 55 */
	       /* The higher 2 bytes of the len are assumed to be 0. */
	       /* The lower two bytes of the "outer" len, big endian: */
	       spi_len = p[2]; spi_len<<=8; spi_len+=p[3];
	       /* The "inner" len, little endian. */
	       eth_len = p[9]; eth_len<<=8; eth_len+=p[8];
	       if ((p[4]=0xAA) && (p[5]=0xAA) && (p[6]=0xAA) && (p[7]=0xAA)
	             && (eth_len+10 == spi_len)) {
	           //counterOfEthFramesInSpiFrame++;
	           /* The start of frame and the two length informations are plausible. Copy the payload to the eth receive buffer. */
	    	   eth_rx_size = eth_len;
	           /* but limit the length, to avoid buffer overflow */
//	           if (eth_rx_size > ETH_RECEIVE_BUFFER_SIZE) {
//	               eth_rx_size = ETH_RECEIVE_BUFFER_SIZE;
//	               debugCounter_cutted_myethreceivebufferLen++;
//	           }
	           memcpy(eth_rx_buffer, &p[12], eth_rx_size);
	           /* We received an ethernet package. Determine its type, and dispatch it to the related handler. */
	           uint16_t mtype = HPGP_Get_MTYPE(eth_rx_buffer);
	           if (mtype == 0x88E1) {
	        	   /* it is a HomePlug message */
                   //Serial.println("Its a HomePlug message.");
                   sprintf(serial_output_buffer, "Its a HomePlug message.");
                   Serial_Print();
                   //evaluateReceivedHomeplugPacket();
//                   HPGP_Evaluate_HomePlug_Packet();
         		  PEF_Evaluate_Exchange_Data();
               }
	           /* Code for evse respond*/
//	           if(eth_rx_buffer[15] == 0x64 && eth_rx_buffer[16] == 0x60){
//	        	   HPGP_EVSE_Respond_SLAC_PARM_CNF();
//	        	   SPI_QCA7000_Send_Eth_Frame();
//	           }
	           /*-------------------------*/
	           available_bytes = available_bytes - spi_len - 4;
	           p += spi_len+4;
	           if(available_bytes == 0){
                   sprintf(serial_output_buffer, "Only one frame! ");
                   Serial_Print();
	           }
	           else if(available_bytes != 0){
	        	   spi_len = p[2]; spi_len<<=8; spi_len+=p[3];
	        	   eth_len = p[9]; eth_len<<=8; eth_len+=p[8];
	        	   if ((p[4]=0xAA) && (p[5]=0xAA) && (p[6]=0xAA) && (p[7]=0xAA)
	        	   	             && (eth_len+10 == spi_len)) {
	        		   eth_2nd_rx_size = eth_len;
	           	   }
	        	   memcpy(eth_2nd_rx_buffer, &p[12], eth_2nd_rx_size);
	           }
	           //Serial.println("Avail after first run:" + String(available_bytes));
//	           if (available_bytes>10) { /*
//	             Serial.println("There is more data.");
//	             Serial.print(String(p[0], HEX) + " ");
//	             Serial.print(String(p[1], HEX) + " ");
//	             Serial.print(String(p[2], HEX) + " ");
//	             Serial.print(String(p[3], HEX) + " ");
//	             Serial.print(String(p[4], HEX) + " ");
//	             Serial.print(String(p[5], HEX) + " ");
//	             Serial.print(String(p[6], HEX) + " ");
//	             Serial.print(String(p[7], HEX) + " ");
//	             Serial.print(String(p[8], HEX) + " ");
//	             Serial.print(String(p[9], HEX) + " ");
//	             */
//	           } else {
//	             blDone=1;
//	           }
	           blDone=1;
	     }
	     else {
	         /* no valid header -> end */
	         blDone=1;
	     }
	 }
}

void SPI_QCA7000_Send_Eth_Frame(void){
    SPI_QCA7000_Read_WRBUF_SPC_AVA();
    SPI_QCA7000_Write_BFR_SIZE(eth_tx_size + 10);

    spi_tx_buffer[0] = 0x00;
    spi_tx_buffer[1] = 0x00;
    spi_tx_buffer[2] = 0xAA;
    spi_tx_buffer[3] = 0xAA;
    spi_tx_buffer[4] = 0xAA;
    spi_tx_buffer[5] = 0xAA;
    spi_tx_buffer[6] = (uint8_t)eth_tx_size;
    spi_tx_buffer[7] = eth_tx_size >> 8;
    spi_tx_buffer[8] = 0x00;
    spi_tx_buffer[9] = 0x00;
    memcpy(&spi_tx_buffer[10], eth_tx_buffer, eth_tx_size);
    spi_tx_buffer[10 + eth_tx_size] = 0x55;
    spi_tx_buffer[11 + eth_tx_size] = 0x55;
    spi_data_size = 12 + eth_tx_size;
    SPI_Transmit_Receive();

//    uint8_t i;
//    for(i=0; i<spi_data_size; i++){
//    	sprintf(serial_output_buffer + strlen(serial_output_buffer), " %X ", spi_tx_buffer[i]);
//    }
//    Serial_Print();
}

void SPI_QCA7000_Read_Eth_Frame(void){
    uint8_t i;
    i = 0;
    uint16_t ava_size;
    ava_size = SPI_QCA7000_Read_RDBUF_SPC_AVA();
    if(ava_size == 0){
    	return;
    }
    SPI_QCA7000_Write_BFR_SIZE(ava_size);

    spi_tx_buffer[i++] = 0x80;
    spi_tx_buffer[i++] = 0x00;

    spi_data_size = ava_size + 2;

    SPI_Transmit_Receive();

    for(i=0; i<ava_size; i++){
        spi_rx_buffer[i] = spi_rx_buffer[i+2];
   		sprintf(serial_output_buffer + strlen(serial_output_buffer), " %X ", spi_rx_buffer[i]);
    }
	Serial_Print();

    SPI_QCA7000_Check_Rx_Data(ava_size);
//	sprintf(serial_output_buffer, "abc: %X %X ", spi_rx_buffer[2], spi_rx_buffer[3] );
//	Serial_Print();
}

void SPI_QCA7000_Handling_Intr(void){
    SPI_QCA7000_Disable_REG_INTR_DISABLE();
    uint16_t cause;
    cause = SPI_QCA7000_Read_REG_INTR_CAUSE();
    SPI_QCA7000_Confirm_REG_INTR_CAUSE(cause);
    SPI_QCA7000_Enable_REG_INTR_ENABLE();
}



/* Setup on main.c
void Serial_Print(void) {
  #define TIMEOUT_100_MS 100
  HAL_UART_Transmit(&huart1, (uint8_t*)serial_output_buffer, strlen(serial_output_buffer), TIMEOUT_100_MS);
}

void SPI_Transmit_Receive(void) {
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_TransmitReceive(&hspi1, spi_tx_buffer, spi_rx_buffer, spi_data_size, 200);
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
}
-------------------------*/

