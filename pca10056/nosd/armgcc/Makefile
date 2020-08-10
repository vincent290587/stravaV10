PROJECT_NAME     := strava10_pca10052_nosd
TARGETS          := nrf52840_xxaa
OUTPUT_DIRECTORY := _build

include Makefile.local

PROJ_DIR := ../../..

$(OUTPUT_DIRECTORY)/nrf52840_xxaa.out: \
  LINKER_SCRIPT  := ble_app_lns_c_gcc_nrf52.ld
  
# Source files common to all targets
SRC_FILES += \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52840.S \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52840.c \
  $(wildcard $(PROJ_DIR)/source/usb/*.c) \
  $(wildcard $(PROJ_DIR)/source/usb/*.cpp) \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd_core.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd_serial_num.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd_string_desc.c \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc/acm/app_usbd_cdc_acm.c \
  $(SDK_ROOT)/components/libraries/usbd/class/msc/app_usbd_msc.c \
  $(SDK_ROOT)/components/libraries/usbd/class/dummy/app_usbd_dummy.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_usbd.c \
  $(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd/nrf_nvic.c \
  $(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd/nrf_soc.c \

# Include folders common to all targets
INC_FOLDERS += \
  ../../s340/config \
  $(SDK_ROOT)/components/softdevice/mbr/headers \
  $(PROJ_DIR)/source/usb \
  $(SDK_ROOT)/modules/nrfx/drivers/src/ \
  $(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd/ \
  $(SDK_ROOT)/components/libraries/usbd \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc/acm \
  $(SDK_ROOT)/components/libraries/usbd/class/msc \
  $(SDK_ROOT)/components/libraries/usbd/class/dummy \
  
include $(PROJ_DIR)/Makefile.files
  
# Libraries common to all targets
LIB_FILES += \
  -lm -lstdc++ \
  $(SDK_ROOT)/components/toolchain/cmsis/dsp/GCC/libarm_cortexM4lf_math.a \


# Optimization flags
OPT = -O3 -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DUSB_ENABLED -DUSE_VCOM_LOGS=0
CFLAGS += -DNRF_DRV_USBD_ERRATA_ENABLE=1
CFLAGS += -DUSE_SVIEW=0 -DNRF_LOG_ENABLED=1 -DUSE_RTT=1
#CFLAGS += -DNRF_LOG_ENABLED=0 -DNRF_LOG_USES_RTT=0 -DNRF_LOG_BACKEND_RTT_ENABLED=0
#CFLAGS += -DNRF_LOG_BACKEND_UART_ENABLED=1 -DNRF_LOG_BACKEND_RTT_ENABLED=0
CFLAGS += -DDEBUG -DDEBUG_NRF -DDEBUG_NRF_USER
CFLAGS += -DAPP_TIMER_V2
CFLAGS += -DAPP_TIMER_V2_RTC1_ENABLED
CFLAGS += -DUSE_APP_CONFIG
CFLAGS += -DNRF52840
CFLAGS += -DNRF52840_XXAA
CFLAGS += -DCONFIG_NFCT_PINS_AS_GPIOS
CFLAGS += -DBOARD_CUSTOM
CFLAGS += -D__HEAP_SIZE=150000
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DNRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED=0
CFLAGS += -mcpu=cortex-m4 -DARM_MATH_CM4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall 
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant
#CFLAGS += -Wdouble-promotion -Wfloat-conversion
# keep every function in separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums -nostdlib -MMD -nostdlib

# C++ flags common to all targets
CXXFLAGS += $(OPT) -std=gnu++0x -felide-constructors -fno-exceptions -fno-rtti

# Assembler flags common to all targets
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DARM_MATH_CM4
ASMFLAGS += -DBOARD_CUSTOM
ASMFLAGS += -DAPP_TIMER_V2
ASMFLAGS += -DAPP_TIMER_V2_RTC1_ENABLED
ASMFLAGS += -DNRF52840
ASMFLAGS += -DNRF52840_XXAA
ASMFLAGS += -D__HEAP_SIZE=150000
ASMFLAGS += -DSWI_DISABLE0

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -u _printf_float -u _scanf_float

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm

.PHONY: $(TARGETS) default all clean help flash flash_softdevice

# Default target - first one defined
default: nrf52840_xxaa

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo 	nrf52840_xxaa

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc

include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

# Flash the program
flash: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex
	@echo Flashing: $<
	nrfjprog --program $< -f nrf52 --sectorerase
	nrfjprog --reset -f nrf52
	nrfjprog --run

# Flash the program and the DFU settings so that
flash_dfu: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex
	@echo Generating DFU settings...
	nrfutil settings generate --family NRF52840 --application-version 1 --application $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex  --bootloader-version 1 --bl-settings-version 2 settings_dfu.hex
	mergehex --merge $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex settings_dfu.hex --output app_dfu_settings.hex
	@echo Flashing: app_dfu_settings.hex
	nrfjprog --program app_dfu_settings.hex -f nrf52 --sectorerase
	nrfjprog --reset -f nrf52
	nrfjprog --run

settings:
	nrfutil settings generate --family NRF52840 --application $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex --application-version 1 --bootloader-version 1 --bl-settings-version 1 settings.hex

dfu:
	nrfutil --verbose pkg generate --hw-version 52 --sd-req 0 --application-version 1 --application $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex --key-file $(SDK_ROOT)/../vault/priv.pem stravaV10.zip
	nrfutil pkg display stravaV10.zip
	nrfutil dfu usb-serial -pkg stravaV10.zip -p $(COM_PORT)

dfu_send:
	nrfutil dfu usb-serial -pkg stravaV10.zip -p $(COM_PORT)

jumper: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex
	jumper run -g --fw $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex --gpio --platform nrf52840

SDK_CONFIG_FILE := ../config/sdk_config.h
SDK_CONFIG_FILE_ANT := ../config/sdk_config_ant.h
CMSIS_CONFIG_TOOL := $(SDK_ROOT)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar
sdk_config:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE)
	
sdk_config_ant:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE_ANT)

erase:
	nrfjprog --eraseall -f nrf52
