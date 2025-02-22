################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../user/CRC8_CRC16.c \
../user/IRQ.c \
../user/PID.c 

OBJS += \
./user/CRC8_CRC16.o \
./user/IRQ.o \
./user/PID.o 

C_DEPS += \
./user/CRC8_CRC16.d \
./user/IRQ.d \
./user/PID.d 


# Each subdirectory must supply rules for building sources it contributes
user/%.o user/%.su user/%.cyclo: ../user/%.c user/subdir.mk
	arm-none-eabi-gcc -gdwarf-4 "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I"D:/ROBOCON_SP/PID_By_Kha/PID_F103/user" -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-user

clean-user:
	-$(RM) ./user/CRC8_CRC16.cyclo ./user/CRC8_CRC16.d ./user/CRC8_CRC16.o ./user/CRC8_CRC16.su ./user/IRQ.cyclo ./user/IRQ.d ./user/IRQ.o ./user/IRQ.su ./user/PID.cyclo ./user/PID.d ./user/PID.o ./user/PID.su

.PHONY: clean-user

