/*
 * homeplug_evse.c
 *
 *  Created on: Dec 7, 2023
 *      Author: PC
 */

#include "cims.h"


/* PEV State */

uint8_t evseAddress[6]= {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
uint8_t pevAddress[6];
uint8_t runID[6];
uint8_t atten_profile[58];

void HPGP_EVSE_Get_Address(uint8_t* mac, uint8_t offset, uint8_t len){
	memcpy(mac, &eth_rx_buffer[offset], len);
}

void HPGP_EVSE_Get_Atten_Profile(uint8_t offset){
	memcpy(atten_profile, &eth_rx_buffer[offset], 59);
}

void HPGP_EVSE_Respond_SLAC_PARM_CNF(void){
	/* SLAC_PARM request, as it was recorded 2021-12-17 WP charger 2 */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_EVSE_Get_Address(pevAddress, 6, 6);
    HPGP_Fill_Address(pevAddress, 0);
    /* Source MAC */
    HPGP_Fill_Address(evseAddress, 6);
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0x65; // SLAC_PARAM.CNF
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    eth_tx_buffer[19]=0xFF; //M-Sound Target
    eth_tx_buffer[20]=0xFF;
    eth_tx_buffer[21]=0xFF;
    eth_tx_buffer[22]=0xFF;
    eth_tx_buffer[23]=0xFF;
    eth_tx_buffer[24]=0xFF;
    eth_tx_buffer[25]=0x0A; //number of sounds: 10
    eth_tx_buffer[26]=0x06; // timeout N*100ms. Normally 6, means in 600ms all sounds must have been tranmitted.
                            // Todo: As long we are a little bit slow, lets give 1000ms instead of 600, so that the
                            // charger is able to catch it all.
    eth_tx_buffer[27]=0x01; // response type
    HPGP_Fill_Address(pevAddress, 28); // 28 to 33: sound_forwarding_sta, MAC of the PEV
    eth_tx_buffer[34]=0x00;
    eth_tx_buffer[35]=0x00;
    HPGP_EVSE_Get_Address(runID, 21, 6);
    HPGP_Fill_Address(runID, 36); // 36 to 41: runid, filled with MAC of PEV and two bytes 00 00
}

void HPGP_EVSE_Respond_ATTEN_CHAR_IND(void){
	/* SLAC_PARM request, as it was recorded 2021-12-17 WP charger 2 */
    eth_tx_size = 129;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_EVSE_Get_Address(pevAddress, 6, 6);
    HPGP_Fill_Address(pevAddress, 0);
    /* Source MAC */
    HPGP_Fill_Address(evseAddress, 6);
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0x6E; // ATTEN_CHAR.IND
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    eth_tx_buffer[19]=0x00; // application type
    eth_tx_buffer[20]=0x00; // security type
    HPGP_Fill_Address(pevAddress, 21);
    HPGP_Fill_Address(runID, 27);
    //fill atten_profile, but not finish
}

void HPGP_EVSE_Respond_SLAC_MATCH_CNF(void){
	/* SLAC_PARM request, as it was recorded 2021-12-17 WP charger 2 */
    eth_tx_size = 109;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_EVSE_Get_Address(pevAddress, 6, 6);
    HPGP_Fill_Address(pevAddress, 0);
    /* Source MAC */
    HPGP_Fill_Address(evseAddress, 6);
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0x7D; // SLAC_MATCH.CNF
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    eth_tx_buffer[19]=0x00; // application type
    eth_tx_buffer[20]=0x00; // security type
    eth_tx_buffer[21]=0x56;	// MVF Length
    eth_tx_buffer[22]=0x56;
    HPGP_Fill_Address(pevAddress, 40);
    HPGP_Fill_Address(evseAddress, 63);
    HPGP_Fill_Address(runID, 69);
    HPGP_Set_NID(85);
    HPGP_Set_NMK(93);
}

