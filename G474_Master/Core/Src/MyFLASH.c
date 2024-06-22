/*
 * MyFLASH.c
 *
 *  Created on: Oct 24, 2023
 *      Author: Admin
 */

#include "MyFLASH.h"






// This function erases a specific page in Flash memory.
// Parameters:
//   - Page: The page number in Flash memory to be erased.
//   - bank: The Flash bank (FLASH_BANK_1 or FLASH_BANK_2) where the page is located.
void eraseFlash(uint32_t Page, uint32_t bank)
{
	HAL_StatusTypeDef status;
	uint32_t PageError;
	HAL_FLASH_Unlock();

	 // Xóa trang
	  FLASH_EraseInitTypeDef erase_init;
	  erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
	  erase_init.Page = Page; // Điều chỉnh trang dựa trên nhu cầu của bạn
	  erase_init.NbPages = 1;
	  erase_init.Banks=bank;

	  status = HAL_FLASHEx_Erase(&erase_init, &PageError);
	   if (status != HAL_OK)
	   {
	     // Xử lý lỗi khi xóa
	     HAL_FLASH_Lock();
	     return;
	   }

	HAL_FLASH_Lock();
}




// This function writes data to Flash memory.
// Parameters:
//   - data: An array of uint64_t containing the data to be written.
//   - size: The number of uint64_t values to write from the data array.
//   - page: The page number in Flash memory where data will be written.
//   - bank: The Flash bank (FLASH_BANK_1 or FLASH_BANK_2) where data will be written.
void writeFlash(uint64_t* data, uint32_t size, uint32_t page, uint32_t bank)
{
  HAL_StatusTypeDef status;
  uint32_t PageError;

  HAL_FLASH_Unlock();

  // Xóa trang trước khi ghi
  FLASH_EraseInitTypeDef erase_init;
  erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
  //erase_init.TypeErase = FLASH_TYPEERASE_MASSERASE;
  erase_init.Page = 127; // Điều chỉnh trang dựa trên nhu cầu của bạn
  erase_init.NbPages = 1;
  erase_init.Banks=bank;

  status = HAL_FLASHEx_Erase(&erase_init, &PageError);
  if (status != HAL_OK)
  {
    // Xử lý lỗi khi xóa
    HAL_FLASH_Lock();
    return;
  }

  // Ghi dữ liệu v??��?��?o flash
  uint32_t start_add;
  if(bank==FLASH_BANK_1) start_add=FLASH_BANK1_START_ADDR;
  if(bank==FLASH_BANK_2) start_add=FLASH_BANK2_START_ADDR;

  for (uint32_t i = 0; i < size; i++)
  {
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, start_add+ page*2048 + (i * 8), data[i]);
    if (status != HAL_OK)
    {
      // Xử lý lỗi khi ghi;
    	HAL_FLASH_Lock();
    }
  }

  HAL_FLASH_Lock();
}

// This function reads data from Flash memory and stores it in an array of uint64_t.
// Parameters:
//   - data: An array of uint64_t to store the read data.
//   - size: The number of uint64_t values to read and store in the data array.
//   - page: The page number in Flash memory from which to read data.
//   - bank: The Flash bank (FLASH_BANK_1 or FLASH_BANK_2) that contains the data.
void readFlash(uint64_t* data, uint32_t size, uint32_t page, uint32_t bank)
{
	  uint32_t start_add;
	  if(bank==FLASH_BANK_1) start_add=FLASH_BANK1_START_ADDR;
	  if(bank==FLASH_BANK_2) start_add=FLASH_BANK2_START_ADDR;
  for (uint32_t i = 0; i < size; i++) {
    data[i] = *(__IO uint64_t*)(start_add + page*2048+ (i * 8));
  }
}


// This function converts an array of uint64_t to an array of uint8_t in big-endian format.
// Parameters:
//   - input: An array of uint64_t containing the data to be converted.
//   - input_length: The number of uint64_t values in the input array.
//   - output: An array of uint8_t where the converted data will be stored.
void uint64_array_to_uint8_array_big_endian(const uint64_t* input, size_t input_length, uint8_t* output)
{
    for (size_t i = 0; i < input_length; i++) {
        uint64_t value = input[i];
        for (int j = 7; j >= 0; j--) {
            output[i * 8 + (7 - j)] = (uint8_t)((value >> (j * 8)) & 0xFF);
        }
    }
}

// This function converts an array of uint8_t in big-endian format to an array of uint64_t.
// Parameters:
//   - input: An array of uint8_t containing the data to be converted.
//   - input_length: The number of uint64_t values to be converted and stored in the output array.
//   - output: An array of uint64_t where the converted data will be stored.
void uint8_array_to_uint64_array_big_endian(const uint8_t* input, size_t input_length, uint64_t* output)
{
    for (size_t i = 0; i < input_length; i++) {
        uint64_t value = 0;
        for (int j = 0; j < 8; j++) {
            // Construct the uint64_t value by shifting and combining the bytes.
            value |= ((uint64_t)input[i * 8 + j] << ((7 - j) * 8));
        }
        output[i] = value;
    }
}
