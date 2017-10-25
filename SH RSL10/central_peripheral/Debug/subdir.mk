################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app.c 

OBJS += \
./app.o 

C_DEPS += \
./app.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -DRSL10_CID=101 -DCFG_BLE -DCFG_ALLROLES -DCFG_APP -DCFG_ATTC -DCFG_ATTS -DCFG_CON=4 -DCFG_EMB -DCFG_HOST -DCFG_RF_ATLAS -DCFG_ALLPRF -DCFG_PRF -DCFG_NB_PRF=2 -DCFG_CHNL_ASSESS -DCFG_SEC_CON -DCFG_EXT_DB -DCFG_PRF_BASC -DCFG_PRF_BASS -I"C:\Program Files (x86)\ON Semiconductor\RSL10 EDK oxigen\eclipse\..\include\bb" -I"C:\Program Files (x86)\ON Semiconductor\RSL10 EDK oxigen\eclipse\..\include\ble\profiles" -I"C:\Program Files (x86)\ON Semiconductor\RSL10 EDK oxigen\eclipse\..\include\ble" -I"C:\Program Files (x86)\ON Semiconductor\RSL10 EDK oxigen\eclipse\..\include\kernel" -I"C:\Program Files (x86)\ON Semiconductor\RSL10 EDK oxigen\eclipse\..\include" -I"C:\Users\fg7fww\_Install\App\DemoBoards\RSL10\workspace_oxigen\central_peripheral\include" -std=gnu11 -Wmissing-prototypes -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


