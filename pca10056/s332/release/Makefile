PROJECT_NAME     := ble_app_lns_c_pca10052_s332
TARGETS          := nrf52840_xxaa
SOFTDEVICE       := components/softdevice/s332_v5.0.0/hex/ANT_s332_nrf52_5.0.0.hex

include Makefile.local

PROJ_DIR := ../../..

$(OUTPUT_DIRECTORY)/nrf52840_xxaa.out: \
  LINKER_SCRIPT  := ble_app_lns_c_gcc_nrf52.ld
  
# Source files common to all targets
SRC_FILES += \
  $(PROJ_DIR)/rf/ant.c \
  $(PROJ_DIR)/rf/fec.c \
  $(PROJ_DIR)/rf/ble_api5.c \
  $(PROJ_DIR)/rf/glasses.c \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52840.S \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52840.c \
  $(SDK_ROOT)/components/ant/ant_channel_config/ant_channel_config.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_hrm/ant_hrm.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_hrm/pages/ant_hrm_page_0.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_hrm/pages/ant_hrm_page_1.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_hrm/pages/ant_hrm_page_2.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_hrm/pages/ant_hrm_page_3.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_hrm/pages/ant_hrm_page_4.c \
  $(SDK_ROOT)/components/ant/ant_key_manager/ant_key_manager.c \
  $(SDK_ROOT)/components/ant/ant_state_indicator/ant_state_indicator.c \
  $(SDK_ROOT)/components/ant/ant_search_config/ant_search_config.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_glasses/ant_glasses.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_bsc/ant_bsc.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_bsc/pages/ant_bsc_combined_page_0.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_bsc/pages/ant_bsc_page_0.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_bsc/pages/ant_bsc_page_1.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_bsc/pages/ant_bsc_page_2.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_bsc/pages/ant_bsc_page_3.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_bsc/pages/ant_bsc_page_4.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_bsc/pages/ant_bsc_page_5.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_fec/ant_fec.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_fec/utils/ant_fec_utils.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_fec/pages/ant_fec_page_1.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_fec/pages/ant_fec_page_2.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_fec/pages/ant_fec_page_16.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_fec/pages/ant_fec_page_17.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_fec/pages/ant_fec_page_21.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_fec/pages/ant_fec_page_25.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_fec/pages/ant_fec_page_48.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_fec/pages/ant_fec_page_49.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_fec/pages/ant_fec_page_51.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_fec/pages/ant_fec_page_54.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_common/pages/ant_common_page_71.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_common/pages/ant_common_page_80.c \
  $(SDK_ROOT)/components/ant/ant_profiles/ant_common/pages/ant_common_page_81.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_ant.c \

include $(PROJ_DIR)/Makefile.files

include $(PROJ_DIR)/ble.files

# Include folders common to all targets
INC_FOLDERS += \
  ../config \
  $(SDK_ROOT)/components/softdevice/s332_v5.0.0/headers \
  $(SDK_ROOT)/components/softdevice/s332_v5.0.0/headers/nrf52 \
  
# Libraries common to all targets
LIB_FILES += \
  -lm -lstdc++ \
  $(SDK_ROOT)/components/toolchain/cmsis/dsp/GCC/libarm_cortexM4lf_math.a \


# Optimization flags
OPT = -O3
# Uncomment the line below to enable link time optimization
OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DUSE_VCOM_LOGS=0
CFLAGS += -DNRF_DRV_USBD_ERRATA_ENABLE=1
#CFLAGS += -DUSE_SVIEW=1 -DNRF_LOG_ENABLED=0 -DUSE_RTT=1
CFLAGS += -DUSE_SVIEW=0 -DNRF_LOG_ENABLED=0 -DUSE_RTT=1
#CFLAGS += -DUSE_SVIEW=0 -DUSE_JSCOPE -DNRF_LOG_ENABLED=0 -DUSE_RTT=1
#CFLAGS += -DDEBUG -DDEBUG_NRF -DDEBUG_NRF_USER
CFLAGS += -DUSE_APP_CONFIG
CFLAGS += -DNRF52840
CFLAGS += -DNRF52840_XXAA
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DSUPERSEDE_BOARD_ID -DPROTO_V9
CFLAGS += -DCONFIG_NFCT_PINS_AS_GPIOS
CFLAGS += -DBOARD_CUSTOM
CFLAGS += -D__HEAP_SIZE=150000
CFLAGS += -DANT_STACK_SUPPORT_REQD
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DNRF_SD_BLE_API_VERSION=5
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DS332
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -mcpu=cortex-m4 -DARM_MATH_CM4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall 
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums -nostdlib -MMD -nostdlib

# C++ flags common to all targets
CXXFLAGS += $(OPT) -std=gnu++0x -felide-constructors -fno-exceptions -fno-rtti

# Assembler flags common to all targets
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DARM_MATH_CM4
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DANT_STACK_SUPPORT_REQD
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DBOARD_CUSTOM
ASMFLAGS += -DNRF52840
ASMFLAGS += -DNRF52840_XXAA
ASMFLAGS += -D__HEAP_SIZE=150000
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=5
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DS332

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

# Flash softdevice
flash_softdevice:
	@echo Flashing
	nrfjprog --program $(SDK_ROOT)/$(SOFTDEVICE) -f nrf52 --sectorerase 
#	nrfjprog --reset -f nrf52

settings:
	nrfutil settings generate --family NRF52840 --application $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex --application-version 1 --bootloader-version 1 --bl-settings-version 1 settings.hex

dfu_softdevice:
	nrfutil --verbose pkg generate --hw-version 52 --sd-req 0 --softdevice $(SDK_ROOT)/$(SOFTDEVICE) --key-file $(SDK_ROOT)/../vault/priv.pem stravaV10_soft.zip
	nrfutil pkg display stravaV10_soft.zip
	nrfutil dfu usb_serial -pkg stravaV10_soft.zip -p $(COM_PORT)

dfu:
	nrfutil --verbose pkg generate --hw-version 52 --sd-req 0x9b --application-version 1 --application $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex --key-file $(SDK_ROOT)/../vault/priv.pem stravaV10.zip
	nrfutil pkg display stravaV10.zip
	nrfutil dfu usb_serial -pkg stravaV10.zip -p $(COM_PORT)

dfu_send:
	nrfutil dfu usb_serial -pkg stravaV10.zip -p $(COM_PORT)


SDK_CONFIG_FILE := ../config/sdk_config.h
SDK_CONFIG_FILE_ANT := ../config/sdk_config_ant.h
CMSIS_CONFIG_TOOL := $(SDK_ROOT)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar
sdk_config:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE)
	
sdk_config_ant:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE_ANT)

erase:
	nrfjprog --eraseall -f nrf52
