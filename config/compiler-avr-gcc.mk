CC = avr-gcc
LD = avr-gcc
AR = avr-ar
LOADER = avrdude

LIBPREFIX := lib
LIBEXT := .a

CDEFS = F_CPU=16000000UL

ARFLAGS = rcs

CFLAGS = -c
CFLAGS += -Wall
CFLAGS += -mmcu=atmega8
CFLAGS += -O2
CFLAGS += -std=c99
CFLAGS += $(addprefix -I,$(INCLUDE_DIR))
CFLAGS += $(addprefix -D,$(CDEFS))
CFLAGS += -Wa,-adhlns=$(OBJECTS_DIR)/$(@F).lst
CFLAGS += -o $(OBJECTS_DIR)/$(@F)


LDFLAGS += -mmcu=atmega8
LDFLAGS += -Wl,-Map,$(BIN_DIR_FORMATED)$(DELIM)$(OUTPUT).map
LDFLAGS += -Wl,--cref
LDFLAGS += -Wl,-L$(LIB_DIR)

LOADER_FLAGS += -c usbasp
LOADER_FLAGS += -p m8 -B 8
LOADER_FLAGS += -U flash:w:$(BIN_DIR_FORMATED)$(DELIM)$(OUTPUT).elf
