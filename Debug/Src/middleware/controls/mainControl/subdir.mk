################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/middleware/controls/mainControl/mainControl.c \
../Src/middleware/controls/mainControl/positionControl.c \
../Src/middleware/controls/mainControl/speedControl.c \
../Src/middleware/controls/mainControl/transfertFunction.c 

OBJS += \
./Src/middleware/controls/mainControl/mainControl.o \
./Src/middleware/controls/mainControl/positionControl.o \
./Src/middleware/controls/mainControl/speedControl.o \
./Src/middleware/controls/mainControl/transfertFunction.o 

C_DEPS += \
./Src/middleware/controls/mainControl/mainControl.d \
./Src/middleware/controls/mainControl/positionControl.d \
./Src/middleware/controls/mainControl/speedControl.d \
./Src/middleware/controls/mainControl/transfertFunction.d 


# Each subdirectory must supply rules for building sources it contributes
Src/middleware/controls/mainControl/%.o: ../Src/middleware/controls/mainControl/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g -DSTM32H743xx -DARM_MATH_CM7 -DUSE_HAL_DRIVER -I"/mnt/Data/Documents/workspace_Pacabot/NARA/Inc" -I"/mnt/Data/Documents/workspace_Pacabot/NARA/Drivers/CMSIS/DSP/Include" -I"/mnt/Data/Documents/workspace_Pacabot/NARA/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"/mnt/Data/Documents/workspace_Pacabot/NARA/Drivers/STM32H7xx_HAL_Driver/Inc" -I"/mnt/Data/Documents/workspace_Pacabot/NARA/Drivers/CMSIS/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


