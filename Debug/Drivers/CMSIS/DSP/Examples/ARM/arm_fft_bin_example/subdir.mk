################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/DSP/Examples/ARM/arm_fft_bin_example/arm_fft_bin_data.c \
../Drivers/CMSIS/DSP/Examples/ARM/arm_fft_bin_example/arm_fft_bin_example_f32.c 

OBJS += \
./Drivers/CMSIS/DSP/Examples/ARM/arm_fft_bin_example/arm_fft_bin_data.o \
./Drivers/CMSIS/DSP/Examples/ARM/arm_fft_bin_example/arm_fft_bin_example_f32.o 

C_DEPS += \
./Drivers/CMSIS/DSP/Examples/ARM/arm_fft_bin_example/arm_fft_bin_data.d \
./Drivers/CMSIS/DSP/Examples/ARM/arm_fft_bin_example/arm_fft_bin_example_f32.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/DSP/Examples/ARM/arm_fft_bin_example/%.o: ../Drivers/CMSIS/DSP/Examples/ARM/arm_fft_bin_example/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g -DSTM32H743xx -DARM_MATH_CM7 -DUSE_HAL_DRIVER -I"/mnt/Data/Documents/workspace_Pacabot/NARA/Inc" -I"/mnt/Data/Documents/workspace_Pacabot/NARA/Drivers/CMSIS/DSP/Include" -I"/mnt/Data/Documents/workspace_Pacabot/NARA/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"/mnt/Data/Documents/workspace_Pacabot/NARA/Drivers/STM32H7xx_HAL_Driver/Inc" -I"/mnt/Data/Documents/workspace_Pacabot/NARA/Drivers/CMSIS/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


