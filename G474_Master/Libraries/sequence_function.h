/*
 * sequence_function.h
 *
 *  Created on: Dec 18, 2023
 *      Author: PC
 */

#define PARAMETER_EXCHANGE_LEN 200
/* Session */
#define INITIALIZATION 0x0000
#define ENERGY_TRANSFER 0x0100
#define SHUT_DOWN 0x0200

/* State */
#define STATE_A 0xA000
#define STATE_B 0xB000
#define STATE_C 0xC000

/* Sequence Fuction */
#define COMMUNICATION_INIT 0x00
#define PARAMETER_EXCHANGE 0x04
#define EVSE_CONNECTOR_LOCK 0x08
#define EV_CONTACTOR_CLOSE 0x0C
#define CHARGING_CURRENT_DEMAND 0x10
#define CURRENT_SUPPRESSION 0x14
#define ZERO_CURRENT_CONFIRM 0x18
#define VOLTAGE_VERIFICATION 0x1C
#define CONNECTOR_UNLOCK 0x20
#define END_OF_CHARGE 0x24

/* Message Type */
#define REQUEST 0x00
#define CONFIRM 0x01
#define INDICATION 0x02
#define RESPONSE 0x03

typedef struct evse_parameter{
	uint8_t control_protocol_number;
	uint16_t available_output_voltage;
	uint16_t available_output_current;
	uint8_t battery_incompability;

	uint8_t station_status;
	uint16_t output_voltage;
	uint16_t output_current;
	uint16_t remaining_charging_time;
	uint8_t station_malfunction;
	uint8_t charging_system_malfunction;

	uint8_t charging_stop_control;
}evse_parameter;

typedef struct ev_parameter{
	uint8_t control_protocol_number;
	uint8_t rate_capacity_battery;
	uint8_t current_battery;
	uint16_t max_battery_voltage;
	uint16_t max_charging_time;
	uint16_t target_battery_voltage;
	uint8_t vehicle_charging_enabled;

	uint16_t charging_current_request;
	uint8_t charging_system_fault;
	uint8_t vehicle_shift_lever_position;
}ev_parameter;

extern ev_parameter myEV;
extern evse_parameter myESVE;

extern uint16_t PEF_Get_Sequence_State();
extern void PEF_Compose_Initialization_Req();
extern void PEF_Compose_Connector_Lock_Req();
extern void PEF_Compose_Contactor_Close_Req();
extern void PEF_Compose_Charging_Current_Demand_Req(uint16_t current_request, uint8_t system_fault, uint8_t shift_pos, uint8_t current_battery);
extern void PEF_Compose_Current_Suppression_Req();
extern void PEF_Evaluate_Exchange_Data();
