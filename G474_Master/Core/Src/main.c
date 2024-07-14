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
uint8_t buffer[UART_SIZE] = { 0 };
uint8_t size = 0;
uint8_t flag = 0;

uint32_t previoustick_master = 0;
uint32_t previoustick_slave[Maximum_number_of_IDs];
uint32_t previoustick_HMI = 0;
uint32_t previoustick_ESP = 0;
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
uint32_t list_node_update[Maximum_number_of_IDs];//The list saves the time that the IDs were last updated

int uart_flag = 0;
int button_flag = 0;
int button_flag2 = 0;
int can_flag = 0;

int can_setting_confirm_flag = 0;//so luong thong so da cai dat xong, 1 node co 4 thong so
int can_node_number = 2;//so node dang tham gia mang, se cap nhat trong timer

uint16_t EVstate;
uint16_t Current = 30;
uint16_t Voltage = 0;
uint16_t Temperature = 50;
uint16_t Energy = 0;
// Simulation
#define MAX_CONNECTORS 2

int connectors[MAX_CONNECTORS] = { 0, 0 };
int users[MAX_CONNECTORS] = { 255, 255 };
uint8_t current_user = 255;
int status[MAX_CONNECTORS] = { 0, 0 }; // Connector charging or not
uint8_t connectorID;

void Serial_Print(void) {
#define TIMEOUT_100_MS 100
	HAL_UART_Transmit(&huart1, (uint8_t*) serial_output_buffer,
			strlen(serial_output_buffer), TIMEOUT_100_MS);
}

void EVSE_state() {
	sprintf(serial_output_buffer, "Connector 1: %X, Connector 2: %X\n",
			connectors[0], connectors[1]);
	Serial_Print();
	sprintf(serial_output_buffer, "Status 1: %X, Status 2: %X\n", status[0],
			status[1]);
	Serial_Print();
	sprintf(serial_output_buffer, "User 1: %X, User 2: %X\n", users[0], users[1]);
	Serial_Print();
	sprintf(serial_output_buffer, "curent_User: %X\n", current_user);
	Serial_Print();
}

void ESP_Send(void) {
	HAL_UART_Transmit(&huart3, ESP_Payload, 8, 100);
}

void SPI_Transmit_Receive(void) {
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi2, spi_tx_buffer, spi_rx_buffer, spi_data_size,
			200);
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
}

