################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/QCA7000.c \
../Libraries/exchange_data.c \
../Libraries/hmi.c \
../Libraries/homeplug.c \
../Libraries/homeplug_evse.c \
../Libraries/sequence_function.c 

OBJS += \
./Libraries/QCA7000.o \
./Libraries/exchange_data.o \
./Libraries/hmi.o \
./Libraries/homeplug.o \
./Libraries/homeplug_evse.o \
./Libraries/sequence_function.o 

C_DEPS += \
./Libraries/QCA7000.d \
./Libraries/exchange_data.d \
./Libraries/hmi.d \
./Libraries/homeplug.d \
./Libraries/homeplug_evse.d \
./Libraries/sequence_function.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/%.o Libraries/%.su Libraries/%.cyclo: ../Libraries/%.c Libraries/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I"D:/App/STM32/CubeIDE/Project/G474_Master_20_11_2023/Libraries" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Libraries

clean-Libraries:
	-$(RM) ./Libraries/QCA7000.cyclo ./Libraries/QCA7000.d ./Libraries/QCA7000.o ./Libraries/QCA7000.su ./Libraries/exchange_data.cyclo ./Libraries/exchange_data.d ./Libraries/exchange_data.o ./Libraries/exchange_data.su ./Libraries/hmi.cyclo ./Libraries/hmi.d ./Libraries/hmi.o ./Libraries/hmi.su ./Libraries/homeplug.cyclo ./Libraries/homeplug.d ./Libraries/homeplug.o ./Libraries/homeplug.su ./Libraries/homeplug_evse.cyclo ./Libraries/homeplug_evse.d ./Libraries/homeplug_evse.o ./Libraries/homeplug_evse.su ./Libraries/sequence_function.cyclo ./Libraries/sequence_function.d ./Libraries/sequence_function.o ./Libraries/sequence_function.su

.PHONY: clean-Libraries

