################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include code/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
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
central_peripheral.hex \

SECONDARY_SIZE += \
central_peripheral.siz \


# All Target
all: central_peripheral.elf secondary-outputs

dependents:
	-cd C:/Users/fg7fww/_Install/App/DemoBoards/RSL10/workspace_r1.3.4_test/central_peripheral/Release && $(MAKE) all

# Tool invocations
central_peripheral.elf: $(OBJS) $(USER_OBJS) C:/Users/fg7fww/_Install/App/DemoBoards/RSL10/workspace_r1.3.4_test/central_peripheral/Release/central_peripheral.elf
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C Linker'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -T "C:\Program Files (x86)\ON Semiconductor\RSL10 EDK\eclipse\..\configuration\sections.ld" -nostartfiles -Xlinker --gc-sections -L"C:\Program Files (x86)\ON Semiconductor\RSL10 EDK\eclipse\..\lib\release" -L"C:\Program Files (x86)\ON Semiconductor\RSL10 EDK\eclipse\..\lib\ble_core\release" -L"C:\Program Files (x86)\ON Semiconductor\RSL10 EDK\eclipse\..\lib\ble_profiles\release" -Wl,-Map,"central_peripheral.map" --specs=nano.specs -o "central_peripheral.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

central_peripheral.hex: central_peripheral.elf
	@echo 'Invoking: Cross ARM GNU Create Flash Image'
	arm-none-eabi-objcopy -O ihex "central_peripheral.elf"  "central_peripheral.hex"
	@echo 'Finished building: $@'
	@echo ' '

central_peripheral.siz: central_peripheral.elf
	@echo 'Invoking: Cross ARM GNU Print Size'
	arm-none-eabi-size --format=berkeley "central_peripheral.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS) central_peripheral.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY:
C:/Users/fg7fww/_Install/App/DemoBoards/RSL10/workspace_r1.3.4_test/central_peripheral/Release/central_peripheral.elf:

-include ../makefile.targets
