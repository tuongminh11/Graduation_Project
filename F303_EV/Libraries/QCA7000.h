/*
 * QCA7000.h
 *
 *  Created on: Nov 20, 2023
 *      Author: PC
 */



#define SERIAL_BUFFER_SIZE 2100
#define SPI_BUFFER_SIZE 2100
#define ETH_TRANSMIT_BUFFER_SIZE 250
#define ETH_RECEIVE_BUFFER_SIZE 250

/* Global Variables */

extern char serial_output_buffer[SERIAL_BUFFER_SIZE];
extern char serial_input_buffer[SERIAL_BUFFER_SIZE];
extern uint16_t serial_data_size;

extern uint8_t spi_tx_buffer[SPI_BUFFER_SIZE];
extern uint8_t spi_rx_buffer[SPI_BUFFER_SIZE];
extern uint16_t spi_data_size;

extern uint8_t eth_tx_buffer[ETH_TRANSMIT_BUFFER_SIZE];
extern uint16_t eth_tx_size;

extern uint8_t eth_rx_buffer[ETH_RECEIVE_BUFFER_SIZE];
extern uint16_t eth_rx_size;

extern uint8_t eth_2nd_rx_buffer[ETH_RECEIVE_BUFFER_SIZE];
extern uint16_t eth_2nd_rx_size;

extern uint16_t debugCounter_cutted_myethreceivebufferLen;

/* Global Functions */

extern uint16_t SPI_QCA7000_Read_Signature(void);
extern uint16_t SPI_QCA7000_Read_WRBUF_SPC_AVA(void);
extern uint16_t SPI_QCA7000_Read_RDBUF_SPC_AVA(void);
extern void SPI_QCA7000_Write_BFR_SIZE(uint16_t len);

extern void SPI_QCA7000_Disable_REG_INTR_ENABLE(void);
extern void SPI_QCA7000_Enable_REG_INTR_ENABLE(void);
extern uint16_t SPI_QCA7000_Read_REG_INTR_CAUSE(void);
extern void SPI_QCA7000_Confirm_REG_INTR_CAUSE(uint16_t cause);

extern void SPI_QCA7000_Reset_Config(void);
extern void SPI_QCA7000_Write_Config(void);
extern void SPI_QCA7000_Init_Setup(void);

extern void SPI_QCA7000_Check_Rx_Data(uint16_t available_bytes);
extern void SPI_QCA7000_Send_Eth_Frame(void);
extern void SPI_QCA7000_Read_Eth_Frame(void);
extern void SPI_QCA7000_Handling_Intr(void);

extern void Serial_Print(void);
extern void SPI_Transmit_Receive(void);


