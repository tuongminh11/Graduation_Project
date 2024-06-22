/*
 * sequence_function.c
 *
 *  Created on: Dec 18, 2023
 *      Author: PC
 */

#include "cims.h"

///* Session */
//#define INITIALIZATION 0x0000
//#define ENERGY_TRANSFER 0x0100
//#define SHUT_DOWN 0x0200
//
///* State */
//#define STATE_A 0xA000
//#define STATE_B 0xB000
//#define STATE_C 0xC000
//
///* Sequence Fuction */
//#define COMMUNICATION_INIT 0x00
//#define PARAMETER_EXCHANGE 0x04
//#define EVSE_CONNECTOR_LOCK 0x08
//#define EV_CONTACTOR_CLOSE 0x0C
//#define CHARGING_CURRENT_DEMAND 0x10
//#define CURRENT_SUPPRESSION 0x14
//#define ZERO_CURRENT_CONFIRM 0x18
//#define VOLTAGE_VERIFICATION 0x1C
//#define CONNECTOR_UNLOCK 0x20
//#define END_OF_CHARGE 0x24
//
///* Message Type */
//#define REQUEST 0x00
//#define CONFIRM 0x01
//#define INDICATION 0x02
//#define RESPONSE 0x03

uint8_t sessionID[6]={0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF};
uint16_t notification;
uint16_t faultCode;

uint8_t evID[6]={0x16, 0x08, 0x20, 0x01, 0x17, 0x15};
//uint8_t evseID[6]={0x05, 0x08, 0x20, 0x01, 0x19, 0x82};
uint8_t evseID[6]={0x00, 0x00, 0x00, 0x44, 0x55, 0x66};
uint8_t yourID[6];
uint8_t broadcastID[6]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

ev_parameter myEV;
evse_parameter myEVSE;

