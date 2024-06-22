/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "HMI.h"
#include "stdlib.h"
#include "mycanopen.h"
#include "MyFLASH.h"
#include "myerror.h"
#include "string.h"
#include "../../Libraries/cims.h"
#include "ToESP.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

FDCAN_HandleTypeDef hfdcan2;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim17;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_uart4_rx;
DMA_HandleTypeDef hdma_usart3_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_FDCAN2_Init(void);
static void MX_UART4_Init(void);
static void MX_TIM17_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define TIMEOUT 1000			//maximum time for uart and SPI transmit
#define TPDO_Event_Time 100
#define current_multiplier 1
#define voltage_multiplier 1
#define temperature_multiplier 1
#define Maximum_number_of_Slave 6							//nen la 8*n-2
#define Maximum_time_not_update_of_IDs 5000
#define Maximum_number_of_IDs (Maximum_number_of_Slave+2)	//number of ID, =number of slave+2, 1 for master, 1 for initial slave
#define Number_of_Used_Slave 2								//<Maximum_number_of_Slave

#define UART_SIZE 4
uint8_t buffer[UART_SIZE]={0};
uint8_t size = 0;
uint8_t flag = 0;

uint32_t previoustick_master = 0;
uint32_t previoustick_slave[Maximum_number_of_IDs];
uint32_t previoustick_HMI = 0;
uint32_t previoustick_ESP=0;
uint32_t previoustick_SPI = 0;
uint32_t previoustick_calculate = 0;
uint32_t previoustick_CAN = 0;


uint8_t uart4_rx_data[8];
uint8_t uart4_tx_data[8];
uint8_t uart3_rx_data[8];
uint8_t uart1_rx_data[8];
uint8_t uart3_tx_data[8];

FDCAN_TxHeaderTypeDef CAN_Master_Tx_Header;
FDCAN_RxHeaderTypeDef CAN_Master_Rx_Header;

uint8_t CAN_Master_Tx_Data[8];
uint8_t CAN_Master_Rx_Data[8];

SPDO_Data Master_Tx_SPDO_Data;
SPDO_Data Master_Rx_SPDO_Data;

SPDO_Data Slave_Data[Maximum_number_of_Slave];

uint8_t list_node_available[Maximum_number_of_IDs];		//Stored list of IDs
uint32_t list_node_update[Maximum_number_of_IDs];		//The list saves the time that the IDs were last updated

int uart_flag=0;
int button_flag=0;
int button_flag2=0;
int can_flag=0;

int can_setting_confirm_flag=0;	//so luong thong so da cai dat xong, 1 node co 4 thong so
int can_node_number=2;	//so node dang tham gia mang, se cap nhat trong timer

uint16_t EVstate;
uint16_t Current=30;
uint16_t Voltage=0;
uint16_t Temperature=50;
uint16_t Energy=0;
uint16_t random_number() {
    return rand() % 2;
}
uint16_t random_voltage() {
    return rand() % 30;
}
void Serial_Print(void) {
  #define TIMEOUT_100_MS 100
  HAL_UART_Transmit(&huart1, (uint8_t*)serial_output_buffer, strlen(serial_output_buffer), TIMEOUT_100_MS);
}
void ESP_Send(void)	{
  HAL_UART_Transmit(&huart3, ESP_Payload, 8, 100);
}

void SPI_Transmit_Receive(void) {
  HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_TransmitReceive(&hspi2, spi_tx_buffer, spi_rx_buffer, spi_data_size, 200);
  HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
}

