/*
 * MyFLASH.h
 *
 *  Created on: Oct 24, 2023
 *      Author: Admin
 */

#ifndef INC_MYFLASH_H_
#define INC_MYFLASH_H_



#endif /* INC_MYFLASH_H_ */

#include "stm32g4xx_hal.h"


#define FLASH_BANK1_START_ADDR 0x08000000
#define FLASH_BANK2_START_ADDR 0x08040000

void eraseFlash(uint32_t Page, uint32_t bank);

void writeFlash(uint64_t* data, uint32_t size, uint32_t page, uint32_t bank);

void readFlash(uint64_t* data, uint32_t size, uint32_t page, uint32_t bank);

void uint64_array_to_uint8_array_big_endian(const uint64_t* input, size_t input_length, uint8_t* output);

void uint8_array_to_uint64_array_big_endian(const uint8_t* input, size_t input_length, uint64_t* output);
