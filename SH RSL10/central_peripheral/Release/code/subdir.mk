################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../code/app_init.c \
../code/app_process.c \
../code/app_spi.c \
../code/ble_basc.c \
../code/ble_bass.c \
../code/ble_custom.c \
../code/ble_std.c 

OBJS += \
./code/app_init.o \
./code/app_process.o \
./code/app_spi.o \
./code/ble_basc.o \
./code/ble_bass.o \
./code/ble_custom.o \
./code/ble_std.o 

C_DEPS += \
./code/app_init.d \
./code/app_process.d \
./code/app_spi.d \
./code/ble_basc.d \
./code/ble_bass.d \
./code/ble_custom.d \
./code/ble_std.d 


# Each subdirectory must supply rules for building sources it contributes
code/%.o: ../code/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -DRSL10_CID=101 -DCFG_BLE -DCFG_ALLROLES -DCFG_APP -DCFG_ATTC -DCFG_ATTS -DCFG_CON=4 -DCFG_EMB -DCFG_HOST -DCFG_RF_ATLAS -DCFG_ALLPRF -DCFG_PRF -DCFG_NB_PRF=2 -DCFG_CHNL_ASSESS -DCFG_SEC_CON -DCFG_EXT_DB -DCFG_PRF_BASC -DCFG_PRF_BASS -I"C:\Program Files (x86)\ON Semiconductor\RSL10 EDK\eclipse\..\include\bb" -I"C:\Program Files (x86)\ON Semiconductor\RSL10 EDK\eclipse\..\include\ble\profiles" -I"C:\Program Files (x86)\ON Semiconductor\RSL10 EDK\eclipse\..\include\ble" -I"C:\Program Files (x86)\ON Semiconductor\RSL10 EDK\eclipse\..\include\kernel" -I"C:\Program Files (x86)\ON Semiconductor\RSL10 EDK\eclipse\..\include" -I"C:\Users\fg7fww\_Install\App\DemoBoards\RSL10\workspace_r1.3.4_test\central_peripheral\include" -I"C:\Users\fg7fww\_Install\App\DemoBoards\RSL10\workspace_r1.3.4_test\central_peripheral\code" -std=gnu11 -Wmissing-prototypes -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


