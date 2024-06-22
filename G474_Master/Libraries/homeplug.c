/*
 * homeplug.c
 *
 *  Created on: Nov 28, 2023
 *      Author: PC
 */

#include "cims.h"


/* Communication Module Functions */
#define CM_SET_KEY  0x6008
#define CM_GET_KEY  0x600C
#define CM_SC_JOIN  0x6010
#define CM_CHAN_EST  0x6014
#define CM_TM_UPDATE  0x6018
#define CM_AMP_MAP  0x601C
#define CM_BRG_INFO  0x6020
#define CM_CONN_NEW  0x6024
#define CM_CONN_REL  0x6028
#define CM_CONN_MOD  0x602C
#define CM_CONN_INFO  0x6030
#define CM_STA_CAP  0x6034
#define CM_NW_INFO  0x6038
#define CM_GET_BEACON  0x603C
#define CM_HFID  0x6040
#define CM_MME_ERROR  0x6044
#define CM_NW_STATS  0x6048
#define CM_SLAC_PARAM  0x6064
#define CM_START_ATTEN_CHAR  0x6068
#define CM_ATTEN_CHAR  0x606C
#define CM_PKCS_CERT  0x6070
#define CM_MNBC_SOUND  0x6074
#define CM_VALIDATE  0x6078
#define CM_SLAC_MATCH  0x607C
#define CM_SLAC_USER_DATA  0x6080
#define CM_ATTEN_PROFILE  0x6084
#define CM_GET_SW  0xA000

#define CM_EXCHANGE_DATA 0x60E0
#define CM_SETUP_DONE 0x60E4
#define CM_START_CHARGING 0x60E8
#define CM_STOP_CHARGING 0x60EC


/* Management Message Type */
#define MMTYPE_REQ  0x0000
#define MMTYPE_CNF  0x0001
#define MMTYPE_IND  0x0002
#define MMTYPE_RSP  0x0003

/* EVSE State*/
#define STATE_INITIAL  0
#define STATE_MODEM_SEARCH_ONGOING  1
#define STATE_READY_FOR_SLAC        2
#define STATE_WAITING_FOR_MODEM_RESTARTED  3
#define STATE_WAITING_FOR_SLAC_PARAM_CNF   4
#define STATE_SLAC_PARAM_CNF_RECEIVED      5
#define STATE_BEFORE_START_ATTEN_CHAR      6
#define STATE_SOUNDING                     7
#define STATE_WAIT_FOR_ATTEN_CHAR_IND      8
#define STATE_ATTEN_CHAR_IND_RECEIVED      9
#define STATE_DELAY_BEFORE_MATCH           10
#define STATE_WAITING_FOR_SLAC_MATCH_CNF   11
#define STATE_WAITING_FOR_RESTART2         12
#define STATE_FIND_MODEMS2                 13
#define STATE_WAITING_FOR_SW_VERSIONS      14
#define STATE_READY_FOR_SDP                15
#define STATE_SDP                          16

#define iAmPev 1 /* This project is intended only for PEV mode at the moment. */
#define iAmEvse 0

/* Global Variable */
uint8_t NID[7];
uint8_t NMK[16];
uint8_t evseMac[6];
uint8_t sourceMac[6];
uint8_t localModemMac[6];
uint8_t verLen;
char strVersion[200];

uint8_t localModemCurrentKey[16];
uint8_t localModemFound;

uint16_t checkpointNumber;

uint8_t numberOfSoftwareVersionResponses;
uint8_t numberOfFoundModems;

uint8_t pevSequenceState;
uint16_t pevSequenceCyclesInState;
uint16_t pevSequenceDelayCycles;
uint8_t remainingNumberOfSounds;

uint8_t nRemainingStartAttenChar;
uint8_t AttenCharIndNumberOfSounds;
uint8_t SdpRepetitionCounter;
uint8_t isSDPDone;
uint8_t sdp_state;
uint8_t nEvseModemMissingCounter;

