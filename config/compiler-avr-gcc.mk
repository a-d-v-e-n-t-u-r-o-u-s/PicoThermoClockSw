CC = avr-gcc
LD = avr-gcc
AR = avr-ar
NM = avr-nm
SIZE = avr-size
LOADER = avrdude

LIBPREFIX := lib
LIBEXT := .a

CDEFS = PCB=$(PCB)

ARFLAGS = rcs

CFLAGS = -c
CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -Wundef
CFLAGS += -Wpedantic
#CFLAGS += -Wconversion
#CFLAGS += -Wsign-conversion
CFLAGS += -fdata-sections
CFLAGS += -ffunction-sections
CFLAGS += -mmcu=$(MCU)
CFLAGS += -Os
CFLAGS += -std=c99
CFLAGS += $(addprefix -I,$(INCLUDE_DIR))
CFLAGS += $(addprefix -D,$(CDEFS))
CFLAGS += -Wa,-adhlns=$(OBJECTS_DIR)/$(@F).lst
CFLAGS += -o $(OBJECTS_DIR)/$(@F)


LDFLAGS += -mmcu=$(MCU)
LDFLAGS += -Wl,-Map,$(BIN_DIR_FORMATED)$(DELIM)$(OUTPUT).map
LDFLAGS += -Wl,--cref
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-L$(LIB_DIR)

LOADER_FLAGS += -c usbasp
LOADER_FLAGS += -p m8 -B 8
LOADER_FLAGS += -U flash:w:$(BIN_DIR_FORMATED)$(DELIM)$(OUTPUT).elf
