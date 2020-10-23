include compiler-$(TARGET)-$(COMPILER).mk

CURDIR_RELATIVE := $(subst $(PROJECT_DIR),,$(CURDIR))
OBJECTS_DIR := $(BUILD_DIR)$(CURDIR_RELATIVE)

OBJECTS := $(addsuffix .rel,$(basename $(SOURCE)))

INCLUDE_DIR += ../../projects/$(PROJECT) $(patsubst %,../../%,$(GLOBAL_INCLUDE_DIR))

vpath
vpath %.c 		$(subst  ,:,$(SOURCE_DIR))
vpath %.rel 		$(OBJECTS_DIR)

.SUFFIXES:
.SUFFIXES: .c .rel

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
	cd $(OBJECTS_DIR) && $(AR) $(ARFLAGS) -rc $@ $(^F)
	cp $(OBJECTS_DIR)/$@ $(LIB_DIR)

$(EXECUTABLE): $(OBJECTS)
	-$(RM_DIR) $(subst /,$(DELIM),$(OBJECTS_DIR))$(DELIM)$(subst /,$(DELIM),$@)
	@echo Creating executable $(@F) ... in $(CURDIR)
	$(LD) $(LDFLAGS) $(patsubst %,$(OBJECTS_DIR)/%,$(OBJECTS)) -L$(LIB_DIR) \
	$(addprefix lib, $(addsuffix .lib, $(TARGET_LINK_LIBRARIES))) -o $(BIN_DIR)/$(EXECUTABLE)

%.rel: %.c
	@echo $(<F)
	$(Q2)$(CC) $(CFLAGS) $(subst /,$(DELIM),$(CURDIR))$(DELIM)$(subst /,$(DELIM),$<)
