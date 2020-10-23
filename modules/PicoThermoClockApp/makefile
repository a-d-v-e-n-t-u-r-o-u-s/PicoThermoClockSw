SOURCE += main.c

SOURCE_DIR := source
INCLUDE_DIR := include

#LINKER_SCRIPT := source/$(PROJECT).lkf
LIBRARY := PicoThermoClockApp

TARGET_LINK_LIBRARIES := gpio
TARGET_LINK_LIBRARIES +=

EXTENSION_EXE := elf
EXECUTABLE := $(OUTPUT).$(EXTENSION_EXE)

include rules-$(COMPILER).mk
