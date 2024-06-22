/*
 * exchange_data.h
 *
 *  Created on: Dec 16, 2023
 *      Author: PC
 */

#define EXCHANGE_DATA_LEN 200

typedef struct ev_exchange{
	uint8_t ev_model;
	uint8_t battery_capacity;
	uint16_t current_request;
	uint16_t voltage_request;
	uint16_t max_voltage_limit;
	uint16_t min_current_limit;
	uint16_t max_charging_time;
}ev_exchange;

typedef struct evse_exchange{
	uint16_t max_voltage_rate;
	uint16_t max_current_rate;
	uint16_t charging_time;
}evse_exchange;

extern ev_exchange ev1;
extern evse_exchange evse1;

extern uint8_t data_exchange[EXCHANGE_DATA_LEN];
extern uint8_t exchange_data_size;

extern void EV_Compose_Start_Communication_Req(void);
extern void EV_Evaluate_Start_Communication_Req(void);
extern void EV_Compose_Start_Communication_Cnf(void);
extern void EV_Evaluate_Start_Communication_Cnf(void);

extern void EV_Compose_Setup_Done_Req(void);
extern void EV_Compose_Setup_Done_Cnf(void);

extern void EV_Compose_Start_Charging_Req(void);
extern void EV_Compose_Start_Charging_Cnf(void);

extern void EV_Compose_Stop_Charging_Req(void);
extern void EV_Compose_Stop_Charging_Cnf(void);
