CC = sdcc
LD = sdcc
AR = sdar
LOADER = stm8flash

LIBPREFIX := lib
LIBEXT := .lib

#!\todo make this setting configurable
# RemoteControl project setting is enabled
#CDEFS = STM8L15X_MD
CDEFS = STM8L15X_HD

CFLAGS = -c
CFLAGS += -DSDCC
CFLAGS += -mstm8
CFLAGS += --std-c99
CFLAGS += $(addprefix -I,$(INCLUDE_DIR))
CFLAGS += $(addprefix -D,$(CDEFS))
CFLAGS += -o $(OBJECTS_DIR)/$(@F)

LDFLAGS = -mstm8

LOADER_FLAGS += -c stlink
LOADER_FLAGS += -s 0x8000
#!\todo make this setting configurable
# RemoteControl project setting is enabled
#LOADER_FLAGS += -p stm8l152?6
LOADER_FLAGS += -pstm8l151?8
LOADER_FLAGS += -w $(BIN_DIR_FORMATED)$(DELIM)$(OUTPUT).ihx