/* MAC Address */
const uint8_t MAC_BROADCAST[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
const uint8_t myMAC[6] = {0xFE, 0xED, 0xBE, 0xEF, 0xAF, 0xFE}; //pev MAC
const uint8_t yourMAC[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}; //evse MAC
uint8_t pevMAC[6];
uint8_t evseMAC[6];

/********** local prototypes *****************************************/
void HPGP_Compose_ATTEN_CHAR_RSP(void);
void HPGP_SLAC_Enter_State(int n);
void HPGP_Compose_SET_KEY_REQ(void);

void HPGP_Fill_Address(const uint8_t *mac, uint8_t offset) {
 /* at offset 0 in the ethernet frame, we have the destination MAC.
    we can give a different offset, to re-use the MAC also in the data area */
  memcpy(&eth_tx_buffer[offset], mac, 6);
}

/* Extracting the MTYPE from a received message. */
uint16_t HPGP_Get_MTYPE(uint8_t *eth_rx_buffer) {
  uint16_t mtype=0;
  mtype=eth_rx_buffer[12]*256 + eth_rx_buffer[13];
  return mtype;
}

void HPGP_Clean_Tx_Buffer(void) {
  /* fill the complete ethernet transmit buffer with 0x00 */
  int i;
  for (i=0; i<ETH_TRANSMIT_BUFFER_SIZE; i++) {
    eth_tx_buffer[i]=0;
  }
}

void HPGP_Set_NMK(uint8_t index) {
  /* sets the Network Membership Key (NMK) at a certain position in the transmit buffer */
  uint8_t i;
  for (i=0; i<16; i++) {
    eth_tx_buffer[index+i]=NMK[i]; // NMK
  }
}

void HPGP_Set_NID(uint8_t index) {
  /* copies the network ID (NID, 7 bytes) into the wished position in the transmit buffer */
  uint8_t i;
  for (i=0; i<7; i++) {
	  eth_tx_buffer[index+i]=NID[i];
  }
}

uint16_t HPGP_Get_MMTYPE(void) {
  /* calculates the MMTYPE (base value + lower two bits), see Table 11-2 of homeplug spec */
  return (eth_rx_buffer[16]<<8) + eth_rx_buffer[15];
}

void HPGP_Compose_Test_String(void){
	/* Generate a test string to test comunicate betweent 2 module */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(MAC_BROADCAST, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    /* Protocol */
    eth_tx_buffer[12]=0x88;
    eth_tx_buffer[13]=0xE1;
    eth_tx_buffer[14]=0xAA;
    eth_tx_buffer[15]=0xBB;
    eth_tx_buffer[16]=0xCC;
    eth_tx_buffer[17]=0xDD;
    eth_tx_buffer[18]=0xEE;
    eth_tx_buffer[19]=0xFF;

    eth_tx_buffer[51]=0x77;
    eth_tx_buffer[52]=0x88;
    eth_tx_buffer[53]=0x99;
    eth_tx_buffer[54]=0xAA;
    eth_tx_buffer[55]=0xBB;
    eth_tx_buffer[56]=0xCC;
    eth_tx_buffer[57]=0xDD;
    eth_tx_buffer[58]=0xEE;
    eth_tx_buffer[59]=0xFF;
}

void HPGP_Compose_Get_Sw_Req(void) {
	/* GET_SW.REQ request, as used by the win10 laptop */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(MAC_BROADCAST, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    /* Protocol */
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x00; // version
    eth_tx_buffer[15]=0x00; // GET_SW.REQ
    eth_tx_buffer[16]=0xA0; //
    eth_tx_buffer[17]=0x00; // Vendor OUI
    eth_tx_buffer[18]=0xB0; //
    eth_tx_buffer[19]=0x52; //
}

void HPGP_Compose_SLAC_PARM_REQ(void) {
	/* SLAC_PARM request, as it was recorded 2021-12-17 WP charger 2 */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(MAC_BROADCAST, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0x64; // SLAC_PARAM.REQ
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    eth_tx_buffer[19]=0x00; //
    eth_tx_buffer[20]=0x00; //
    HPGP_Fill_Address(myMAC, 21);; // 21 to 28: 8 bytes runid. The Ioniq uses the PEV mac plus 00 00.
    eth_tx_buffer[27]=0x00; //
    eth_tx_buffer[28]=0x00; //
    // rest is 00
}

void HPGP_Evaluate_SLAC_PARM_CNF(void) {
  /* As PEV, we receive the first response from the charger. */
	sprintf(serial_output_buffer, "[PEVSLAC] Checkpoint102: received SLAC_PARM.CNF");
	Serial_Print();
	checkpointNumber = 102;
	if (iAmPev) {
		if (pevSequenceState==STATE_WAITING_FOR_SLAC_PARAM_CNF) { //  we were waiting for the SlacParamCnf
			pevSequenceDelayCycles = 4; // original Ioniq is waiting 200ms
			HPGP_SLAC_Enter_State(STATE_SLAC_PARAM_CNF_RECEIVED); // enter next state. Will be handled in the cyclic runSlacSequencer
		}
	}
}

void HPGP_Compose_START_ATTEN_CHAR_IND(void){
    /* reference: see wireshark interpreted frame from ioniq */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(MAC_BROADCAST, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0x6A; // START_ATTEN_CHAR.IND
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    eth_tx_buffer[19]=0x00; // apptype
    eth_tx_buffer[20]=0x00; // sectype
    eth_tx_buffer[21]=0x0a; // number of sounds: 10
    eth_tx_buffer[22]=6; // timeout N*100ms. Normally 6, means in 600ms all sounds must have been tranmitted.
                            // Todo: As long we are a little bit slow, lets give 1000ms instead of 600, so that the
                            // charger is able to catch it all.
    eth_tx_buffer[23]=0x01; // response type
    HPGP_Fill_Address(myMAC, 24); // 24 to 29: sound_forwarding_sta, MAC of the PEV
    HPGP_Fill_Address(myMAC, 30); // 30 to 37: runid, filled with MAC of PEV and two bytes 00 00
    // rest is 00
}

void HPGP_Compose_MNBC_SOUND_IND(uint8_t sound){
    /* reference: see wireshark interpreted frame from Ioniq */
    uint8_t i;
    eth_tx_size = 71;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(MAC_BROADCAST, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);;
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0x76; // NMBC_SOUND.IND
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    eth_tx_buffer[19]=0x00; // apptype
    eth_tx_buffer[20]=0x00; // sectype
    eth_tx_buffer[21]=0x00; // 21 to 37 sender ID, all 00
//    eth_tx_buffer[38]=remainingNumberOfSounds; // countdown. Remaining number of sounds. Starts with 9 and counts down to 0.
    eth_tx_buffer[38] = sound;
    HPGP_Fill_Address(myMAC, 39); // 39 to 46: runid, filled with MAC of PEV and two bytes 00 00
    eth_tx_buffer[47]=0x00; // 47 to 54: reserved, all 00
    //55 to 70: random number. All 0xff in the ioniq message.
    for (i=55; i<71; i++) { // i in range(55, 71):
    	eth_tx_buffer[i]=0xFF;
    }
}

void HPGP_Evaluate_ATTEN_CHAR_IND(void){
	uint8_t i;
	if(iAmPev==1){
		sprintf(serial_output_buffer, "[PEVSLAC] received ATTEN_CHAR.IND");
		Serial_Print();
		if (pevSequenceState==STATE_WAIT_FOR_ATTEN_CHAR_IND) { // we were waiting for the AttenCharInd
			//todo: Handle the case when we receive multiple responses from different chargers.
			//      Wait a certain time, and compare the attenuation profiles. Decide for the nearest charger.
			//Take the MAC of the charger from the frame, and store it for later use.
			for (i=0; i<6; i++) {
				evseMac[i] = eth_rx_buffer[6+i]; // source MAC starts at offset 6
			}
			AttenCharIndNumberOfSounds = eth_rx_buffer[69];
			//addToTrace("[PEVSLAC] number of sounds reported by the EVSE (should be 10): " + str(AttenCharIndNumberOfSounds))
//			composeAttenCharRsp();
			HPGP_Compose_ATTEN_CHAR_RSP();
			checkpointNumber = 140;
	        SPI_Transmit_Receive();
	        pevSequenceState=STATE_ATTEN_CHAR_IND_RECEIVED; // enter next state. Will be handled in the cyclic runSlacSequencer
		}

	}
}

void HPGP_Compose_ATTEN_CHAR_RSP(void){
    /* reference: see wireshark interpreted frame from Ioniq */
    eth_tx_size = 70;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(evseMac, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0x6F; // ATTEN_CHAR.RSP
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    eth_tx_buffer[19]=0x00; // apptype
    eth_tx_buffer[20]=0x00; // sectype
    HPGP_Fill_Address(myMAC, 21); // 21 to 26: source MAC
    HPGP_Fill_Address(myMAC, 27); // 27 to 34: runid. The PEV mac, plus 00 00.
    // 35 to 51: source_id, all 00
    // 52 to 68: resp_id, all 00
    // 69: result. 0 is ok
}

void HPGP_Compose_SLAC_MATCH_REQ(void){
    /* reference: see wireshark interpreted frame from Ioniq */
    eth_tx_size = 85;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(evseMac, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0x7C; // SLAC_MATCH.REQ
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // 2 bytes fragmentation information. 0000 means: unfragmented.
    eth_tx_buffer[18]=0x00; //
    eth_tx_buffer[19]=0x00; // apptype
    eth_tx_buffer[20]=0x00; // sectype
    eth_tx_buffer[21]=0x3E; // 21 to 22: length
    eth_tx_buffer[22]=0x00; //
    // 23 to 39: pev_id, all 00
    HPGP_Fill_Address(myMAC, 40); // 40 to 45: PEV MAC
    // 46 to 62: evse_id, all 00
    HPGP_Fill_Address(evseMac, 63); // 63 to 68: EVSE MAC
    HPGP_Fill_Address(myMAC, 69); // 69 to 76: runid. The PEV mac, plus 00 00.
    // 77 to 84: reserved, all 00
}

void HPGP_Evaluate_SLAC_MATCH_CNF(void){
    uint8_t i;
    // The SLAC_MATCH.CNF contains the NMK and the NID.
    // We extract this information, so that we can use it for the CM_SET_KEY afterwards.
    // References: https://github.com/qca/open-plc-utils/blob/master/slac/evse_cm_slac_match.c
    // 2021-12-16_HPC_säule1_full_slac.pcapng
    if (iAmEvse==1) {
            // If we are EVSE, nothing to do. We have sent the match.CNF by our own.
            // The SET_KEY was already done at startup.
    }
    else {
    	sprintf(serial_output_buffer, "[PEVSLAC] received SLAC_MATCH.CNF");
        Serial_Print();
        for (i=0; i<7; i++) { // NID has 7 bytes
        	NID[i] = eth_rx_buffer[85+i];
        }
        for (i=0; i<16; i++)
        	NMK[i] = eth_rx_buffer[93+i];
        }
        sprintf(serial_output_buffer, "[PEVSLAC] From SlacMatchCnf, got network membership key (NMK) and NID");
        Serial_Print();
        // use the extracted NMK and NID to set the key in the adaptor:
        //composeSetKey();
        HPGP_Compose_SET_KEY_REQ();
        sprintf(serial_output_buffer, "[PEVSLAC] Checkpoint170: transmitting CM_SET_KEY.REQ");
        Serial_Print();
        checkpointNumber = 170;
        SPI_Transmit_Receive();
        if (pevSequenceState==STATE_WAITING_FOR_SLAC_MATCH_CNF) { // we were waiting for finishing the SLAC_MATCH.CNF and SET_KEY.REQ
        	HPGP_SLAC_Enter_State(STATE_WAITING_FOR_RESTART2);
        }
}

void HPGP_Compose_SET_KEY_REQ(void){
	/* CM_SET_KEY.REQ request */
  /* From example trace from catphish https://openinverter.org/forum/viewtopic.php?p=40558&sid=9c23d8c3842e95c4cf42173996803241#p40558
     Table 11-88 in the homeplug_av21_specification_final_public.pdf */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(evseMac, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    // Protocol
    eth_tx_buffer[12]=0x88; // Protocol HomeplugAV
    eth_tx_buffer[13]=0xE1; //
    eth_tx_buffer[14]=0x01; // version
    eth_tx_buffer[15]=0x08; // CM_SET_KEY.REQ
    eth_tx_buffer[16]=0x60; //
    eth_tx_buffer[17]=0x00; // frag_index
    eth_tx_buffer[18]=0x00; // frag_seqnum
    eth_tx_buffer[19]=0x01; // 0 key info type

    eth_tx_buffer[20]=0xaa; // 1 my nonce
    eth_tx_buffer[21]=0xaa; // 2
    eth_tx_buffer[22]=0xaa; // 3
    eth_tx_buffer[23]=0xaa; // 4

    eth_tx_buffer[24]=0x00; // 5 your nonce
    eth_tx_buffer[25]=0x00; // 6
    eth_tx_buffer[26]=0x00; // 7
    eth_tx_buffer[27]=0x00; // 8

    eth_tx_buffer[28]=0x04; // 9 nw info pid

    eth_tx_buffer[29]=0x00; // 10 info prn
    eth_tx_buffer[30]=0x00; // 11
    eth_tx_buffer[31]=0x00; // 12 pmn
    eth_tx_buffer[32]=0x00; // 13 cco cap
    HPGP_Set_NID(33);// 14-20 nid  7 bytes from 33 to 39
                //          Network ID to be associated with the key distributed herein.
                //          The 54 LSBs of this field contain the NID (refer to Section 3.4.3.1). The
                //          two MSBs shall be set to 0b00.
    eth_tx_buffer[40]=0x01; // 21 peks (payload encryption key select) Table 11-83. 01 is NMK. We had 02 here, why???
                               // with 0x0F we could choose "no key, payload is sent in the clear"
  	HPGP_Set_NMK(41);
  	#define variation 0
  	eth_tx_buffer[41]+=variation; // to try different NMKs
  	// and three remaining zeros
}

void HPGP_Evaluate_SET_KEY_CNF(void){
    // The Setkey confirmation
    uint8_t result;
    // In spec, the result 0 means "success". But in reality, the 0 means: did not work. When it works,
    // then the LEDs are blinking (device is restarting), and the response is 1.
    sprintf(serial_output_buffer, "[PEVSLAC] received SET_KEY.CNF");
    Serial_Print();
    result = eth_rx_buffer[19];
    if (result == 0) {
        sprintf(serial_output_buffer, "[PEVSLAC] SetKeyCnf says 0, this would be a bad sign for local modem, but normal for remote");
        Serial_Print();
    }
    else {
    	sprintf(serial_output_buffer, "[PEVSLAC] SetKeyCnf says %d, this is formally 'rejected', but indeed ok" , result);
    	Serial_Print();
    }
}

void HPGP_Evaluate_Get_Sw_Cnf(void){
	uint8_t i, x;
	//char strMac[20];
    sprintf(serial_output_buffer, "[PEVSLAC] received GET_SW.CNF ");
    Serial_Print();
    numberOfSoftwareVersionResponses+=1;
    for (i=0; i<6; i++) {
        sourceMac[i] = eth_rx_buffer[6+i];
    }
    verLen = eth_rx_buffer[22];
    if ((verLen>0) && (verLen<0x30)) {
    	for (i=0; i<verLen; i++) {
            x = eth_rx_buffer[23+i];
            if (x<0x20) { x=0x20; } /* make unprintable character to space. */
            strVersion[i]=x;
    	}
    	strVersion[i] = 0;
    	//addToTrace(strMac);
    	sprintf(serial_output_buffer, "software version %s ", strVersion);
    	Serial_Print();
    }
}



/*My additional code*/
void HPGP_Compose_Exchange_Data_Req(void){
    /* reference: see wireshark interpreted frame from Ioniq */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(evseMac, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    // Protocol
    EV_Compose_Start_Communication_Req();
    memcpy(&eth_tx_buffer[12], data_exchange, exchange_data_size);
}

void HPGP_Compose_Exchange_Data_Cnf(void){
    /* reference: see wireshark interpreted frame from Ioniq */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(evseMac, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    // Protocol
    EV_Compose_Start_Communication_Cnf();
    memcpy(&eth_tx_buffer[12], data_exchange, exchange_data_size);
}

void HPGP_Evaluate_Exchange_Data_Req(void){
	sprintf(serial_output_buffer, "[PEVSLAC] received EXCHANGE_DATA.REQ");
    Serial_Print();
    EV_Evaluate_Start_Communication_Req();

    HPGP_Compose_Exchange_Data_Cnf();
    SPI_QCA7000_Send_Eth_Frame();
}

void HPGP_Evaluate_Exchange_Data_Cnf(void){
	sprintf(serial_output_buffer, "[PEVSLAC] received EXCHANGE_DATA.CNF");
    Serial_Print();
    EV_Evaluate_Start_Communication_Cnf();
}

void HPGP_Compose_Setup_Done_Req(void){
    /* reference: see wireshark interpreted frame from Ioniq */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(evseMac, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    // Protocol
    EV_Compose_Setup_Done_Req();
    memcpy(&eth_tx_buffer[12], data_exchange, exchange_data_size);
}

void HPGP_Compose_Setup_Done_Cnf(void){
    /* reference: see wireshark interpreted frame from Ioniq */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(evseMac, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    // Protocol
    EV_Compose_Setup_Done_Cnf();
    memcpy(&eth_tx_buffer[12], data_exchange, exchange_data_size);
}

void HPGP_Evaluate_Setup_Done_Req(void){
	sprintf(serial_output_buffer, "[PEVSLAC] received SETUP_DONE.REQ");
    Serial_Print();

    HPGP_Compose_Setup_Done_Cnf();
    SPI_QCA7000_Send_Eth_Frame();
}

void HPGP_Evaluate_Setup_Done_Cnf(void){
	sprintf(serial_output_buffer, "[PEVSLAC] received SETUP_DONE.CNF");
    Serial_Print();
}

void HPGP_Compose_Start_Charging_Req(void){
    /* reference: see wireshark interpreted frame from Ioniq */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(evseMac, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    // Protocol
    EV_Compose_Start_Charging_Req();
    memcpy(&eth_tx_buffer[12], data_exchange, exchange_data_size);
}

void HPGP_Compose_Start_Charging_Cnf(void){
    /* reference: see wireshark interpreted frame from Ioniq */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(evseMac, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    // Protocol
    EV_Compose_Start_Charging_Cnf();
    memcpy(&eth_tx_buffer[12], data_exchange, exchange_data_size);
}

void HPGP_Evaluate_Start_Charging_Req(void){
	sprintf(serial_output_buffer, "[PEVSLAC] received START_CHARGING.REQ");
    Serial_Print();

    HPGP_Compose_Start_Charging_Cnf();
    SPI_QCA7000_Send_Eth_Frame();
}

void HPGP_Evaluate_Start_Charging_Cnf(void){
	sprintf(serial_output_buffer, "[PEVSLAC] received START_CHARGING.CNF");
    Serial_Print();
}


void HPGP_Compose_Stop_Charging_Req(void){
    /* reference: see wireshark interpreted frame from Ioniq */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(evseMac, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    // Protocol
    EV_Compose_Stop_Charging_Req();
    memcpy(&eth_tx_buffer[12], data_exchange, exchange_data_size);
}

void HPGP_Compose_Stop_Charging_Cnf(void){
    /* reference: see wireshark interpreted frame from Ioniq */
    eth_tx_size = 60;
    HPGP_Clean_Tx_Buffer();
    /* Destination MAC */
    HPGP_Fill_Address(evseMac, 0);
    /* Source MAC */
    HPGP_Fill_Address(myMAC, 6);
    // Protocol
    EV_Compose_Stop_Charging_Cnf();
    memcpy(&eth_tx_buffer[12], data_exchange, exchange_data_size);
}

void HPGP_Evaluate_Stop_Charging_Req(void){
	sprintf(serial_output_buffer, "[PEVSLAC] received STOP_CHARGING.REQ");
    Serial_Print();

    HPGP_Compose_Stop_Charging_Cnf();
    SPI_QCA7000_Send_Eth_Frame();
}

void HPGP_Evaluate_Stop_Charging_Cnf(void){
	sprintf(serial_output_buffer, "[PEVSLAC] received STOP_CHARGING.CNF");
    Serial_Print();
}

/*---------------------------------------------------*/


uint8_t HPGP_EVSE_Modem_Found(void){
	/* todo: look whether the MAC of the EVSE modem is in the list of detected modems */
	/* as simple solution we say: If we see two modems, then it should be one
    local in the car, and one in the charger. */
	return numberOfFoundModems>1;
}

void HPGP_SLAC_Enter_State(int n){
	sprintf(serial_output_buffer, "[PEVSLAC] from %d entering %d", pevSequenceState, n);
	Serial_Print();
	pevSequenceState = n;
	pevSequenceCyclesInState = 0;
}

int HPGP_Too_Long(void){
	/* The timeout handling function. */
	return (pevSequenceCyclesInState > 500);
}

void HPGP_Run_SLAC_Sequencer(void){
	pevSequenceCyclesInState++;
    if (pevSequenceState == STATE_INITIAL)   {
        /* The modem is present, starting SLAC. */
        HPGP_SLAC_Enter_State(STATE_READY_FOR_SLAC);
        return;
    }
    if (pevSequenceState==STATE_READY_FOR_SLAC) {
        sprintf(serial_output_buffer, "[PEVSLAC] Checkpoint100: Sending SLAC_PARAM.REQ...");
        Serial_Print();
        checkpointNumber = 100;
        HPGP_Compose_SLAC_PARM_REQ();
        SPI_Transmit_Receive();
        HPGP_SLAC_Enter_State(STATE_WAITING_FOR_SLAC_PARAM_CNF);
        return;
	}
    if (pevSequenceState==STATE_WAITING_FOR_SLAC_PARAM_CNF) { // Waiting for slac_param confirmation.
        if (pevSequenceCyclesInState>=33) {
        	// No response for 1s, this is an error.
            sprintf(serial_output_buffer, "[PEVSLAC] Timeout while waiting for SLAC_PARAM.CNF");
            Serial_Print();
            HPGP_SLAC_Enter_State(STATE_INITIAL);
        }
        // (the normal state transition is done in the reception handler)
        return;
	}
    if (pevSequenceState==STATE_SLAC_PARAM_CNF_RECEIVED) { // slac_param confirmation was received.
    	pevSequenceDelayCycles = 1; //  1*30=30ms as preparation for the next state.
    	//  Between the SLAC_PARAM.CNF and the first START_ATTEN_CHAR.IND the Ioniq waits 100ms.
    	//  The allowed time TP_match_sequence is 0 to 100ms.
    	//  Alpitronic and ABB chargers are more tolerant, they worked with a delay of approx
    	//  250ms. In contrast, Supercharger and Compleo do not respond anymore if we
    	//  wait so long.
    	nRemainingStartAttenChar = 3; // There shall be 3 START_ATTEN_CHAR messages.
    	HPGP_SLAC_Enter_State(STATE_BEFORE_START_ATTEN_CHAR);
    	return;
    }
    if (pevSequenceState==STATE_BEFORE_START_ATTEN_CHAR) { // received SLAC_PARAM.CNF. Multiple transmissions of START_ATTEN_CHAR.
    	if (pevSequenceDelayCycles>0) {
        pevSequenceDelayCycles-=1;
        return;
    	}
		// The delay time is over. Let's transmit.
		if (nRemainingStartAttenChar>0) {
			nRemainingStartAttenChar-=1;
			HPGP_Compose_START_ATTEN_CHAR_IND();
			sprintf(serial_output_buffer, "[PEVSLAC] transmitting START_ATTEN_CHAR.IND...");
			Serial_Print();
			SPI_Transmit_Receive();
			pevSequenceDelayCycles = 0; // original from ioniq is 20ms between the START_ATTEN_CHAR. Shall be 20ms to 50ms. So we set to 0 and the normal 30ms call cycle is perfect.
			return;
		}
		else {
			// all three START_ATTEN_CHAR.IND are finished. Now we send 10 MNBC_SOUND.IND
			pevSequenceDelayCycles = 0; // original from ioniq is 40ms after the last START_ATTEN_CHAR.IND.
										// Shall be 20ms to 50ms. So we set to 0 and the normal 30ms call cycle is perfect.
			remainingNumberOfSounds = 10; // We shall transmit 10 sound messages.
			HPGP_SLAC_Enter_State(STATE_SOUNDING);
		}
		return;
	}
    if (pevSequenceState==STATE_SOUNDING) { // Multiple transmissions of MNBC_SOUND.IND.
        if (pevSequenceDelayCycles>0) {
        	pevSequenceDelayCycles-=1;
            return;
        }
        if (remainingNumberOfSounds>0) {
            remainingNumberOfSounds-=1;
//            HPGP_Compose_MNBC_SOUND_IND();
            sprintf(serial_output_buffer, "[PEVSLAC] transmitting MNBC_SOUND.IND..."); // original from ioniq is 40ms after the last START_ATTEN_CHAR.IND
            Serial_Print();
            checkpointNumber = 104;
            SPI_Transmit_Receive();
            if (remainingNumberOfSounds==0) {
            	HPGP_SLAC_Enter_State(STATE_WAIT_FOR_ATTEN_CHAR_IND); // move fast to the next state, so that a fast response is catched in the correct state
    		}
            pevSequenceDelayCycles = 0; // original from ioniq is 20ms between the messages.
            							// Shall be 20ms to 50ms. So we set to 0 and the normal 30ms call cycle is perfect.
            return;
        }
    }
    if (pevSequenceState==STATE_WAIT_FOR_ATTEN_CHAR_IND) { // waiting for ATTEN_CHAR.IND
    	// todo: it is possible that we receive this message from multiple chargers. We need
    	// to select the charger with the loudest reported signals.
        if (HPGP_Too_Long()) {
        	HPGP_SLAC_Enter_State(STATE_INITIAL);
    	}
        return;
        // (the normal state transition is done in the reception handler)
    }
    if (pevSequenceState==STATE_ATTEN_CHAR_IND_RECEIVED) { // ATTEN_CHAR.IND was received and the
                                                           // nearest charger decided and the
    													   // ATTEN_CHAR.RSP was sent.
        HPGP_SLAC_Enter_State(STATE_DELAY_BEFORE_MATCH);
        pevSequenceDelayCycles = 30; // original from ioniq is 860ms to 980ms from ATTEN_CHAR.RSP to SLAC_MATCH.REQ
        return;
    }
    if (pevSequenceState==STATE_DELAY_BEFORE_MATCH) { // Waiting time before SLAC_MATCH.REQ
        if (pevSequenceDelayCycles>0) {
        	pevSequenceDelayCycles-=1;
            return;
        }
        HPGP_Compose_SLAC_MATCH_REQ();
        sprintf(serial_output_buffer, "[PEVSLAC] Checkpoint150: transmitting SLAC_MATCH.REQ...");
        Serial_Print();
        checkpointNumber = 150;
        SPI_Transmit_Receive();
        HPGP_SLAC_Enter_State(STATE_WAITING_FOR_SLAC_MATCH_CNF);
        return;
    }
    if (pevSequenceState==STATE_WAITING_FOR_SLAC_MATCH_CNF) { // waiting for SLAC_MATCH.CNF
    	if (HPGP_Too_Long()) {
    		HPGP_SLAC_Enter_State(STATE_INITIAL);
            return;
        }
        pevSequenceDelayCycles = 100; // 3s reset wait time (may be a little bit too short, need a retry)
        // (the normal state transition is done in the receive handler of SLAC_MATCH.CNF,
        // including the transmission of SET_KEY.REQ)
        return;
    }
    if (pevSequenceState==STATE_WAITING_FOR_RESTART2) { // SLAC is finished, SET_KEY.REQ was
                                                            // transmitted. The homeplug modem makes
                                                            // the reset and we need to wait until it
                                                            // is up with the new key.
    	if (pevSequenceDelayCycles>0) {
            pevSequenceDelayCycles-=1;
            return;
        }
        sprintf(serial_output_buffer, "[PEVSLAC] Checking whether the pairing worked, by GET_KEY.REQ...");
        Serial_Print();
        numberOfFoundModems = 0; // reset the number, we want to count the modems newly.
        nEvseModemMissingCounter=0; // reset the retry counter
//        composeGetKey();
//        myEthTransmit();
        HPGP_SLAC_Enter_State(STATE_FIND_MODEMS2);
        return;
    }
    if (pevSequenceState==STATE_FIND_MODEMS2) { // Waiting for the modems to answer.
        if (pevSequenceCyclesInState>=10) { //
            // It was sufficient time to get the answers from the modems.
            sprintf(serial_output_buffer, "[PEVSLAC] It was sufficient time to get the answers from the modems.");
            Serial_Print();
            // Let's see what we received.
            if (!HPGP_EVSE_Modem_Found()) {
            	nEvseModemMissingCounter+=1;
                sprintf(serial_output_buffer, "[PEVSLAC] No EVSE seen (yet). Still waiting for it.");
                Serial_Print();
                // At the Alpitronic we measured, that it takes 7s between the SlacMatchResponse and
                // the chargers modem reacts to GetKeyRequest. So we should wait here at least 10s.
            if (nEvseModemMissingCounter>20) {
                // We lost the connection to the EVSE modem. Back to the beginning.
                sprintf(serial_output_buffer, "[PEVSLAC] We lost the connection to the EVSE modem. Back to the beginning.");
                Serial_Print();
                HPGP_SLAC_Enter_State(STATE_INITIAL);
                return;
            }
            // The EVSE modem is (shortly) not seen. Ask again.
            pevSequenceDelayCycles=30;
            HPGP_SLAC_Enter_State(STATE_WAITING_FOR_RESTART2);
            return;
            }
            // The EVSE modem is present (or we are simulating)
            sprintf(serial_output_buffer, "[PEVSLAC] EVSE is up, pairing successful.");
            Serial_Print();
            nEvseModemMissingCounter=0;
            //connMgr_ModemFinderOk(2); /* Two modems were found. */
            /* This is the end of the SLAC. */
            /* The AVLN is established, we have at least two modems in the network. */
            HPGP_SLAC_Enter_State(STATE_INITIAL);
        }
        return;
    }
    // invalid state is reached. As robustness measure, go to initial state.
    sprintf(serial_output_buffer, "[PEVSLAC] ERROR: Invalid state reached");
    Serial_Print();
    HPGP_SLAC_Enter_State(STATE_INITIAL);
}


void HPGP_Evaluate_HomePlug_Packet(void){
	  switch (HPGP_Get_MMTYPE()) {
//	    case CM_GET_KEY + MMTYPE_CNF:    evaluateGetKeyCnf();    break;
	    case CM_SLAC_MATCH + MMTYPE_CNF: HPGP_Evaluate_SLAC_MATCH_CNF(); break;
	    case CM_SLAC_PARAM + MMTYPE_CNF: HPGP_Evaluate_SLAC_PARM_CNF(); break;
	    case CM_ATTEN_CHAR + MMTYPE_IND: HPGP_Evaluate_ATTEN_CHAR_IND(); break;
	    case CM_SET_KEY + MMTYPE_CNF:    HPGP_Evaluate_SET_KEY_CNF();    break;
	    case CM_GET_SW + MMTYPE_CNF:     HPGP_Evaluate_Get_Sw_Cnf();     break;

	    case CM_EXCHANGE_DATA + MMTYPE_REQ: HPGP_Evaluate_Exchange_Data_Req();	break;
	    case CM_EXCHANGE_DATA + MMTYPE_CNF: HPGP_Evaluate_Exchange_Data_Cnf();	break;

	    case CM_SETUP_DONE + MMTYPE_REQ: HPGP_Evaluate_Setup_Done_Req();	break;
	    case CM_SETUP_DONE + MMTYPE_CNF: HPGP_Evaluate_Setup_Done_Cnf();	break;

	    case CM_START_CHARGING + MMTYPE_REQ: HPGP_Evaluate_Start_Charging_Req();	break;
	    case CM_START_CHARGING + MMTYPE_CNF: HPGP_Evaluate_Start_Charging_Cnf();	break;

	    case CM_STOP_CHARGING + MMTYPE_REQ: HPGP_Evaluate_Stop_Charging_Req();	break;
	    case CM_STOP_CHARGING + MMTYPE_CNF: HPGP_Evaluate_Stop_Charging_Cnf();	break;
	  }
}




