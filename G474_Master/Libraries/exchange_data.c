/*
 * exchange_data.c
 *
 *  Created on: Dec 16, 2023
 *      Author: PC
 */

#include "cims.h"

ev_exchange ev1;
evse_exchange evse1;

uint8_t data_exchange[EXCHANGE_DATA_LEN];
uint8_t exchange_data_size;

void Clean_Data_Exchange(void) {
  /* fill the complete ethernet transmit buffer with 0x00 */
  int i;
  for (i=0; i<EXCHANGE_DATA_LEN; i++) {
    data_exchange[i]=0;
  }
}

void EV_Compose_Start_Communication_Req(void){
	ev1.ev_model = 0xAA;
	ev1.battery_capacity = 0x1E;
	ev1.current_request = 0x0014;
	ev1.voltage_request = 0x00C8;
	ev1.max_voltage_limit = 0x00DC;
	ev1.min_current_limit = 0x000A;
	ev1.max_charging_time = 0x0258;

	Clean_Data_Exchange();

	exchange_data_size = 20;

    data_exchange[0]=0x88; // Protocol HomeplugAV
    data_exchange[1]=0xE1; //
    data_exchange[2]=0x01; // version
    data_exchange[3]=0xE0; // Start_Communication.REQ
    data_exchange[4]=0x60; //
    data_exchange[5]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    data_exchange[6]=0x00; //
    data_exchange[7]=ev1.ev_model; //
    data_exchange[8]=(ev1.battery_capacity >> 8);
    data_exchange[9]=ev1.battery_capacity;
    data_exchange[10]=(ev1.current_request >> 8);
    data_exchange[11]=ev1.current_request;
    data_exchange[12]=(ev1.voltage_request >> 8);
    data_exchange[13]=ev1.voltage_request;
    data_exchange[14]=(ev1.max_voltage_limit >> 8);
    data_exchange[15]=ev1.max_voltage_limit;
    data_exchange[16]=(ev1.min_current_limit >> 8);
    data_exchange[17]=ev1.min_current_limit;
    data_exchange[18]=(ev1.max_charging_time >> 8);
    data_exchange[19]=ev1.max_charging_time;
}

void EV_Evaluate_Start_Communication_Req(void){
	ev1.ev_model = eth_rx_buffer[19];
	ev1.battery_capacity = (eth_rx_buffer[20] << 8) + eth_rx_buffer[21];
	ev1.current_request = (eth_rx_buffer[22] << 8) + eth_rx_buffer[23];
	ev1.voltage_request = (eth_rx_buffer[24] << 8) + eth_rx_buffer[25];
	ev1.max_voltage_limit = (eth_rx_buffer[26] << 8) + eth_rx_buffer[27];
	ev1.min_current_limit = (eth_rx_buffer[28] << 8) + eth_rx_buffer[29];
	ev1.max_charging_time = (eth_rx_buffer[30] << 8) + eth_rx_buffer[31];

//	sprintf(serial_output_buffer, "model: %x, current_request: %x, voltage_request: %x ", ev1.ev_model, ev1.current_request, ev1.voltage_request );
//	Serial_Print();
}

void EV_Compose_Start_Communication_Cnf(void){
	evse1.max_voltage_rate = 0x00DC;
	evse1.max_current_rate = 0x0014;
	evse1.charging_time = 0x0078;

	Clean_Data_Exchange();

	exchange_data_size = 13;

	data_exchange[0]=0x88; // Protocol HomeplugAV
    data_exchange[1]=0xE1; //
    data_exchange[2]=0x01; // version
    data_exchange[3]=0xE1; // Start_Communication.CNF
    data_exchange[4]=0x60; //
    data_exchange[5]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    data_exchange[6]=0x00; //
    data_exchange[7]=(evse1.max_voltage_rate >> 8);
    data_exchange[8]=evse1.max_voltage_rate;
    data_exchange[9]=(evse1.max_current_rate >> 8);
    data_exchange[10]=evse1.max_current_rate;
    data_exchange[11]=(evse1.charging_time >> 8);
    data_exchange[12]=evse1.charging_time;
}

void EV_Evaluate_Start_Communication_Cnf(void){
	evse1.max_voltage_rate = (eth_rx_buffer[19] << 8) + eth_rx_buffer[20];
	evse1.max_current_rate = (eth_rx_buffer[21] << 8) + eth_rx_buffer[22];
	evse1.charging_time = (eth_rx_buffer[23] << 8) + eth_rx_buffer[24];

//	sprintf(serial_output_buffer, "Vmax: %u, Imax: %u, Charging Time: %u ", evse1.max_voltage_rate, evse1.max_current_rate, evse1.charging_time );
//	Serial_Print();
}

void EV_Compose_Setup_Done_Req()
{
	Clean_Data_Exchange();

	exchange_data_size = 7;

	data_exchange[0]=0x88; // Protocol HomeplugAV
    data_exchange[1]=0xE1; //
    data_exchange[2]=0x01; // version
    data_exchange[3]=0xE4; // Start_Communication.CNF
    data_exchange[4]=0x60; //
    data_exchange[5]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    data_exchange[6]=0x00; //
}

void EV_Compose_Setup_Done_Cnf()
{
	Clean_Data_Exchange();

	exchange_data_size = 7;

	data_exchange[0]=0x88; // Protocol HomeplugAV
    data_exchange[1]=0xE1; //
    data_exchange[2]=0x01; // version
    data_exchange[3]=0xE5; // Start_Communication.CNF
    data_exchange[4]=0x60; //
    data_exchange[5]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    data_exchange[6]=0x00; //
}

void EV_Compose_Start_Charging_Req()
{
	Clean_Data_Exchange();

	exchange_data_size = 7;

	data_exchange[0]=0x88; // Protocol HomeplugAV
    data_exchange[1]=0xE1; //
    data_exchange[2]=0x01; // version
    data_exchange[3]=0xE8; // Start_Communication.CNF
    data_exchange[4]=0x60; //
    data_exchange[5]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    data_exchange[6]=0x00; //
}

void EV_Compose_Start_Charging_Cnf()
{
	Clean_Data_Exchange();

	exchange_data_size = 7;

	data_exchange[0]=0x88; // Protocol HomeplugAV
    data_exchange[1]=0xE1; //
    data_exchange[2]=0x01; // version
    data_exchange[3]=0xE9; // Start_Communication.CNF
    data_exchange[4]=0x60; //
    data_exchange[5]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    data_exchange[6]=0x00; //
}

void EV_Compose_Stop_Charging_Req()
{
	Clean_Data_Exchange();

	exchange_data_size = 7;

	data_exchange[0]=0x88; // Protocol HomeplugAV
    data_exchange[1]=0xE1; //
    data_exchange[2]=0x01; // version
    data_exchange[3]=0xEC; // Start_Communication.CNF
    data_exchange[4]=0x60; //
    data_exchange[5]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    data_exchange[6]=0x00; //
}

void EV_Compose_Stop_Charging_Cnf()
{
	Clean_Data_Exchange();

	exchange_data_size = 7;

	data_exchange[0]=0x88; // Protocol HomeplugAV
    data_exchange[1]=0xE1; //
    data_exchange[2]=0x01; // version
    data_exchange[3]=0xED; // Start_Communication.CNF
    data_exchange[4]=0x60; //
    data_exchange[5]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    data_exchange[6]=0x00; //
}


