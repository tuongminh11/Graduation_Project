/*
 * homeplug.h
 *
 *  Created on: Nov 28, 2023
 *      Author: PC
 */


extern void HPGP_Fill_Address(const uint8_t *mac, uint8_t offset);

extern uint16_t HPGP_Get_MTYPE(uint8_t *eth_rx_buffer);
extern void HPGP_Clean_Tx_Buffer(void);
extern void HPGP_Set_NMK(uint8_t index);
extern void HPGP_Set_NID(uint8_t index);

extern uint16_t HPGP_Get_MMTYPE(void);

extern void HPGP_Compose_Test_String(void);
extern void HPGP_Compose_Get_Sw_Req(void);
extern void HPGP_Compose_SLAC_PARM_REQ(void);
extern void HPGP_Evaluate_SLAC_PARM_CNF(void);

extern void HPGP_Compose_START_ATTEN_CHAR_IND(void);
extern void HPGP_Compose_MNBC_SOUND_IND(uint8_t sound);
extern void HPGP_Evaluate_ATTEN_CHAR_IND(void);
extern void HPGP_Compose_ATTEN_CHAR_RSP(void);

extern void HPGP_Compose_Exchange_Data_Req(void);
extern void HPGP_Compose_Exchange_Data_Cnf(void);

extern void HPGP_Compose_Setup_Done_Req(void);
extern void HPGP_Compose_Setup_Done_Cnf(void);

extern void HPGP_Compose_Start_Charging_Req(void);
extern void HPGP_Compose_Start_Charging_Cnf(void);

extern void HPGP_Compose_Stop_Charging_Req(void);
extern void HPGP_Compose_Stop_Charging_Cnf(void);

extern void HPGP_Compose_SET_KEY_REQ(void);

extern void HPGP_Evaluate_Get_Sw_Cnf(void);
extern void HPGP_Evaluate_HomePlug_Packet(void);

