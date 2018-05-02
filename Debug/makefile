################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include startup/subdir.mk
-include Src/peripherals/tone/subdir.mk
-include Src/peripherals/times_base/subdir.mk
-include Src/peripherals/motors/subdir.mk
-include Src/peripherals/encoders/subdir.mk
-include Src/peripherals/callback/subdir.mk
-include Src/peripherals/bluetooth/subdir.mk
-include Src/middleware/moves/basicMoves/subdir.mk
-include Src/middleware/controls/pidController/subdir.mk
-include Src/middleware/controls/mainControl/subdir.mk
-include Src/application/statistiques/subdir.mk
-include Src/subdir.mk
-include Drivers/STM32H7xx_HAL_Driver/Src/subdir.mk
-include Drivers/CMSIS/DSP/Source/ControllerFunctions/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
NARA.hex \

SECONDARY_SIZE += \
NARA.siz \


# All Target
all: NARA.elf secondary-outputs

# Tool invocations
NARA.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM Cross C Linker'
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g -T "/mnt/Data/Documents/workspace_Pacabot/NARA/Linker_Scripts/STM32H743VITx_FLASH.ld" -Xlinker --gc-sections -Wl,-Map,"NARA.map" --specs=nano.specs -u _printf_float -o "NARA.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

NARA.hex: NARA.elf
	@echo 'Invoking: GNU ARM Cross Create Flash Image'
	arm-none-eabi-objcopy -O ihex "NARA.elf"  "NARA.hex"
	@echo 'Finished building: $@'
	@echo ' '

NARA.siz: NARA.elf
	@echo 'Invoking: GNU ARM Cross Print Size'
	arm-none-eabi-size --format=berkeley "NARA.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_DEPS)$(S_UPPER_DEPS)$(C_DEPS) NARA.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
