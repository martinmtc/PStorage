################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
..\PStorageTest.cpp \
..\sloeber.ino.cpp 

LINK_OBJ += \
.\PStorageTest.cpp.o \
.\sloeber.ino.cpp.o 

CPP_DEPS += \
.\PStorageTest.cpp.d \
.\sloeber.ino.cpp.d 


# Each subdirectory must supply rules for building sources it contributes
PStorageTest.cpp.o: ..\PStorageTest.cpp
	@echo 'Building file: $<'
	@echo 'Starting C++ compile'
	"C:\Dev\Eclipse\arduinoPlugin\packages\esp8266\tools\xtensa-lx106-elf-gcc\1.20.0-26-gb404fb9-2/bin/xtensa-lx106-elf-g++" -D__ets__ -DICACHE_FLASH -U__STRICT_ANSI__ "-IC:\Dev\Eclipse\/arduinoPlugin/packages/esp8266/hardware/esp8266/2.4.1/tools/sdk/include" "-IC:\Dev\Eclipse\/arduinoPlugin/packages/esp8266/hardware/esp8266/2.4.1/tools/sdk/lwip2/include" "-IC:\Dev\Eclipse\/arduinoPlugin/packages/esp8266/hardware/esp8266/2.4.1/tools/sdk/libc/xtensa-lx106-elf/include" "-IC:/Users/marti/Documents/Arduino/libraries_own/PStorage/Release/core" -c -Wall -Wextra -Os -g -mlongcalls -mtext-section-literals -fno-exceptions -fno-rtti -falign-functions=4 -std=c++11 -MMD -ffunction-sections -fdata-sections -DF_CPU=80000000L -DLWIP_OPEN_SRC -DTCP_MSS=536  -DDEBUG_ESP_CORE -DARDUINO=10802 -DARDUINO_ESP8266_WEMOS_D1MINI -DARDUINO_ARCH_ESP8266 -DARDUINO_BOARD="ESP8266_WEMOS_D1MINI"   -DESP8266   -I"C:\Dev\Eclipse\arduinoPlugin\packages\esp8266\hardware\esp8266\2.4.1\cores\esp8266" -I"C:\Dev\Eclipse\arduinoPlugin\packages\esp8266\hardware\esp8266\2.4.1\variants\d1_mini" -I"C:\Users\marti\Documents\Arduino\libraries_own\PStorage\src" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -D__IN_ECLIPSE__=1 -x c++ "$<"  -o  "$@"
	@echo 'Finished building: $<'
	@echo ' '

sloeber.ino.cpp.o: ..\sloeber.ino.cpp
	@echo 'Building file: $<'
	@echo 'Starting C++ compile'
	"C:\Dev\Eclipse\arduinoPlugin\packages\esp8266\tools\xtensa-lx106-elf-gcc\1.20.0-26-gb404fb9-2/bin/xtensa-lx106-elf-g++" -D__ets__ -DICACHE_FLASH -U__STRICT_ANSI__ "-IC:\Dev\Eclipse\/arduinoPlugin/packages/esp8266/hardware/esp8266/2.4.1/tools/sdk/include" "-IC:\Dev\Eclipse\/arduinoPlugin/packages/esp8266/hardware/esp8266/2.4.1/tools/sdk/lwip2/include" "-IC:\Dev\Eclipse\/arduinoPlugin/packages/esp8266/hardware/esp8266/2.4.1/tools/sdk/libc/xtensa-lx106-elf/include" "-IC:/Users/marti/Documents/Arduino/libraries_own/PStorage/Release/core" -c -Wall -Wextra -Os -g -mlongcalls -mtext-section-literals -fno-exceptions -fno-rtti -falign-functions=4 -std=c++11 -MMD -ffunction-sections -fdata-sections -DF_CPU=80000000L -DLWIP_OPEN_SRC -DTCP_MSS=536  -DDEBUG_ESP_CORE -DARDUINO=10802 -DARDUINO_ESP8266_WEMOS_D1MINI -DARDUINO_ARCH_ESP8266 -DARDUINO_BOARD="ESP8266_WEMOS_D1MINI"   -DESP8266   -I"C:\Dev\Eclipse\arduinoPlugin\packages\esp8266\hardware\esp8266\2.4.1\cores\esp8266" -I"C:\Dev\Eclipse\arduinoPlugin\packages\esp8266\hardware\esp8266\2.4.1\variants\d1_mini" -I"C:\Users\marti\Documents\Arduino\libraries_own\PStorage\src" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -D__IN_ECLIPSE__=1 -x c++ "$<"  -o  "$@"

	@echo 'Finished building: $<'
	@echo ' '


