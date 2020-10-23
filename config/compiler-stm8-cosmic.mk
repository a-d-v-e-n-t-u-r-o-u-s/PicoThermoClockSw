CC = cxstm8
LD = clnk
AR = clib
SREC = chex
LOADER = STVP_CmdLine
OBJDUMP = cobj

LIBPREFIX := lib
LIBEXT := .lib

CDEFS = STM8L15X_MD

CFLAGS = +mods0
CFLAGS += $(addprefix -i,$(INCLUDE_DIR))
CFLAGS += $(addprefix -d,$(CDEFS))
CFLAGS += -co$(OBJECTS_DIR_RELATIVE)
CFLAGS += -cl$(OBJECTS_DIR_RELATIVE)
CFLAGS += -l
#! \TODO check does it work ?
CFLAGS += +proto
#! \TODO check does it work ?
CFLAGS += +split
CFLAGS += +strict
CFLAGS += +warn

#ARFLAGS =
#ARFLAGS +=

#LDFLAGS =
#LDFLAGS +=

LOADER_FLAGS += -progress
LOADER_FLAGS += -BoardName=ST-LINK
LOADER_FLAGS += -Device=STM8L15xC6
LOADER_FLAGS += -Port=USB
LOADER_FLAGS += -ProgMode=SWIM
LOADER_FLAGS += -FileProg=$(BIN_DIR_FORMATED)$(DELIM)$(OUTPUT).hex

OBJDUMP_FLAGS += -s
OBJDUMP_FLAGS += -n
OBJDUMP_FLAGS += -x
