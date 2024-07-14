/*
 * homeplug_evse.h
 *
 *  Created on: Dec 11, 2023
 *      Author: PC
 */

extern void HPGP_EVSE_Get_Address(uint8_t* mac, uint8_t offset, uint8_t len);
extern void HPGP_EVSE_Respond_SLAC_PARM_CNF(void);
extern void HPGP_EVSE_Respond_ATTEN_CHAR_IND(void);
extern void HPGP_EVSE_Respond_SLAC_MATCH_CNF(void);