/* Parameter Exchanged Function */
void PEF_Get_Data(uint8_t* mac, uint8_t offset, uint8_t len){
	memcpy(mac, &eth_rx_buffer[offset], len);
}
void PEF_Compose_Initialization_Req(void){
	/* Initialization Request */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(broadcastID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB0; // Session + State
    eth_tx_buffer[26]=0x00; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}

void PEF_Compose_Initialization_Cnf(void){
	/* Initialization Confirm */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evseID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE1; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB0; // Session + State
    eth_tx_buffer[26]=0x01; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}

void PEF_Evaluate_Initialization(void){
    PEF_Get_Data(yourID, 6, 6);
    PEF_Get_Data(sessionID, 19, 6);
}



void PEF_Compose_Parameter_Exchange_Req(void){
	myEV.control_protocol_number = 0xAA;
	myEV.rate_capacity_battery = 0x3A;
	myEV.current_battery = 0x14;
	myEV.max_battery_voltage = 0x0320;
	myEV.max_charging_time = 0x0258;
	myEV.target_battery_voltage = 0x00C8;
	myEV.vehicle_charging_enabled = 0xAA;

	/* Parameter Exchange Request */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB0; // Session + State
    eth_tx_buffer[26]=0x04; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
    eth_tx_buffer[29] = myEV.control_protocol_number;
    eth_tx_buffer[30] = myEV.rate_capacity_battery ;
    eth_tx_buffer[31] = myEV.current_battery;
    eth_tx_buffer[32] = (myEV.max_battery_voltage >> 8);
    eth_tx_buffer[33] = myEV.max_battery_voltage;
    eth_tx_buffer[34] = (myEV.max_charging_time >> 8);
    eth_tx_buffer[35] = myEV.max_charging_time;
    eth_tx_buffer[36] = (myEV.target_battery_voltage >> 8);
    eth_tx_buffer[37] = myEV.target_battery_voltage;
    eth_tx_buffer[38] = myEV.vehicle_charging_enabled;
}

void PEF_Compose_Parameter_Exchange_Res(void){
	myEVSE.control_protocol_number = 0xAA;
	myEVSE.available_output_voltage = 0x0190;
	myEVSE.available_output_current = 0x0032;
	myEVSE.battery_incompability = 0xAA;

	/* Parameter Exchange Response */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evseID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB0; // Session + State
    eth_tx_buffer[26]=0x07; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
    eth_tx_buffer[29] = myEVSE.control_protocol_number;
    eth_tx_buffer[30] = (myEVSE.available_output_voltage >> 8);
    eth_tx_buffer[31] = myEVSE.available_output_voltage;
    eth_tx_buffer[32] = (myEVSE.available_output_current >> 8);
    eth_tx_buffer[33] = myEVSE.available_output_current;
    eth_tx_buffer[34] = myEVSE.battery_incompability;
}

void PEF_Compose_Parameter_Exchange_Cnf(void){
	/* Parameter Exchange Confirm */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB0; // Session + State
    eth_tx_buffer[26]=0x05; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}

void PEF_Evaluate_Parameter_Exchange_Req(void){
	myEV.control_protocol_number = eth_rx_buffer[29];
	myEV.rate_capacity_battery = eth_rx_buffer[30];
	myEV.current_battery = eth_rx_buffer[31];
	myEV.max_battery_voltage = (eth_rx_buffer[32] << 8) + eth_rx_buffer[33];
	myEV.max_charging_time = (eth_rx_buffer[34] << 8) + eth_rx_buffer[35];
	myEV.target_battery_voltage = (eth_rx_buffer[36] << 8) + eth_rx_buffer[37];
	myEV.vehicle_charging_enabled = eth_rx_buffer[38];
}

void PEF_Evaluate_Parameter_Exchange_Res(void){
	myEVSE.control_protocol_number = eth_rx_buffer[29];
	myEVSE.available_output_voltage = (eth_rx_buffer[30] << 8) + eth_rx_buffer[31];
	myEVSE.available_output_current = (eth_rx_buffer[32] << 8) + eth_rx_buffer[33];
	myEVSE.battery_incompability = eth_rx_buffer[34];
}



void PEF_Compose_Connector_Lock_Req(void){
	/* Connector Lock Request */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB0; // Session + State
    eth_tx_buffer[26]=0x08; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}

void PEF_Compose_Connector_Lock_Cnf(void){
	/* Connector Lock Confirm */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evseID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB0; // Session + State
    eth_tx_buffer[26]=0x09; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}

void PEF_Compose_Contactor_Close_Req(void){
	/* Contactor Close Request */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evseID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xC1; // Session + State
    eth_tx_buffer[26]=0x0C; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}

void PEF_Compose_Contactor_Close_Cnf(void){
	/* Contactor Close Confirm */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xC1; // Session + State
    eth_tx_buffer[26]=0x0D; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}



void PEF_Compose_Charging_Current_Demand_Req(uint16_t current_request, uint8_t system_fault, uint8_t shift_pos, uint8_t current_battery){
	myEV.charging_current_request = current_request;
	myEV.charging_system_fault = system_fault;
	myEV.vehicle_shift_lever_position = shift_pos;
	myEV.current_battery = current_battery;

	/* Charging by Current Demand Request */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xC1; // Session + State
    eth_tx_buffer[26]=0x10; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
    eth_tx_buffer[29] = (myEV.charging_current_request >> 8);
    eth_tx_buffer[30] = myEV.charging_current_request;
    eth_tx_buffer[31] = myEV.charging_system_fault;
    eth_tx_buffer[32] = myEV.vehicle_shift_lever_position;
    eth_tx_buffer[33] = myEV.current_battery;
}

void PEF_Compose_Charging_Current_Demand_Res(uint8_t station_status, uint16_t output_voltage, uint16_t output_current, uint16_t remaining_charging_time, uint8_t station_mal, uint8_t charge_system_mal){
	myEVSE.station_status = station_status;
	myEVSE.output_current = output_current;
	myEVSE.output_voltage = output_voltage;
	myEVSE.remaining_charging_time = remaining_charging_time;
	myEVSE.station_malfunction = station_mal;
	myEVSE.charging_system_malfunction = charge_system_mal;

	/* Charging by Current Demand Response */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evseID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xC1; // Session + State
    eth_tx_buffer[26]=0x13; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
    eth_tx_buffer[29] = myEVSE.station_status;
    eth_tx_buffer[30] = (myEVSE.output_current >> 8);
    eth_tx_buffer[31] = myEVSE.output_current;
    eth_tx_buffer[32] = (myEVSE.output_voltage >> 8);
    eth_tx_buffer[33] = myEVSE.output_voltage;
    eth_tx_buffer[34] = (myEVSE.remaining_charging_time >> 8);
    eth_tx_buffer[35] = myEVSE.remaining_charging_time;
    eth_tx_buffer[36] = myEVSE.station_malfunction;
    eth_tx_buffer[37] = myEVSE.charging_system_malfunction;
}

void PEF_Compose_Charging_Current_Demand_Cnf(void){
	/* Charging by Current Demand Confirm */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xC1; // Session + State
    eth_tx_buffer[26]=0x11; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}

void PEF_Evaluate_Charging_Current_Demand_Req(void){
	myEV.charging_current_request = (eth_rx_buffer[29] << 8) + eth_rx_buffer[30];
	myEV.charging_system_fault = eth_rx_buffer[31];
	myEV.vehicle_shift_lever_position = eth_rx_buffer[32];
	myEV.current_battery = eth_rx_buffer[33];
}

void PEF_Evaluate_Charging_Current_Demand_Res(void){
	myEVSE.station_status = eth_rx_buffer[29];
	myEVSE.output_current = (eth_rx_buffer[30] << 8) + eth_rx_buffer[31];
	myEVSE.output_voltage = (eth_rx_buffer[32] << 8) + eth_rx_buffer[33];
	myEVSE.remaining_charging_time = (eth_rx_buffer[34] << 8) + eth_rx_buffer[35];
	myEVSE.station_malfunction = eth_rx_buffer[36];
	myEVSE.charging_system_malfunction = eth_rx_buffer[37];
}



void PEF_Compose_Current_Suppression_Req(void){
	myEV.vehicle_charging_enabled = 0xAA;

	/* Current Suppression Request */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xC1; // Session + State
    eth_tx_buffer[26]=0x14; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
    eth_tx_buffer[29] = myEV.vehicle_charging_enabled;
}

void PEF_Compose_Current_Suppression_Res(uint8_t station_status, uint8_t charging_stop_control, uint16_t output_voltage, uint16_t output_current){
	myEVSE.station_status = station_status;
	myEVSE.charging_stop_control = charging_stop_control;
	myEVSE.output_voltage = output_voltage;
	myEVSE.output_current = output_current;

	/* Current Suppression Response */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evseID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xC1; // Session + State
    eth_tx_buffer[26]=0x17; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
	eth_tx_buffer[29] = myEVSE.station_status;
	eth_tx_buffer[30] = myEVSE.charging_stop_control;
	eth_tx_buffer[31] = (myEVSE.output_voltage >> 8);
	eth_tx_buffer[32] = myEVSE.output_voltage;
	eth_tx_buffer[33] = (myEVSE.output_current >>8);
	eth_tx_buffer[34] = myEVSE.output_current;
}

void PEF_Compose_Current_Suppression_Cnf(void){
	/* Current Suppression Confirm */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xC1; // Session + State
    eth_tx_buffer[26]=0x15; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}

void PEF_Evaluate_Current_Suppression_Res(void){
	myEVSE.station_status = eth_rx_buffer[29];
	myEVSE.charging_stop_control = eth_rx_buffer[30];
	myEVSE.output_voltage = (eth_rx_buffer[31] << 8) + eth_rx_buffer[32];
	myEVSE.output_current = (eth_rx_buffer[33] << 8) + eth_rx_buffer[34];
}



void PEF_Compose_Zero_Current_Confirm_Req(void){
	/* Zero Current Confirm Request */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evseID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB2; // Session + State
    eth_tx_buffer[26]=0x18; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}

void PEF_Compose_Zero_Current_Confirm_Cnf(void){
	/* Zero Current Confirm */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB2; // Session + State
    eth_tx_buffer[26]=0x19; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}



void PEF_Compose_Voltage_Verification_Req(uint16_t output_voltage){
	myEVSE.output_voltage = output_voltage;

	/* Voltage Verification Request */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evseID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB2; // Session + State
    eth_tx_buffer[26]=0x1C; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
    eth_tx_buffer[29] = (myEVSE.output_voltage >> 8);
    eth_tx_buffer[30] = myEVSE.output_voltage;
}

void PEF_Compose_Voltage_Verification_Cnf(void){
	/* Voltage Verification Confirm */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB2; // Session + State
    eth_tx_buffer[26]=0x1D; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}

void PEF_Evaluate_Voltage_Verification_Cnf(void){
	myEVSE.output_voltage = (eth_rx_buffer[29] << 8) + eth_rx_buffer[30];
}

void PEF_Compose_Connector_Unlock_Req(void){
	/* Connector Unlock Request */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evseID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB2; // Session + State
    eth_tx_buffer[26]=0x20; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}

void PEF_Compose_Connector_Unlock_Cnf(void){
	/* Connector Unlock Confirm */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB2; // Session + State
    eth_tx_buffer[26]=0x21; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}



void PEF_Compose_End_of_Charge_Req(void){
	/* End of Charge Request */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB2; // Session + State
    eth_tx_buffer[26]=0x24; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}

void PEF_Compose_End_of_Charge_Cnf(void){
	/* End of Charge Request */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(yourID, 0);
    /* Source MAC */
    HPGP_Fill_Address(evseID, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0xE0; // EXCHANGE_DATA
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    /* Exchange Data */
    // Header
    HPGP_Fill_Address(sessionID, 19); // Session ID
    eth_tx_buffer[25]=0xB2; // Session + State
    eth_tx_buffer[26]=0x25; // Function + Message Type
    eth_tx_buffer[27]=0x00;	// 2 bytes fault code
    eth_tx_buffer[28]=0x00;
    // Body
}

uint16_t PEF_Get_Sequence_State(void){
	uint16_t message_type;
	message_type = (eth_rx_buffer[25] << 8) + eth_rx_buffer[26];
	return message_type;
}

void PEF_Handle_Initialization_Req(void){
	sprintf(serial_output_buffer, "[PLC] received INITIALIZATION.REQ ");
	Serial_Print();

	PEF_Evaluate_Initialization();
	PEF_Compose_Initialization_Cnf();
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Initialization_Cnf(void){
	sprintf(serial_output_buffer, "[PLC] received INITIALIZATION.CNF ");
	Serial_Print();

	PEF_Evaluate_Initialization();
	PEF_Compose_Parameter_Exchange_Req();
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Parameter_Exchange_Req(void){
	sprintf(serial_output_buffer, "[PLC] received PARAMETER_EXCHANGE.REQ ");
	Serial_Print();

	PEF_Evaluate_Parameter_Exchange_Req();
	PEF_Compose_Parameter_Exchange_Res();
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Parameter_Exchange_Res(void){
	sprintf(serial_output_buffer, "[PLC] received PARAMETER_EXCHANGE.RES ");
	Serial_Print();

	PEF_Evaluate_Parameter_Exchange_Res();
	PEF_Compose_Parameter_Exchange_Cnf();
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Parameter_Exchange_Cnf(void){
	sprintf(serial_output_buffer, "[PLC] received PARAMETER_EXCHANGE.CNF ");
	Serial_Print();
}

void PEF_Handle_Connector_Lock_Req(void){
	sprintf(serial_output_buffer, "[PLC] received CONNECTOR_LOCK.REQ ");
	Serial_Print();

	PEF_Compose_Connector_Lock_Cnf();
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Connector_Lock_Cnf(void){
	sprintf(serial_output_buffer, "[PLC] received CONNECTOR_LOCK.CNF ");
	Serial_Print();
}

void PEF_Handle_Contactor_Close_Req(void){
	sprintf(serial_output_buffer, "[PLC] received CONTACTOR_CLOSE.REQ ");
	Serial_Print();

	PEF_Compose_Contactor_Close_Cnf();
	SPI_QCA7000_Send_Eth_Frame();

	//	PEF_Compose_Charging_Current_Demand_Req(current_request, system_fault, shift_pos, current_battery);
	//	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Contactor_Close_Cnf(void){
	sprintf(serial_output_buffer, "[PLC] received CONTACTOR_CLOSE.CNF ");
	Serial_Print();
}

void PEF_Handle_Charging_Current_Demand_Req(void){
	sprintf(serial_output_buffer, "[PLC] received CHARGING_CURRENT_DEMAND.REQ ");
	Serial_Print();

	PEF_Evaluate_Charging_Current_Demand_Req();
	uint16_t a = 0x0123;
	uint16_t b = 0x0050;
	uint16_t c = 0x0280;
	PEF_Compose_Charging_Current_Demand_Res( 0xAA, a, b, c, 0xAA, 0xAA);
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Charging_Current_Demand_Res(void){
	sprintf(serial_output_buffer, "[PLC] received CHARGING_CURRENT_DEMAND.RES ");
	Serial_Print();

	PEF_Evaluate_Charging_Current_Demand_Res();
}


void PEF_Handle_Current_Suppression_Req(void){
	sprintf(serial_output_buffer, "[PLC] received CURRENT_SUPPRESSION.REQ ");
	Serial_Print();

	uint16_t a = 0x0123;
	uint16_t b = 0x0050;
	PEF_Compose_Current_Suppression_Res(0xAA, 0xAA, a, b);
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Current_Suppression_Res(void){
	sprintf(serial_output_buffer, "[PLC] received CURRENT_SUPPRESSION.RES ");
	Serial_Print();

	PEF_Evaluate_Current_Suppression_Res();
	PEF_Compose_Current_Suppression_Cnf();
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Current_Suppression_Cnf(void){
	sprintf(serial_output_buffer, "[PLC] received CURRENT_SUPPRESSION.CNF ");
	Serial_Print();

	PEF_Compose_Zero_Current_Confirm_Req();
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Zero_CURRENT_CONFIRM_Req(void){
	sprintf(serial_output_buffer, "[PLC] received ZERO_CURRENT_CONFIRM.REQ ");
	Serial_Print();

	PEF_Compose_Zero_Current_Confirm_Cnf();
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Zero_CURRENT_CONFIRM_Cnf(void){
	sprintf(serial_output_buffer, "[PLC] received ZERO_CURRENT_CONFIRM.CNF ");
	Serial_Print();

	uint16_t vol = 0;
	PEF_Compose_Voltage_Verification_Req(vol);
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Voltage_Verification_Req(void){
	sprintf(serial_output_buffer, "[PLC] received VOLTAGE_VERIFICATION.REQ ");
	Serial_Print();

	PEF_Compose_Voltage_Verification_Cnf();
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Voltage_Verification_Cnf(void){
	sprintf(serial_output_buffer, "[PLC] received VOLTAGE_VERIFICATION.CNF ");
	Serial_Print();

	PEF_Compose_Connector_Unlock_Req();
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Connector_Unlock_Req(void){
	sprintf(serial_output_buffer, "[PLC] received CONNECTOR_UNLOCK.REQ ");
	Serial_Print();

	PEF_Compose_Connector_Unlock_Cnf();
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_Connector_Unlock_Cnf(void){
	sprintf(serial_output_buffer, "[PLC] received CONNECTOR_UNLOCK.CNF ");
	Serial_Print();

	PEF_Compose_End_of_Charge_Req();
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_End_of_Charge_Req(void){
	sprintf(serial_output_buffer, "[PLC] received END_OF_CHARGE.REQ ");
	Serial_Print();

	PEF_Compose_End_of_Charge_Cnf();
	SPI_QCA7000_Send_Eth_Frame();
}

void PEF_Handle_End_of_Charge_Cnf(void){
	sprintf(serial_output_buffer, "[PLC] received END_OF_CHARGE.CNF ");
	Serial_Print();
}

void PEF_Evaluate_Exchange_Data(){
	switch(PEF_Get_Sequence_State()){
	case INITIALIZATION + STATE_B + COMMUNICATION_INIT + REQUEST:
		PEF_Handle_Initialization_Req();
		break;
	case INITIALIZATION + STATE_B + COMMUNICATION_INIT + CONFIRM:
		PEF_Handle_Initialization_Cnf();
		break;
	case INITIALIZATION + STATE_B + PARAMETER_EXCHANGE + REQUEST:
		PEF_Handle_Parameter_Exchange_Req();
		break;
	case INITIALIZATION + STATE_B + PARAMETER_EXCHANGE + RESPONSE:
		PEF_Handle_Parameter_Exchange_Res();
		break;
	case INITIALIZATION + STATE_B + PARAMETER_EXCHANGE + CONFIRM:
		PEF_Handle_Parameter_Exchange_Cnf();
		break;
	case INITIALIZATION + STATE_B + EVSE_CONNECTOR_LOCK + REQUEST:
		PEF_Handle_Connector_Lock_Req();
		break;
	case INITIALIZATION + STATE_B + EVSE_CONNECTOR_LOCK + CONFIRM:
		PEF_Handle_Connector_Lock_Cnf();
		break;
	case ENERGY_TRANSFER + STATE_C + EV_CONTACTOR_CLOSE + REQUEST:
		PEF_Handle_Contactor_Close_Req();
		break;
	case ENERGY_TRANSFER + STATE_C + EV_CONTACTOR_CLOSE + CONFIRM:
		PEF_Handle_Contactor_Close_Cnf();
		break;
	case ENERGY_TRANSFER + STATE_C + CHARGING_CURRENT_DEMAND + REQUEST:
		PEF_Handle_Charging_Current_Demand_Req();
		break;
	case ENERGY_TRANSFER + STATE_C + CHARGING_CURRENT_DEMAND + RESPONSE:
		PEF_Handle_Charging_Current_Demand_Res();
		break;
	case ENERGY_TRANSFER + STATE_C + CURRENT_SUPPRESSION + REQUEST:
		PEF_Handle_Current_Suppression_Req();
		break;
	case ENERGY_TRANSFER + STATE_C + CURRENT_SUPPRESSION + RESPONSE:
		PEF_Handle_Current_Suppression_Res();
		break;
	case ENERGY_TRANSFER + STATE_C + CURRENT_SUPPRESSION + CONFIRM:
		PEF_Handle_Current_Suppression_Cnf();
		break;
	case SHUT_DOWN + STATE_B + ZERO_CURRENT_CONFIRM + REQUEST:
		PEF_Handle_Zero_CURRENT_CONFIRM_Req();
		break;
	case SHUT_DOWN + STATE_B + ZERO_CURRENT_CONFIRM + CONFIRM:
		PEF_Handle_Zero_CURRENT_CONFIRM_Cnf();
		break;
	case SHUT_DOWN + STATE_B + VOLTAGE_VERIFICATION + REQUEST:
		PEF_Handle_Voltage_Verification_Req();
		break;
	case SHUT_DOWN + STATE_B + VOLTAGE_VERIFICATION + CONFIRM:
		PEF_Handle_Voltage_Verification_Cnf();
		break;
	case SHUT_DOWN + STATE_B + CONNECTOR_UNLOCK + REQUEST:
		PEF_Handle_Connector_Unlock_Req();
		break;
	case SHUT_DOWN + STATE_B + CONNECTOR_UNLOCK + CONFIRM:
		PEF_Handle_Connector_Unlock_Cnf();
		break;
	case SHUT_DOWN + STATE_B + END_OF_CHARGE + REQUEST:
		PEF_Handle_End_of_Charge_Req();
		break;
	case SHUT_DOWN + STATE_B + END_OF_CHARGE + CONFIRM:
		PEF_Handle_End_of_Charge_Cnf();
		break;
	}
}




