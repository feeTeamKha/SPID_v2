################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../user/CRC8_CRC16.c \
../user/Flash_Func.c \
../user/IRQ.c \
../user/PID.c \
../user/USB_App.c \
../user/flash.c 

OBJS += \
./user/CRC8_CRC16.o \
./user/Flash_Func.o \
./user/IRQ.o \
./user/PID.o \
./user/USB_App.o \
./user/flash.o 

C_DEPS += \
./user/CRC8_CRC16.d \
./user/Flash_Func.d \
./user/IRQ.d \
./user/PID.d \
./user/USB_App.d \
./user/flash.d 


# Each subdirectory must supply rules for building sources it contributes
user/%.o user/%.su user/%.cyclo: ../user/%.c user/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/ROBOCON_SP/PID_By_Kha/PID_F103_RTOS/user" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-user

clean-user:
	-$(RM) ./user/CRC8_CRC16.cyclo ./user/CRC8_CRC16.d ./user/CRC8_CRC16.o ./user/CRC8_CRC16.su ./user/Flash_Func.cyclo ./user/Flash_Func.d ./user/Flash_Func.o ./user/Flash_Func.su ./user/IRQ.cyclo ./user/IRQ.d ./user/IRQ.o ./user/IRQ.su ./user/PID.cyclo ./user/PID.d ./user/PID.o ./user/PID.su ./user/USB_App.cyclo ./user/USB_App.d ./user/USB_App.o ./user/USB_App.su ./user/flash.cyclo ./user/flash.d ./user/flash.o ./user/flash.su

.PHONY: clean-user

