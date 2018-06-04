PROJECT_NAME     := ble_app_lns_c_pca10040_s132
TARGETS          := nrf52832_xxaa
OUTPUT_DIRECTORY := _build

include Makefile.local

PROJ_DIR := ../../..

$(OUTPUT_DIRECTORY)/nrf52832_xxaa.out: \
  LINKER_SCRIPT  := ble_app_lns_c_gcc_nrf52.ld
  
include $(PROJ_DIR)/Makefile.files

# Source files common to all targets
SRC_FILES += \

# Include folders common to all targets
INC_FOLDERS += \


# Libraries common to all targets
LIB_FILES += \

# Optimization flags
OPT = -O3 -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
#CFLAGS += -DNRF_LOG_ENABLED=0
CFLAGS += -DDEBUG
CFLAGS += -DNRF52
CFLAGS += -DNRF52832_XXAA
CFLAGS += -DNRF52_PAN_36
CFLAGS += -DNRF52_PAN_64
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DCONFIG_NFCT_PINS_AS_GPIOS
CFLAGS += -DBOARD_CUSTOM
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DNRF52832
CFLAGS += -DNRF52_PAN_12
CFLAGS += -DNRF52_PAN_58
CFLAGS += -DNRF52_PAN_54
CFLAGS += -DNRF52_PAN_31
CFLAGS += -DNRF52_PAN_51
CFLAGS += -D__HEAP_SIZE=256
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DANT_STACK_SUPPORT_REQD
CFLAGS += -DNRF52_PAN_15
CFLAGS += -DNRF_SD_BLE_API_VERSION=5
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DNRF52_PAN_20
CFLAGS += -DNRF52_PAN_55
CFLAGS += -DS332
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall $(OPT)
CFLAGS += -mfloat-abi=soft
# keep every function in separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums -nostdlib -fno-exceptions

# C++ flags common to all targets
CXXFLAGS += $(OPT) -std=gnu++0x -felide-constructors -fno-exceptions -fno-rtti

# Assembler flags common to all targets
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=soft
ASMFLAGS += -DNRF52
ASMFLAGS += -DNRF52_PAN_36
ASMFLAGS += -DNRF52_PAN_64
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DBOARD_CUSTOM
ASMFLAGS += -DNRF52832
ASMFLAGS += -DNRF52832_XXAA
ASMFLAGS += -DNRF52_PAN_12
ASMFLAGS += -DNRF52_PAN_58
ASMFLAGS += -DNRF52_PAN_54
ASMFLAGS += -DNRF52_PAN_31
ASMFLAGS += -DNRF52_PAN_51
ASMFLAGS += -D__HEAP_SIZE=256
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD -DANT_STACK_SUPPORT_REQD
ASMFLAGS += -DNRF52_PAN_15
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=5
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DNRF52_PAN_20
ASMFLAGS += -DNRF52_PAN_55
ASMFLAGS += -DS332

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=soft
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm

.PHONY: $(TARGETS) default all clean help flash flashl flash_softdevice

# Default target - first one defined
default: nrf52832_xxaa

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo 	nrf52832_xxaa

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc


include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

# Flash the program
flash: $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex
	@echo Flashing: $<
	nrfjprog --program $< -f nrf52 --sectorerase
	nrfjprog --reset -f nrf52
	
# Flash the program
flashl:
	@echo Flashing: nrf52832_xxaa.hex
	nrfjprog --program $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex -f nrf52 --sectorerase
	nrfjprog --reset -f nrf52

# Flash softdevice
flash_softdevice:
	@echo Flashing
	nrfjprog --program $(SDK_ROOT)/components/softdevice/s332_v5.0.0/hex/ANT_s332_nrf52_5.0.0.hex -f nrf52 --sectorerase 
#	nrfjprog --reset -f nrf52

dfu: $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex
	nrfutil --verbose pkg generate --hw-version 52 --sd-req 0x9b --application-version 1 --application $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex --key-file $(SDK_ROOT)/../vault/priv.pem s332_stravaV10.zip
	mv s332_stravaV10.zip $(DROPBOX)

SDK_CONFIG_FILE := ../config/sdk_config.h
SDK_CONFIG_FILE_ANT := ../config/sdk_config_ant.h
CMSIS_CONFIG_TOOL := $(SDK_ROOT)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar
sdk_config:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE)
	
sdk_config_ant:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE_ANT)

erase:
	nrfjprog --eraseall -f nrf52
