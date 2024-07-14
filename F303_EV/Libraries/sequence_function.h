/*
 * sequence_function.h
 *
 *  Created on: Dec 18, 2023
 *      Author: PC
 */

#define PARAMETER_EXCHANGE_LEN 200

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


	uint16_t init_battery;
	uint8_t control_protocol_number;
	uint8_t rate_capacity_battery;
	uint16_t current_battery; // Pre_SoC
	uint16_t max_battery; //SoH
	uint16_t max_charging_time; //
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
extern void PEF_Compose_Charging_Current_Demand_Req(uint16_t current_request, uint8_t system_fault, uint8_t shift_pos);
extern void PEF_Compose_Current_Suppression_Req();
extern void PEF_Evaluate_Exchange_Data();
extern void PEF_Reset_RXBUFFER() ;