void HMI_Print(void) {
	HAL_UART_Transmit(&huart4, (uint8_t*) hmi_buffer, strlen(hmi_buffer),
	TIMEOUT_100_MS);
	uint8_t END_BYTE = 0xFF;
	HAL_UART_Transmit(&huart4, &END_BYTE, 1, 10);
	HAL_UART_Transmit(&huart4, &END_BYTE, 1, 10);

HAL_UART_Transmit(&huart4, &END_BYTE, 1, 10);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	;
}
uint8_t calculate_checksum(uint8_t *frame, size_t length) {
	uint8_t sum = 0;
	for (size_t i = 0; i < length - 1; i++) {
		sum += frame[i];
	}
	return sum & 0xFF;
}
void create_frame(uint8_t file_code, uint8_t *data, uint8_t *frame,
		uint8_t connector) {
	frame[0] = 0xAB;
	frame[1] = 0xCD;
	frame[2] = file_code;
	memcpy(&frame[3], data, 3);
	frame[6] = connector;
	frame[7] = calculate_checksum(frame, 8);
}
void send_frames(UART_HandleTypeDef *huart, uint8_t file_code, uint8_t *data,
		uint8_t connector) {
	uint8_t frame[8];
	create_frame(file_code, data, frame, connector);
	HAL_UART_Transmit(huart, frame, 8, HAL_MAX_DELAY);
	printf("Sent: ");
	for (int i = 0; i < 8; i++) {
		printf("%02X ", frame[i]);
	}
	printf("\n");

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	UNUSED(huart);
	if (huart->Instance == huart3.Instance)	//ESP data
			{
		HAL_UART_Receive_DMA(&huart3, uart3_rx_data, sizeof(uart3_rx_data));
		connectorID = uart3_rx_data[5];
		if (uart3_rx_data[2] == 0x15) {
			EVSE_state();
			if (connectorID == 255 || status[uart3_rx_data[5] - 1] == 0) {
				current_user = uart3_rx_data[4];

				uint8_t data_to_hmi[9] = { 0x70, 0x61, 0x67, 0x65, 0x20, 0x31,
						0xFF, 0xFF, 0xFF };
				uint8_t status_hmi = HAL_UART_Transmit(&huart4, data_to_hmi, 9, 100);
				sprintf(serial_output_buffer,
						"Status HMI: %02X",status_hmi);
				Serial_Print();
				HAL_UART_Transmit(&huart4, data_to_hmi, 9, 100);
				EVSE_state();
			} else if (status[connectorID - 1] == 1) { //swipe card to stop
				uint8_t data[] = { 0x00, 0x00, uart3_rx_data[5] };
				send_frames(&huart3, 0x18, data, 0x00); // request stop to server
				current_user = uart3_rx_data[4];
				// users: số thứ tự connector ; current_user: đại diện cho ID người đến
				if(users[current_user]==2)
				{
					HMI_Compose_Realtime_Data(HMI_TRAN_STATUS2, 0);
					HMI_Print();
				}
				else if(users[current_user]==1)
				{
					HMI_Compose_Realtime_Data(HMI_TRAN_STATUS1, 0);
					HMI_Print();
				}


				}

			}

		 else if (uart3_rx_data[2] == 0x1B) { // confirm from server
			if (uart3_rx_data[3]) {
				status[uart3_rx_data[5] - 1] = 1;
				//send to EV that EVSE is prepare charge
				PEF_Compose_Contactor_Close_Req();
				SPI_QCA7000_Send_Eth_Frame();
				sprintf(serial_output_buffer,
						"Send PEF_Compose_Contactor_Close_Req() ");
				Serial_Print();
				//send logout to ESP
				uint8_t data[] = { 0x00, current_user, users[current_user] };
				send_frames(&huart3, 0x15, data, 0x00);
				current_user = 255;
				EVSE_state();
			} else { // confirm stop transaction
				status[uart3_rx_data[5] - 1] = 0;
				uint8_t data[] = { 0x00, 0x00, users[current_user] };
				send_frames(&huart3, 0x17, data, 0x00); // unplugged connector

				connectors[users[current_user] - 1] = 0;

				data[1] = current_user;
				send_frames(&huart3, 0x15, data, 0x00); // logout
				current_user = 255;

				//send to HMI change to begin UI
				uint8_t data_to_hmi[9] = { 0x70, 0x61, 0x67, 0x65, 0x20, 0x30,
						0xFF, 0xFF, 0xFF };
				HAL_UART_Transmit(&huart4, data_to_hmi, 9, 100);
				EVSE_state();
				PEF_Reset_RXBUFFER();
			}
		}
	    memset(uart3_rx_data, 0, sizeof(uart3_rx_data));
			}
	if (huart->Instance == huart4.Instance)	//HMI data
			{


		uint8_t uart4_status=HAL_UART_Receive_DMA(&huart4, uart4_rx_data, sizeof(uart4_rx_data));
		sprintf(serial_output_buffer, "HMI status: %02X",uart4_status,uart4_rx_data[0]);
		Serial_Print();
		if (uart4_rx_data[0] > 0x29 && uart4_rx_data[0] < 0x60) {
			uart_flag = 1;
			if (uart4_rx_data[0] == 0x40) {
				//start
				sprintf(serial_output_buffer, "HMI gui goi tin Start ");
				Serial_Print();
				uint8_t data_start[] = { 0x00, 0x00, users[current_user] };
				send_frames(&huart3, TYPE_HMI_CONTROL_TRANSACTION, data_start,
						0x01); // request start to server
				ESP_Data.ESP_Data_voltage = 0;
				ESP_Data.ESP_Data_current = 0;
				Packet_to_ESP(0x20, users[current_user]);
				Packet_to_ESP(0x21, users[current_user]);

				Master_Tx_SPDO_Data.state = CHARGING_ON;
				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
			}
			else {
				//data
				//Master_Tx_SPDO_Data.state=CHARGING_ON;
				sprintf(serial_output_buffer, "HMI gui goi tin Setting ");
				Serial_Print();
				HMI_Evaluate_Setting_Data(uart4_rx_data);
			}

		memset(uart4_rx_data, 0, sizeof(uart4_rx_data));
	}
}

}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_13) {
	}

	if (GPIO_Pin == QCA_INT_Pin) {
		SPI_QCA7000_Handling_Intr();
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
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_GPIO_WritePin(QCA_RS_GPIO_Port, QCA_RS_Pin, GPIO_PIN_SET);
	//  HAL_GPIO_WritePin(PP_SELECT_GPIO_Port, PP_SELECT_Pin, GPIO_PIN_SET);
	HAL_UART_Receive_DMA(&huart4, uart4_rx_data, sizeof(uart4_rx_data));
	HAL_UART_Receive_DMA(&huart3, uart3_rx_data, sizeof(uart3_rx_data));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		uint8_t hour, hour2;
		uint8_t min, min2;
		uint16_t soc, soc2;
		uint16_t init_soc, init_soc2;
		uint16_t rate_soc, rate_soc2;
		uint16_t soh, soh2;
		TIM1->CCR1 = 50;
		uint32_t currenttick = HAL_GetTick(); // Lưu th�?i điểm bắt đầu


		if ((currenttick - previoustick_calculate) >= TPDO_Event_Time) //timer 100ms
		{
			previoustick_calculate = currenttick;
//		uint32_t period=currenttick-previoustick_calculate;
			//set duty cho PWM
//		uint32_t duty;
//		if(myEV.charging_current_request<51) duty=myEV.charging_current_request*5/3;
//		else if(myEV.charging_current_request<80) duty=(myEV.charging_current_request*2/5)+64;
//		TIM1->CCR1=duty;	//duty
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 50);
		}

		//EV state timer 1s
		if ((currenttick - previoustick_SPI) >= TPDO_Event_Time * 10) {
			previoustick_SPI = currenttick;

			EVstate = PEF_Get_Sequence_State();
			switch (EVstate) {
			case INITIALIZATION + STATE_B + COMMUNICATION_INIT + REQUEST:
				break;

			case INITIALIZATION + STATE_B + COMMUNICATION_INIT + CONFIRM:
				break;

			case INITIALIZATION + STATE_B + PARAMETER_EXCHANGE + REQUEST: //
				break;

			case INITIALIZATION + STATE_B + PARAMETER_EXCHANGE + RESPONSE:
				break;

			case INITIALIZATION + STATE_B + PARAMETER_EXCHANGE + CONFIRM:
				break;

			case INITIALIZATION + STATE_B + EVSE_CONNECTOR_LOCK + REQUEST: //send connector plugged or not
				//gui connected len HMI
				//simulation that first user always plugged connector 2
				if (current_user == 0) {
					users[current_user] = 2;
				} else if (current_user == 1) {
					users[current_user] = 1;
				}
				if(users[current_user]==1)
				{
					sprintf(serial_output_buffer, "Hello 1");
					Serial_Print();
					HMI_Compose_Status(HMI_CONNECT);
					HMI_Print();
					uint8_t data_connect[] = { 0x00, 0x00, users[current_user] };
					send_frames(&huart3, TYPE_CONNECTER_STATUS, data_connect, 0x01);
					HAL_GPIO_WritePin(CP_SELECT_GPIO_Port, CP_SELECT_Pin,
							GPIO_PIN_SET);


					soc = myEV.current_battery;
					init_soc = 105+rand()%5;
					soh = 100;
					rate_soc =(uint16_t) soc * 100 / soh;
					hour = (uint8_t) ((100 - rate_soc)/18);
					min = ((100 - rate_soc)*60/18) - hour * 60;
					HMI_Compose_Pre_Charge_Parm(HMI_InitSoC, init_soc);
					HMI_Print();
					HMI_Compose_Pre_Charge_Parm(HMI_SoH, (uint16_t)100*soh/init_soc);
					HMI_Print();
					HMI_Compose_Pre_Charge_Parm(HMI_SoC, soc);
					HMI_Print();
					HMI_Compose_Pre_Charge_Parm(HMI_CHARGING_TIME_HOUR, hour);
					HMI_Print();
					HMI_Compose_Pre_Charge_Parm(HMI_CHARGING_TIME_MIN, min);
					HMI_Print();

					//send data init
					uint8_t data_soh[] = { 0x00, (uint8_t) (soh & 0xFF),
							(uint8_t) ((soh >> 8) & 0xFF) };
					send_frames(&huart3, TYPE_SOH, data_soh, users[current_user]);
					uint8_t data_init_soc[] = { 0x00, (uint8_t) (init_soc & 0xFF),
							(uint8_t) ((init_soc >> 8) & 0xFF) };
					send_frames(&huart3, TYPE_INIT_SOC, data_init_soc,
							users[current_user]);
					uint8_t data_soc[] = { 0x00, (uint8_t) (soc & 0xFF),
							(uint8_t) ((soc >> 8) & 0xFF) };
					send_frames(&huart3, TYPE_SOC, data_soc, users[current_user]);
				}
				if(users[current_user]==2)
				{
					sprintf(serial_output_buffer, "Hello 0");
					Serial_Print();
					HMI_Compose_Status(HMI_CONNECT2);
					HMI_Print();
					uint8_t data_connect[] = { 0x00, 0x00, users[current_user] };
					send_frames(&huart3, TYPE_CONNECTER_STATUS, data_connect, 0x01);
					HAL_GPIO_WritePin(CP_SELECT_GPIO_Port, CP_SELECT_Pin,
							GPIO_PIN_SET);

					soc2 = myEV.current_battery+rand()%5;
					init_soc2 = 105+rand()%5;
					soh2 = 100;
					rate_soc2 = (uint16_t)soc2 * 100 / soh2;
					hour2 = (uint8_t) ((100 - rate_soc2)/18);
					min2 = ((100 - rate_soc2)*60/18) - hour2 * 60;
					HMI_Compose_Pre_Charge_Parm(HMI_InitSoC2, 100);
					HMI_Print();
					HMI_Compose_Pre_Charge_Parm(HMI_SoH2, (uint16_t)100*soh2/init_soc2);
					HMI_Print();
					HMI_Compose_Pre_Charge_Parm(HMI_SoC2, soc2);
					HMI_Print();
					HMI_Compose_Pre_Charge_Parm(HMI_CHARGING_TIME_HOUR2, hour2);
					HMI_Print();
					HMI_Compose_Pre_Charge_Parm(HMI_CHARGING_TIME_MIN2, min2);
					HMI_Print();

					//send data init
					uint8_t data_soh2[] = { 0x00, (uint8_t) (soh2 & 0xFF),
							(uint8_t) ((soh2 >> 8) & 0xFF) };
					send_frames(&huart3, TYPE_SOH, data_soh2, users[current_user]);
					uint8_t data_init_soc2[] = { 0x00, (uint8_t) (init_soc2 & 0xFF),
							(uint8_t) ((init_soc2 >> 8) & 0xFF) };
					send_frames(&huart3, TYPE_INIT_SOC, data_init_soc2,
							users[current_user]);
					uint8_t data_soc2[] = { 0x00, (uint8_t) (soc2 & 0xFF),
							(uint8_t) ((soc2 >> 8) & 0xFF) };
					send_frames(&huart3, TYPE_SOC, data_soc2, users[current_user]);
				}
				EVSE_state();
				PEF_Reset_RXBUFFER();//reset EVState - EV buffer
				break;

			case INITIALIZATION + STATE_B + EVSE_CONNECTOR_LOCK + CONFIRM:

				break;

			case ENERGY_TRANSFER + STATE_C + EV_CONTACTOR_CLOSE + REQUEST:

				break;

			case ENERGY_TRANSFER + STATE_C + EV_CONTACTOR_CLOSE + CONFIRM:
				//gui ready len HMI, vao trang theo doi realtime data
				CAN_Master_Tx_Data[0] = 0x02;
				can_setting_confirm_flag = 0;
				Master_Tx_SPDO_Data.state = CHARGING_ON;
				HMI_Compose_Status(HMI_READY);
				HMI_Print();

				break;

			case ENERGY_TRANSFER + STATE_C + CHARGING_CURRENT_DEMAND + REQUEST:
				//gui cac cai dat qua CAN
				//		HMI_Compose_Pre_Charge_Parm(HMI_CURRENT_BATTERY, myEV.current_battery);
				HMI_Print();
				break;

			case ENERGY_TRANSFER + STATE_C + CHARGING_CURRENT_DEMAND + RESPONSE:

				break;

			case ENERGY_TRANSFER + STATE_C + CURRENT_SUPPRESSION + REQUEST:
				can_setting_confirm_flag = 0;
				CAN_Master_Tx_Data[0] = 0x02;
				Master_Tx_SPDO_Data.state = CHARGING_OFF;
				HMI_Compose_Status(HMI_STOP);
				HMI_Print();

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
				uart_flag = 0;
				break;
			}
		}

		if ((currenttick - previoustick_HMI) >= TPDO_Event_Time * 20)//timer 500ms cho HMI. gui realtime data
				{
			previoustick_HMI = currenttick;
			if (status[0] == 1) { //connector 2
				if ((EVstate
						== (ENERGY_TRANSFER + STATE_C + CHARGING_CURRENT_DEMAND
								+ REQUEST))
						|| (EVstate
								== (ENERGY_TRANSFER + STATE_C
										+ CURRENT_SUPPRESSION + REQUEST))
						|| (EVstate
								== (ENERGY_TRANSFER + STATE_C
										+ CURRENT_SUPPRESSION + CONFIRM)))//SPI CCC REQ, SPI Current suppression REQ, SPI Current suppression CNF
						{
					uint8_t temp = 50 + (rand() % 2) - (rand() % 2);
					HAL_GPIO_WritePin(EV_Charge_GPIO_Port, EV_Charge_Pin, SET);
					HMI_Compose_Pre_Charge_Parm(HMI_CHARGING_TIME_HOUR, hour);
					HMI_Print();
					HMI_Compose_Pre_Charge_Parm(HMI_CHARGING_TIME_MIN, min);
					HMI_Print();
					HMI_Compose_Realtime_Data(HMI_VOLTAGE, 600);
					HMI_Print();
					HMI_Compose_Realtime_Data(HMI_CURRENT, 30);
					HMI_Print();
					HMI_Compose_Realtime_Data(HMI_TEMP, 50);
					HMI_Print();
					HMI_Compose_Realtime_Data(HMI_SoC, rate_soc);
					HMI_Print();
					ESP_Data.ESP_Data_voltage = 600;
					ESP_Data.ESP_Data_current = 30;
					sprintf(serial_output_buffer, "SOC1: %u and %u",rate_soc,soc);
					Serial_Print();
					Packet_to_ESP(TYPE_VOLTAGE_VALUE, 2);
					Packet_to_ESP(TYPE_CURRENT_VALUE, 2);
					uint8_t data_sot_soc[] = { temp, (uint8_t) (rate_soc & 0xFF),
							(uint8_t) ((rate_soc >> 8) & 0xFF) };
					send_frames(&huart3, TYPE_SOT_SOC, data_sot_soc,
							2);
					rate_soc = rate_soc + 1;
					min = min - ((100 - rate_soc)*60/18)/(100-rate_soc);
					if (min <=0 && hour > 0) {
						min = 60;
						hour = hour - 1;
					}
					if (min > 0 && hour == 0) {
						min = min;
					}
					if (rate_soc >= 100) {
						rate_soc = 100;
						min = 0;
						hour = 0;
						HMI_Compose_Realtime_Data(HMI_TRAN_STATUS1, 0);
						HMI_Print();
						uint8_t stop_to_hmi[9] = { 0x70, 0x61, 0x67, 0x65, 0x20,
								0x30, 0xFF, 0xFF, 0xFF };
						HAL_UART_Transmit(&huart4, stop_to_hmi, 9, 100);
						HAL_GPIO_WritePin(EV_Charge_GPIO_Port, EV_Charge_Pin,
								GPIO_PIN_RESET);
						HAL_GPIO_WritePin(CP_SELECT_GPIO_Port, CP_SELECT_Pin,
								GPIO_PIN_RESET);
						uint8_t data_stop[] = { 0x00, 0x00, 2 };
						send_frames(&huart3, TYPE_HMI_CONTROL_TRANSACTION,
								data_stop, 0x00);
						PEF_Reset_RXBUFFER();

					}
				}
			}
			if (status[1] == 1) {
				if ((EVstate
						== (ENERGY_TRANSFER + STATE_C + CHARGING_CURRENT_DEMAND
								+ REQUEST))
						|| (EVstate
								== (ENERGY_TRANSFER + STATE_C
										+ CURRENT_SUPPRESSION + REQUEST))
						|| (EVstate
								== (ENERGY_TRANSFER + STATE_C
										+ CURRENT_SUPPRESSION + CONFIRM)))//SPI CCC REQ, SPI Current suppression REQ, SPI Current suppression CNF
						{
					uint8_t temp2 = 50 + (rand() % 2) - (rand() % 2);
					HAL_GPIO_WritePin(EV_Charge_GPIO_Port, EV_Charge_Pin, SET);
					HMI_Compose_Pre_Charge_Parm(HMI_CHARGING_TIME_HOUR2, hour2);
					HMI_Print();
					HMI_Compose_Pre_Charge_Parm(HMI_CHARGING_TIME_MIN2, min2);
					HMI_Print();
					HMI_Compose_Realtime_Data(HMI_VOLTAGE2, 600);
					HMI_Print();
					HMI_Compose_Realtime_Data(HMI_CURRENT2, 30);
					HMI_Print();
					HMI_Compose_Realtime_Data(HMI_TEMP2, temp2);
					HMI_Print();
					HMI_Compose_Pre_Charge_Parm(HMI_SoC2, rate_soc2);
					HMI_Print();
					ESP_Data.ESP_Data_voltage = 600;
					ESP_Data.ESP_Data_current = 30;
					sprintf(serial_output_buffer, "SOC0: %u and %u",rate_soc2,soc2);
					Serial_Print();
					Packet_to_ESP(TYPE_VOLTAGE_VALUE, 2);
					Packet_to_ESP(TYPE_CURRENT_VALUE, 2);
					uint8_t data_sot_soc2[] = { temp2, (uint8_t) (rate_soc2 & 0xFF),
							(uint8_t) ((rate_soc2 >> 8) & 0xFF) };
					send_frames(&huart3, TYPE_SOT_SOC, data_sot_soc2,
							2);
					rate_soc2 = rate_soc2 + 1;
					hour2 = (uint8_t) ((100 - rate_soc2)/18);
					min2 = ((100 - rate_soc2)*60/18) - hour2 * 60;
					if (rate_soc2 > 100) {
						rate_soc2 = 100;
						min2 = 0;
						hour2 = 0;
						HMI_Compose_Realtime_Data(HMI_TRAN_STATUS2, 0);
						HMI_Print();
						uint8_t stop_to_hmi[9] = { 0x70, 0x61, 0x67, 0x65, 0x20,
								0x30, 0xFF, 0xFF, 0xFF };
						HAL_UART_Transmit(&huart4, stop_to_hmi, 9, 100);
						HAL_GPIO_WritePin(EV_Charge_GPIO_Port, EV_Charge_Pin,
								GPIO_PIN_RESET);
						HAL_GPIO_WritePin(CP_SELECT_GPIO_Port, CP_SELECT_Pin,
								GPIO_PIN_RESET);
						uint8_t data_stop[] = { 0x00, 0x00, 2 };
						send_frames(&huart3, TYPE_HMI_CONTROL_TRANSACTION,
								data_stop, 0x00);
						PEF_Reset_RXBUFFER();

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
  htim1.Init.Prescaler = 159;
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
  HAL_GPIO_WritePin(GPIOC, PP_SELECT_Pin|CP_SELECT_Pin|EV_Charge_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, QCA_RS_Pin|SPI2_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BUTTON_INT_Pin */
  GPIO_InitStruct.Pin = BUTTON_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTTON_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PP_SELECT_Pin CP_SELECT_Pin EV_Charge_Pin */
  GPIO_InitStruct.Pin = PP_SELECT_Pin|CP_SELECT_Pin|EV_Charge_Pin;
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
	while (1) {
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
