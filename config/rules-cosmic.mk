include compiler-$(TARGET)-$(COMPILER).mk

CURDIR_RELATIVE := $(subst $(PROJECT_DIR),,$(CURDIR))
OBJECTS_DIR := $(BUILD_DIR)$(CURDIR_RELATIVE)
OBJECTS_DIR := $(subst /,$(DELIM),$(OBJECTS_DIR))
OBJECTS_DIR_RELATIVE := $(subst /,$(DELIM),$(patsubst %,../..%,$(subst $(PROJECT_DIR_FORMATED),,$(OBJECTS_DIR))))
DEP_DIR_RELATIVE := $(subst /,$(DELIM),$(patsubst %,../..%,$(subst $(PROJECT_DIR_FORMATED),,$(DEP_DIR_FORMATED))))
LIB_DIR_RELATIVE := $(subst /,$(DELIM),$(patsubst %,../..%,$(subst $(PROJECT_DIR_FORMATED),,$(LIB_DIR_FORMATED))))
BIN_DIR_RELATIVE := $(subst /,$(DELIM),$(patsubst %,../..%,$(subst $(PROJECT_DIR_FORMATED),,$(BIN_DIR_FORMATED))))
TOOLS_DIR_RELATIVE := $(subst /,$(DELIM),$(patsubst %,../..%,$(subst $(PROJECT_DIR_FORMATED),,$(TOOLS_DIR_FORMATED))))

OBJECTS := $(addsuffix .o,$(basename $(SOURCE)))

INCLUDE_DIR += $(subst /,$(DELIM),../../projects/$(PROJECT)) $(subst /,$(DELIM),$(patsubst %,../../%,$(GLOBAL_INCLUDE_DIR)))

vpath
vpath %.c 		$(subst  ,:,$(SOURCE_DIR))
vpath %.s 		$(subst  ,:,$(SOURCE_DIR))
vpath %.o 		$(OBJECTS_DIR)
vpath %.lib 		$(LIB_DIR_RELATIVE)
vpath %.d 		$(DEP_DIR_RELATIVE)

.SUFFIXES:
.SUFFIXES: .c .o .s .d .lib

.PHONY: all

LIBRARY := $(if $(LIBRARY),$(LIBPREFIX)$(LIBRARY)$(LIBEXT))

all: $(OBJECTS_DIR_RELATIVE) $(LIBRARY) $(EXECUTABLE)

$(OBJECTS_DIR_RELATIVE):
	@echo Creating object directory ...
	$(MKDIR) $(OBJECTS_DIR_RELATIVE)

#$(RM) $(OBJECTS_DIR_RELATIVE)$(DELIM)$@

$(LIBRARY): $(OBJECTS)
	@echo Creating library $(@F) ...
	cd $(OBJECTS_DIR_RELATIVE) && $(AR) $(ARFLAGS) -c $@ $(^F) && $(OBJDUMP) $(OBJDUMP_FLAGS) -o$@.sym $(^F)
	$(MV) $(OBJECTS_DIR_RELATIVE)$(DELIM)$@ $(LIB_DIR_RELATIVE) $(CMDQUIET)

$(EXECUTABLE): $(OBJECTS)
	@echo Creating executable $(@F) ...
	$(LD) -o $(OBJECTS_DIR_RELATIVE)$(DELIM)$(EXECUTABLE) -m $(OBJECTS_DIR_RELATIVE)$(DELIM)$(OUTPUT).map $(LINKER_SCRIPT) $(LDFLAGS)
	$(CP) $(OBJECTS_DIR_RELATIVE)$(DELIM)$(EXECUTABLE) $(BIN_DIR_RELATIVE) $(CMDQUIET)
	$(CP) $(OBJECTS_DIR_RELATIVE)$(DELIM)$(OUTPUT).map $(BIN_DIR_RELATIVE) $(CMDQUIET)
	$(FIND) "segment" $(OBJECTS_DIR_RELATIVE)$(DELIM)$(OUTPUT).map
	$(SREC) -fi -o $(BIN_DIR_RELATIVE)$(DELIM)$(OUTPUT).hex $(BIN_DIR_RELATIVE)$(DELIM)$(EXECUTABLE)

%.o: %.c
	$(CC) $(CFLAGS) $<
	$(CC) $(CFLAGS) -sm $< > $(DEP_DIR_RELATIVE)$(DELIM)$(basename $(notdir $(<))).dep 2>&1
	$(TOOLS_DIR_RELATIVE)$(DELIM)$(SED) /C:\\/d $(DEP_DIR_RELATIVE)$(DELIM)$(basename $(notdir $(<))).dep > $(DEP_DIR_RELATIVE)$(DELIM)$(basename $(notdir $(<))).tmp
	$(TOOLS_DIR_RELATIVE)$(DELIM)$(SED) -e s/$(subst \,\\,$(OBJECTS_DIR_RELATIVE))\\//g $(DEP_DIR_RELATIVE)$(DELIM)$(basename $(notdir $(<))).tmp > $(DEP_DIR_RELATIVE)$(DELIM)$(basename $(notdir $(<))).d
	$(RM) $(DEP_DIR_RELATIVE)$(DELIM)$(basename $(notdir $(<))).dep
	$(RM) $(DEP_DIR_RELATIVE)$(DELIM)$(basename $(notdir $(<))).tmp

%.o: %.s
	$(CC) $(CFLAGS) $<

-include $(DEP_DIR)/*.d
