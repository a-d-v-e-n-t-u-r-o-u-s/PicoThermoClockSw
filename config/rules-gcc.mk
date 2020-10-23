include compiler-$(TARGET)-$(COMPILER).mk

CURDIR_RELATIVE := $(subst $(PROJECT_DIR),,$(CURDIR))
OBJECTS_DIR := $(BUILD_DIR)$(CURDIR_RELATIVE)

OBJECTS := $(addsuffix .o,$(basename $(SOURCE)))

INCLUDE_DIR += ../../projects/$(PROJECT) $(patsubst %,../../%,$(GLOBAL_INCLUDE_DIR))

vpath
vpath %.c 		$(subst  ,:,$(SOURCE_DIR))
vpath %.S 		$(subst  ,:,$(SOURCE_DIR))
vpath %.o 		$(OBJECTS_DIR)

.SUFFIXES:
.SUFFIXES: .c .o .S

.PHONY: all

LIBRARY := $(if $(LIBRARY),$(LIBPREFIX)$(LIBRARY)$(LIBEXT))

all: $(OBJECTS_DIR) $(LIBRARY) $(EXECUTABLE)

$(OBJECTS_DIR):
	@echo Creating object directory ...
	$(MKDIR) $(subst /,$(DELIM),$(OBJECTS_DIR))

$(OBJECTS): $(OBJECTS_DIR)

$(LIBRARY): $(OBJECTS)
	-$(RM_DIR) $(subst /,$(DELIM),$(OBJECTS_DIR))$(DELIM)$(subst /,$(DELIM),$@)
	@echo Creating library $(@F) ...
	cd $(OBJECTS_DIR) && $(AR) $(ARFLAGS) $@ $(^F)
	cp $(OBJECTS_DIR)/$@ $(LIB_DIR)

#TODO fix the creating executable, when $(TARGE_LINK_LIBRARIES) is empty
$(EXECUTABLE): $(OBJECTS)
	-$(RM_DIR) $(subst /,$(DELIM),$(OBJECTS_DIR))$(DELIM)$(subst /,$(DELIM),$@)
	@echo Creating executable $(@F) ... in $(CURDIR)
	$(LD) $(LDFLAGS) -o $(BIN_DIR)/$(EXECUTABLE) \
	$(patsubst %,$(OBJECTS_DIR)/%,$(OBJECTS)) -Wl,$(addprefix -l,$(TARGET_LINK_LIBRARIES))

%.o: %.c
	@echo $(<F)
	$(CC) $(CFLAGS) $(subst /,$(DELIM),$(CURDIR))$(DELIM)$(subst /,$(DELIM),$<)

%.o: %.S
	@echo $(<F)
	$(CC) $(CFLAGS) $(subst /,$(DELIM),$(CURDIR))$(DELIM)$(subst /,$(DELIM),$<)