void HMI_Print(void) {
  HAL_UART_Transmit(&huart4, (uint8_t*)hmi_buffer, strlen(hmi_buffer), TIMEOUT_100_MS);
  uint8_t END_BYTE = 0xFF;
  HAL_UART_Transmit(&huart4, &END_BYTE, 1, 10);
  HAL_UART_Transmit(&huart4, &END_BYTE, 1, 10);
  HAL_UART_Transmit(&huart4, &END_BYTE, 1, 10);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  UNUSED(huart);
  if(huart->Instance==huart3.Instance)	//ESP data
  {
	  HAL_UART_Receive_DMA(&huart3, uart3_rx_data, sizeof(uart3_rx_data));
	  sprintf(serial_output_buffer,"Data received from ESP is: ");
	  for(int i=0;i<5;i++)
	  {
		  sprintf(serial_output_buffer+strlen(serial_output_buffer),"%02X ",uart3_rx_data[i]);
	  }
	  Serial_Print();
	  if(uart3_rx_data[2]==0x15)
	  {
		  switch (uart3_rx_data[3]) {
		  case 0x01:
			 // HMI_Compose_Status(HMI_PAGE1);
			  uint8_t data_to_hmi[9]={0x70, 0x61, 0x67, 0x65, 0x20, 0x31 ,0xFF, 0xFF, 0xFF};
			  HAL_UART_Transmit(&huart4, data_to_hmi, 9, 100);
			  break;
		  case 0x00:
			  break;
		  default:
			  break;
		  }
	  }
//	  else if(uart3_rx_data[2]==0x1B)
//	  {
//
//	  }

  }
  if(huart->Instance==huart1.Instance)	//ESP data
  {
	  HAL_UART_Receive_DMA(&huart1, uart1_rx_data, sizeof(uart1_rx_data));
	  sprintf(serial_output_buffer,"Data received from ESP is: ");
	  for(int i=0;i<8;i++)
	  {
		  sprintf(serial_output_buffer+strlen(serial_output_buffer),"%02X ",uart1_rx_data[i]);
	  }
	  Serial_Print();
  }
  if(huart->Instance==huart4.Instance)	//HMI data
  {
	  HAL_UART_Receive_DMA(&huart4, uart4_rx_data, sizeof(uart4_rx_data));
	  //loc truoc khi nhan, co the day la du lieu loi 1A FF FF FF nen can loai bo
	  if(uart4_rx_data[0]>0x29 && uart4_rx_data[0]<0x60)
	  {
		  uart_flag=1;
		  sprintf(serial_output_buffer,"Data received from HMI is: ");
		  for(int i=0;i<8;i++)
		  {
			  sprintf(serial_output_buffer+strlen(serial_output_buffer),"%02X ",uart4_rx_data[i]);
		  }
		  Serial_Print();

		  if(uart4_rx_data[0]==0x59)
		  {
			  //start
			  sprintf(serial_output_buffer,"HMI gui goi tin Start ");
			  Serial_Print();
			  Packet_to_ESP(TYPE_CONNECTER_STATUS,STATE_ON);
			  Packet_to_ESP(TYPE_HMI_CONTROL_TRANSACTION,STATE_ON);     // Sent start to ESp
			  Master_Tx_SPDO_Data.state=CHARGING_ON;
			  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
			  PEF_Compose_Contactor_Close_Req();
			  SPI_QCA7000_Send_Eth_Frame();
			  sprintf(serial_output_buffer,"Send PEF_Compose_Contactor_Close_Req() ");
			  Serial_Print();
		  }
		  else if(uart4_rx_data[0]==0x30)
		  {
			  sprintf(serial_output_buffer,"HMI gui goi tin Logout ");
			  Serial_Print();
			  Packet_to_ESP(TYPE_ID_TAG,STATE_OFF);
		  }
		  else if(uart4_rx_data[0]==0x4E)
		  {
			  //stop
			  sprintf(serial_output_buffer,"HMI gui goi tin Stop ");
			  Serial_Print();
			  ESP_Data.ESP_Data_voltage=0;
			  ESP_Data.ESP_Data_current=0;
			  Packet_to_ESP(TYPE_CURRENT_VALUE,STATE_ON);
			  Packet_to_ESP(TYPE_VOLTAGE_VALUE,STATE_ON);
			  Packet_to_ESP(TYPE_HMI_CONTROL_TRANSACTION,STATE_OFF);
			  Master_Tx_SPDO_Data.state=CHARGING_OFF;
			  can_setting_confirm_flag=0;
			  CAN_Master_Tx_Data[0]=0x02;
			  Master_Tx_SPDO_Data.type_of_value=CHARGING_DATA_CURRENT;
			  button_flag=0;
			  button_flag2=0;
			  uart_flag=0;
			  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
		  }
		  else
		  {
			  //data
			  //Master_Tx_SPDO_Data.state=CHARGING_ON;
			  sprintf(serial_output_buffer,"HMI gui goi tin Setting ");
			  Serial_Print();
			  HMI_Evaluate_Setting_Data(uart4_rx_data);
		  }

			switch(myHMI.mode)
			{
				case '0':
					Master_Tx_SPDO_Data.mode=CHARGING_MODE_1;
					Master_Tx_SPDO_Data.current_value=myEV.charging_current_request;
					break;
				case '1':
					Master_Tx_SPDO_Data.mode=CHARGING_MODE_2;
					Master_Tx_SPDO_Data.current_value=myEV.charging_current_request;
					break;
				case '2':
					Master_Tx_SPDO_Data.mode=CHARGING_MODE_3;
					Master_Tx_SPDO_Data.current_value=myEV.charging_current_request;
					break;
				case '3':
					Master_Tx_SPDO_Data.mode=CHARGING_MODE_4;
					Master_Tx_SPDO_Data.current_value=myHMI.current/can_node_number;
					break;
			}

			switch(myHMI.cable)
			{
				case '0':	Master_Tx_SPDO_Data.mode=CHARGING_PLUG_1; break;
				case '1':	Master_Tx_SPDO_Data.mode=CHARGING_PLUG_2; break;
			}
			Master_Tx_SPDO_Data.time_value=myHMI.time;
			Master_Tx_SPDO_Data.voltage_value=myHMI.voltage/can_node_number;
	  }
	  memset(uart4_rx_data, 0, sizeof(uart4_rx_data));
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_13)
	{
	}

	if(GPIO_Pin == QCA_INT_Pin)
	{
		SPI_QCA7000_Handling_Intr();
	}

}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &CAN_Master_Rx_Header, CAN_Master_Rx_Data) != HAL_OK)
		{
			/* Reception Error */
			Error_Handler();
		}
		if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
		{
			/* Notification Error */
			Error_Handler();
		}
		HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
		can_flag=1;

		deframe_PDO(CAN_Master_Rx_Data, &Master_Rx_SPDO_Data);

		int id=(int)CAN_Master_Rx_Header.Identifier;
		list_node_update[id]=HAL_GetTick();			//lastest time this ID node communicate with CIMS
		if(list_node_available[id]!=ID_IN_USED)
		{
			list_node_available[id]=ID_IN_USED;			//ID connect/reconnect with the CIMS
			sprintf(serial_output_buffer,"Node 0x%02X reconnect vao mang ",id);
			Serial_Print();
		}

		deframe_PDO(CAN_Master_Rx_Data, &Slave_Data[id-2]);	//slave data


		if(Master_Rx_SPDO_Data.state==CHARGING_INIT) //có slave mới tham gia mang
		{
			if(CAN_Master_Rx_Header.Identifier==CHARGING_SLAVE_ID)	//slave default ID => new node join and need a new ID
			{
				sprintf(serial_output_buffer,"Co node moi tham gia, ID mong muon= 0x%02X ",Master_Rx_SPDO_Data.slaveID);
				Serial_Print();
				int desireID=(int)Master_Rx_SPDO_Data.slaveID;
				if(list_node_available[desireID]==ID_NOT_USED)
				{
					Master_Tx_SPDO_Data.slaveID=Master_Rx_SPDO_Data.slaveID;
					list_node_available[desireID]=ID_IN_USED;

					//send message, if a slave is new (ID=0) then it will have a new ID
					uint8_t state_temp=Master_Tx_SPDO_Data.state;	//luu trang thai master hien tai
					uint8_t destinationaddress=CAN_Master_Tx_Data[0]; //luu dia chi dich hien tai
					Master_Tx_SPDO_Data.state=CHARGING_INIT;		//tam thoi thay doi trang thai master sang khoi tao slave
					CAN_Master_Tx_Data[0]=CHARGING_SLAVE_ID;		//tam thoi thay doi dia chi dich toi slave moi khoi tao
					PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
					HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);
					//send message to uart3
					sprintf(serial_output_buffer,"Node moi co ID= 0x%02X ",Master_Tx_SPDO_Data.slaveID);
					Serial_Print();

					//write list id to flash
					uint64_t datawwritetoflash[Maximum_number_of_IDs/8];
					uint8_array_to_uint64_array_big_endian(list_node_available, Maximum_number_of_IDs/8, datawwritetoflash);
					writeFlash(datawwritetoflash, Maximum_number_of_IDs/8, 127, FLASH_BANK_1);

					Master_Tx_SPDO_Data.state=state_temp;			//tra lai trang thai cu cho master
					CAN_Master_Tx_Data[0]=destinationaddress;		//tra lai dia chi dich cu cho master
				}
				else
				{
					int i = 2;
					while (i < Maximum_number_of_IDs)
					{
						if (list_node_available[i] == ID_NOT_USED)
						{
							Master_Tx_SPDO_Data.slaveID=i;			//nodeID cho slave mới tham gia, có giá tri từ 2-Maximum_number_of_IDs
							list_node_available[i]=ID_IN_USED;

							//send message, if a slave is new (ID=0) then it will have a new ID
							uint8_t state_temp=Master_Tx_SPDO_Data.state;	//luu trang thai master hien tai
							uint8_t destinationaddress=CAN_Master_Tx_Data[0]; //luu dia chi dich hien tai
							Master_Tx_SPDO_Data.state=CHARGING_INIT;		//tam thoi thay doi trang thai master sang khoi tao slave
							CAN_Master_Tx_Data[0]=CHARGING_SLAVE_ID;		//tam thoi thay doi dia chi dich toi slave moi khoi tao
							PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
							HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);

							uint64_t datawwritetoflash[Maximum_number_of_IDs/8];
							uint8_array_to_uint64_array_big_endian(list_node_available, Maximum_number_of_IDs/8, datawwritetoflash);
							writeFlash(datawwritetoflash, Maximum_number_of_IDs/8, 127, FLASH_BANK_1);

							Master_Tx_SPDO_Data.state=state_temp;			//tra lai trang thai cu cho master
							CAN_Master_Tx_Data[0]=destinationaddress;		//tra lai dia chi dich cu cho master

							sprintf(serial_output_buffer,"Node moi co ID= 0x%02X ",Master_Tx_SPDO_Data.slaveID);
							Serial_Print();
							break;	// Phá vỡ vòng lặp while
						}
						i++;
						if(i==Maximum_number_of_IDs)
						{
							sprintf(serial_output_buffer,"Khong co ID phu hop cho node moi nay");
							Serial_Print();
						}
					}
				}
			}
		}

		if(Master_Rx_SPDO_Data.function==CHARGING_CONFIRM)	//slave confirm data setting
		{
			can_setting_confirm_flag++;
		}
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FDCAN2_Init();
  MX_UART4_Init();
  MX_TIM17_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  if(HAL_FDCAN_Start(&hfdcan2)!= HAL_OK)
	{
	  Error_Handler();
	}
  if (HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
	{
	  /* Notification Error */
	  Error_Handler();
	}

  HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &CAN_Master_Rx_Header, CAN_Master_Rx_Data);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);    // Bắt đầu PWM
  HAL_TIM_Base_Start_IT(&htim17);
  HAL_UART_Receive_DMA(&huart4, uart4_rx_data, sizeof(uart4_rx_data));
  HAL_UART_Receive_DMA(&huart3, uart3_rx_data, sizeof(uart3_rx_data));

  Master_Tx_SPDO_Data.state=CHARGING_ON;
  Master_Tx_SPDO_Data.type_of_value=CHARGING_DATA_CURRENT;


  CAN_Master_Tx_Header.Identifier = CHARGING_MASTER_ID;
  CAN_Master_Tx_Header.IdType = FDCAN_STANDARD_ID;
  CAN_Master_Tx_Header.TxFrameType = FDCAN_DATA_FRAME;
  CAN_Master_Tx_Header.DataLength = FDCAN_DLC_BYTES_8;
  CAN_Master_Tx_Header.FDFormat = FDCAN_CLASSIC_CAN;

  CAN_Master_Tx_Header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  CAN_Master_Tx_Header.BitRateSwitch = FDCAN_BRS_OFF;
  CAN_Master_Tx_Header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  CAN_Master_Tx_Header.MessageMarker = 0;

  CAN_Master_Tx_Data[0]=0x02;
  HAL_GPIO_WritePin(QCA_RS_GPIO_Port, QCA_RS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(CP_SELECT_GPIO_Port, CP_SELECT_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(PP_SELECT_GPIO_Port, PP_SELECT_Pin, GPIO_PIN_SET);


  //read list ID in flash
  uint64_t datareadfromflash[Maximum_number_of_IDs/8];
  readFlash(datareadfromflash, Maximum_number_of_IDs/8, 127, FLASH_BANK_1);
  uint64_array_to_uint8_array_big_endian(datareadfromflash, Maximum_number_of_IDs/8, list_node_available);

  list_node_available[0]=ID_IN_USED;
  list_node_available[1]=ID_IN_USED;
  for(int i=2;i<Maximum_number_of_IDs;i++)
  {
	  if(list_node_available[i]==0xFF) //reading error
		  list_node_available[i]=ID_NOT_USED;
  }

  //write list ID to flash
  uint64_t datawwritetoflash[Maximum_number_of_IDs/8];
  uint8_array_to_uint64_array_big_endian(list_node_available, Maximum_number_of_IDs/8, datawwritetoflash);
  writeFlash(datawwritetoflash, Maximum_number_of_IDs/8, 127, FLASH_BANK_1);

  for(int i=2;i<Maximum_number_of_IDs;i++) list_node_update[i]=0;		//the last time node communicated with CIMS, updated when node communicate
  for(int i=2;i<Maximum_number_of_IDs;i++) previoustick_slave[i]=0;		//time to check how long node had disconnected with CIMS, updated by timer

  for(int i=0;i<Maximum_number_of_Slave;i++)
	  Reset_Data(&Slave_Data[i]);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	uint16_t c=30;
	uint16_t v=600;
	uint16_t t=50;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		uint16_t c=30;
		uint16_t v=600;
		uint16_t t=50;
	TIM1->CCR1=50;
	uint32_t currenttick = HAL_GetTick(); // Lưu th�?i điểm bắt đầu



	if((currenttick-previoustick_calculate)>=TPDO_Event_Time)	//timer 100ms
	{
		previoustick_calculate=currenttick;
		for (int i = 0; i < can_node_number; ++i)
		{
//			c=c+ Slave_Data[i].current_value;
//			v=v+ Slave_Data[i].voltage_value;
//			t=t+ Slave_Data[i].temperature_value;
			c=c+ 15;
			v=v+ 300;
			t=t+ 25;
		}
		uint32_t period=currenttick-previoustick_calculate;
		//tinh toan cong suat va nang luong tieu thu
		Voltage=v;							//dien ap
		Current=c;							//dong dien
		Temperature=t+random_number()-random_number();						//nhiet do
		uint32_t p=c*v;						//cong suat
		Energy+=p*period;					//nang luong
		//set duty cho PWM
		uint32_t duty;
//		if(myEV.charging_current_request<51) duty=myEV.charging_current_request*5/3;
//		else if(myEV.charging_current_request<80) duty=(myEV.charging_current_request*2/5)+64;
//		TIM1->CCR1=duty;	//duty
		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 50);
	}

	//EV state timer 1s
	if((currenttick-previoustick_SPI)>=TPDO_Event_Time*10)
	{
		previoustick_SPI=currenttick;

		EVstate=PEF_Get_Sequence_State();
		switch(EVstate)
		{
		case INITIALIZATION + STATE_B + COMMUNICATION_INIT + REQUEST:
			break;

		case INITIALIZATION + STATE_B + COMMUNICATION_INIT + CONFIRM:
			break;

		case INITIALIZATION + STATE_B + PARAMETER_EXCHANGE + REQUEST:
			//HMI_Compose_Pre_Charge_Parm(HMI_COMPATIBLE, myEV.control_protocol_number); 		HMI_Print();
			HMI_Compose_Pre_Charge_Parm(HMI_CAR_MODEL, myEV.control_protocol_number); 		HMI_Print();
			HMI_Compose_Pre_Charge_Parm(HMI_BATTERY_CAPACITY, myEV.rate_capacity_battery);	HMI_Print();
			HMI_Compose_Pre_Charge_Parm(HMI_CURRENT_BATTERY, myEV.current_battery);			HMI_Print();
			HMI_Compose_Pre_Charge_Parm(HMI_BATTERY_VOLTAGE, myEV.max_battery_voltage);		HMI_Print();
			HMI_Compose_Pre_Charge_Parm(HMI_CHARGING_TIME, myEV.max_charging_time);			HMI_Print();
			break;

		case INITIALIZATION + STATE_B + PARAMETER_EXCHANGE + RESPONSE:
			break;

		case INITIALIZATION + STATE_B + PARAMETER_EXCHANGE + CONFIRM:
			break;

		case INITIALIZATION + STATE_B + EVSE_CONNECTOR_LOCK + REQUEST:
			//gui connected len HMI
			HMI_Compose_Status(HMI_CONNECT);	HMI_Print();
			break;

		case INITIALIZATION + STATE_B + EVSE_CONNECTOR_LOCK + CONFIRM:

			break;

		case ENERGY_TRANSFER + STATE_C + EV_CONTACTOR_CLOSE + REQUEST:

			break;

		case ENERGY_TRANSFER + STATE_C + EV_CONTACTOR_CLOSE + CONFIRM:
			//gui ready len HMI, vao trang theo doi realtime data
			CAN_Master_Tx_Data[0]=0x02;
			can_setting_confirm_flag=0;
			Master_Tx_SPDO_Data.state=CHARGING_ON;
			HMI_Compose_Status(HMI_READY);	HMI_Print();
			break;

		case ENERGY_TRANSFER + STATE_C + CHARGING_CURRENT_DEMAND + REQUEST:
			//gui cac cai dat qua CAN
			HMI_Compose_Pre_Charge_Parm(HMI_CURRENT_BATTERY, myEV.current_battery);
			HMI_Print();
			break;

		case ENERGY_TRANSFER + STATE_C + CHARGING_CURRENT_DEMAND + RESPONSE:

			break;

		case ENERGY_TRANSFER + STATE_C + CURRENT_SUPPRESSION + REQUEST:
			can_setting_confirm_flag=0;
			CAN_Master_Tx_Data[0]=0x02;
			Master_Tx_SPDO_Data.state=CHARGING_OFF;
			HMI_Compose_Status(HMI_STOP);	HMI_Print();
			break;

		case ENERGY_TRANSFER + STATE_C + CURRENT_SUPPRESSION + RESPONSE:

			break;

		case ENERGY_TRANSFER + STATE_C + CURRENT_SUPPRESSION + CONFIRM:

			break;

		case SHUT_DOWN + STATE_B + ZERO_CURRENT_CONFIRM + REQUEST:

			break;

		case SHUT_DOWN + STATE_B + ZERO_CURRENT_CONFIRM + CONFIRM:

			break;

		case SHUT_DOWN + STATE_B + VOLTAGE_VERIFICATION + REQUEST:

			break;

		case SHUT_DOWN + STATE_B + VOLTAGE_VERIFICATION + CONFIRM:

			break;

		case SHUT_DOWN + STATE_B + CONNECTOR_UNLOCK + REQUEST:

			break;

		case SHUT_DOWN + STATE_B + CONNECTOR_UNLOCK + CONFIRM:

			break;

		case SHUT_DOWN + STATE_B + END_OF_CHARGE + REQUEST:

			break;

		case SHUT_DOWN + STATE_B + END_OF_CHARGE + CONFIRM:
			//PEF_Handle_End_of_Charge_Cnf();
			uart_flag=0;
			button_flag=0;
			button_flag2=0;
			can_flag=0;
			can_setting_confirm_flag=0;
			Reset_Data(&Master_Tx_SPDO_Data);
			Master_Tx_SPDO_Data.state=CHARGING_ON;
			Master_Tx_SPDO_Data.type_of_value=CHARGING_DATA_CURRENT;
			CAN_Master_Tx_Data[0]=0x02;
			Reset_Data(&Master_Rx_SPDO_Data);
			break;
		}
	  }
	//end timer 1s

	//timer 100ms CAN BUS
	if((currenttick-previoustick_CAN)>=TPDO_Event_Time)
	{
		previoustick_CAN=currenttick;
		//lam viec voi cac trang thai EVSE
		//gui setting Data qua CAN Bus
		if(EVstate==(ENERGY_TRANSFER + STATE_C + CHARGING_CURRENT_DEMAND + REQUEST))	//setting theo EV, SPI CCC REQ
		{
			if(can_setting_confirm_flag<can_node_number*4)	//setting CAN
			{
				Master_Tx_SPDO_Data.function=CHARGING_SETTING;
				Master_Tx_SPDO_Data.current_value=myEV.charging_current_request/can_node_number;
				Master_Tx_SPDO_Data.voltage_value=600/can_node_number;
//				PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
//				HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);
				for(int i=0x02;i<(0x02+can_node_number);i++)
				{
					CAN_Master_Tx_Data[0]=i;
					Master_Tx_SPDO_Data.type_of_value=CHARGING_DATA_CURRENT;
					PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
					HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);
					HAL_Delay(100);
					Master_Tx_SPDO_Data.type_of_value=CHARGING_DATA_VOLTAGE;
					PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
					HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);
					HAL_Delay(100);
					Master_Tx_SPDO_Data.type_of_value=CHARGING_DATA_TEMPERATURE;
					PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
					HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);
					HAL_Delay(100);
					Master_Tx_SPDO_Data.type_of_value=CHARGING_DATA_TIME;
					PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
					HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);
					HAL_Delay(100);
				}
			}
			else		//setting done, request data
			{
				Master_Tx_SPDO_Data.function=CHARGING_REQUEST;
				changeTypeOfValue(&(Master_Tx_SPDO_Data.type_of_value));//change question to slave
				//change Destination ID from 0x02 to 0x07
				UpdateCANMasterTxDesID(0x02, can_node_number+1 , &Master_Tx_SPDO_Data, CAN_Master_Tx_Data);
				PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
				HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);
			}
		}

		if(EVstate==(ENERGY_TRANSFER + STATE_C + CURRENT_SUPPRESSION + CONFIRM))	//setting ve 0, current suppresion CNF
		{
			if(can_setting_confirm_flag<can_node_number*4)	//setting CAN
			{
				Master_Tx_SPDO_Data.function=CHARGING_SETTING;
				Master_Tx_SPDO_Data.current_value=0;
				Master_Tx_SPDO_Data.voltage_value=0;
//				PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
//				HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);
				for(int i=0x02;i<(0x02+can_node_number);i++)
				{
					CAN_Master_Tx_Data[0]=i;
					Master_Tx_SPDO_Data.type_of_value=CHARGING_DATA_CURRENT;
					PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
					HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);
					HAL_Delay(100);
					Master_Tx_SPDO_Data.type_of_value=CHARGING_DATA_VOLTAGE;
					PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
					HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);
					HAL_Delay(100);
					Master_Tx_SPDO_Data.type_of_value=CHARGING_DATA_TEMPERATURE;
					PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
					HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);
					HAL_Delay(100);
					Master_Tx_SPDO_Data.type_of_value=CHARGING_DATA_TIME;
					PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
					HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);
					HAL_Delay(100);
				}
			}
			else		//setting done, request data
			{
				Master_Tx_SPDO_Data.function=CHARGING_REQUEST;
				changeTypeOfValue(&(Master_Tx_SPDO_Data.type_of_value));//change question to slave
				//change Destination ID from 0x02 to 0x07
				UpdateCANMasterTxDesID(0x02, can_node_number+1 , &Master_Tx_SPDO_Data, CAN_Master_Tx_Data);
				//UpdateCANMasterTxDesID(0x02, 0x03, &Master_Tx_SPDO_Data, CAN_Master_Tx_Data);
				PDO_set_data_frame(CAN_Master_Tx_Data,Master_Tx_SPDO_Data);
				HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CAN_Master_Tx_Header, CAN_Master_Tx_Data);
			}
		}

		//kiem tra id mat ket noi
		for(int i=2;i<Maximum_number_of_IDs;i++)			//kiem tra cac node ID co mat ket noi khong
		{
			if(list_node_available[i]==ID_IN_USED)			//ID dang danh dau co ket noi
			{
				previoustick_slave[i]=HAL_GetTick();
				if(previoustick_slave[i]-list_node_update[i]>Maximum_time_not_update_of_IDs)	//NOT UPDATED for more than 5000ms =5s
				{
					list_node_available[i]=ID_DISCONNECTED;		//id da mat ket noi, cho ket noi lại
					Reset_Data(&Slave_Data[i-2]);				//reset data
					sprintf(serial_output_buffer,"Node ID= %d da mat ket noi ",i);
					Serial_Print();
					uint64_t datawwritetoflash[Maximum_number_of_IDs/8];
					uint8_array_to_uint64_array_big_endian(list_node_available, Maximum_number_of_IDs/8, datawwritetoflash);
					writeFlash(datawwritetoflash, Maximum_number_of_IDs/8, 127, FLASH_BANK_1);
				}
			}
		}

		//dem so luong node dang ket noi
			int count=0;
			for(int i=2;i<Maximum_number_of_IDs;i++)
			{
				if(list_node_available[i]==ID_IN_USED) count++;
			}
			can_node_number=count;
	}
	//end timer 100ms cho CAN


	//gui cac thong so dong dien, dien ap len HMI
	//timer 500ms cho HMI. gui realtime data

	if((currenttick-previoustick_HMI)>=TPDO_Event_Time*5)	//timer 500ms cho HMI. gui realtime data
	{
		previoustick_HMI=currenttick;
		if (
				(EVstate==(ENERGY_TRANSFER + STATE_C + CHARGING_CURRENT_DEMAND + REQUEST)) ||
				(EVstate==(ENERGY_TRANSFER + STATE_C + CURRENT_SUPPRESSION + REQUEST) )	 ||
				(EVstate==(ENERGY_TRANSFER + STATE_C + CURRENT_SUPPRESSION + CONFIRM))
			)	//SPI CCC REQ, SPI Current suppression REQ, SPI Current suppression CNF
		{
			int check=0;	//kiem tra xem co node nao loi khong
			for(int i=0;i<can_node_number;i++)
			{
				if(Slave_Data[i].state!=CHARGING_ON)
				{
					check++;
				}
			}

			if(check==0)	//khong co cai nao bi loi
			{
					HMI_Compose_Realtime_Data(HMI_VOLTAGE, Voltage);	HMI_Print();
					HMI_Compose_Realtime_Data(HMI_CURRENT, Current);	HMI_Print();
					HMI_Compose_Realtime_Data(HMI_TEMP, Temperature);	HMI_Print();
					HMI_Compose_Realtime_Data(HMI_IREF, myEV.charging_current_request); HMI_Print();
			}
			//co it nhat 1 cai bi loi send error, arlam to HMI
			for(int i=0;i<can_node_number;i++)
			{
				if(Slave_Data[i].state!=CHARGING_ON && can_setting_confirm_flag>=can_node_number*4)
				{
					HMI_Compose_Status(HMI_ERROR_1);	HMI_Print();
				}
			}

		}
	}
	if((currenttick-previoustick_ESP)>=TPDO_Event_Time*200)	//timer 20s cho ESP. gui realtime data
	{
		previoustick_ESP=currenttick;
		if (
				(EVstate==(ENERGY_TRANSFER + STATE_C + CHARGING_CURRENT_DEMAND + REQUEST)) ||
				(EVstate==(ENERGY_TRANSFER + STATE_C + CURRENT_SUPPRESSION + REQUEST) )	 ||
				(EVstate==(ENERGY_TRANSFER + STATE_C + CURRENT_SUPPRESSION + CONFIRM))
			)	//SPI CCC REQ, SPI Current suppression REQ, SPI Current suppression CNF
		{
			int check=0;	//kiem tra xem co node nao loi khong
			for(int i=0;i<can_node_number;i++)
			{
				if(Slave_Data[i].state!=CHARGING_ON)
				{
					check++;
				}
			}

			if(check==0)	//khong co cai nao bi loi
			{
						ESP_Data.ESP_Data_voltage=Voltage;
						ESP_Data.ESP_Data_current=Current;
						Packet_to_ESP(TYPE_CURRENT_VALUE,STATE_ON);
						Packet_to_ESP(TYPE_VOLTAGE_VALUE,STATE_ON);
			}
			//co it nhat 1 cai bi loi send error, arlam to HMI
			for(int i=0;i<can_node_number;i++)
			{
				if(Slave_Data[i].state!=CHARGING_ON && can_setting_confirm_flag>=can_node_number*4)
				{
					HMI_Compose_Status(HMI_ERROR_1);	HMI_Print();
				}
			}

		}
	}
}


  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV4;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief FDCAN2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN2_Init(void)
{

  /* USER CODE BEGIN FDCAN2_Init 0 */

  /* USER CODE END FDCAN2_Init 0 */

  /* USER CODE BEGIN FDCAN2_Init 1 */

  /* USER CODE END FDCAN2_Init 1 */
  hfdcan2.Instance = FDCAN2;
  hfdcan2.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan2.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan2.Init.AutoRetransmission = ENABLE;
  hfdcan2.Init.TransmitPause = DISABLE;
  hfdcan2.Init.ProtocolException = DISABLE;
  hfdcan2.Init.NominalPrescaler = 16;
  hfdcan2.Init.NominalSyncJumpWidth = 1;
  hfdcan2.Init.NominalTimeSeg1 = 4;
  hfdcan2.Init.NominalTimeSeg2 = 3;
  hfdcan2.Init.DataPrescaler = 1;
  hfdcan2.Init.DataSyncJumpWidth = 1;
  hfdcan2.Init.DataTimeSeg1 = 1;
  hfdcan2.Init.DataTimeSeg2 = 1;
  hfdcan2.Init.StdFiltersNbr = 0;
  hfdcan2.Init.ExtFiltersNbr = 0;
  hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN2_Init 2 */

  /* USER CODE END FDCAN2_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 8000;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 99;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM17 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM17_Init(void)
{

  /* USER CODE BEGIN TIM17_Init 0 */

  /* USER CODE END TIM17_Init 0 */

  /* USER CODE BEGIN TIM17_Init 1 */

  /* USER CODE END TIM17_Init 1 */
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 319;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 9999;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM17_Init 2 */

  /* USER CODE END TIM17_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, PP_SELECT_Pin|CP_SELECT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, QCA_RS_Pin|SPI2_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BUTTON_INT_Pin */
  GPIO_InitStruct.Pin = BUTTON_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTTON_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PP_SELECT_Pin CP_SELECT_Pin */
  GPIO_InitStruct.Pin = PP_SELECT_Pin|CP_SELECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : LED1_Pin */
  GPIO_InitStruct.Pin = LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : QCA_INT_Pin */
  GPIO_InitStruct.Pin = QCA_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(QCA_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : QCA_RS_Pin SPI2_CS_Pin */
  GPIO_InitStruct.Pin = QCA_RS_Pin|SPI2_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
