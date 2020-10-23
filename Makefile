MAKE_VERSION_REQUIRED := 3.81

ifneq ($(MAKE_VERSION_REQUIRED),$(firstword $(sort $(MAKE_VERSION) $(MAKE_VERSION_REQUIRED))))
$(error You shall use make $(MAKE_VERSION) or higher)
endif

ifeq ($(OS),Windows_NT)
    HOST = win32
else
    ifeq ($(shell uname -s),Linux)
    HOST = linux
    else
    $(error Unrecognized host operating system)
    endif
endif

ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS = all
endif

include config/host-$(HOST).mk
include config/config.mk
-include config-user.mk

3RDPARTY_DIR:= 3rdparty
DRIVERS_DIR:= drivers
MODULES_DIR:= modules

include projects/$(PROJECT)/version.mk
include projects/$(PROJECT)/3rdparty.mk

3RDPARTYS_EXPORTS :=$(patsubst %,$(3RDPARTY_DIR)/%/exports.mk,$(USED_3RDPARTY))

include projects/$(PROJECT)/drivers.mk

DRIVERS_EXPORTS :=$(patsubst %,$(DRIVERS_DIR)/%/exports.mk,$(USED_DRIVERS))

include projects/$(PROJECT)/modules.mk

MODULES_EXPORTS :=$(patsubst %,$(MODULES_DIR)/%/exports.mk,$(USED_MODULES) $(MAIN_MODULE))

include $(3RDPARTYS_EXPORTS) $(DRIVERS_EXPORTS) $(MODULES_EXPORTS)

OUTPUT := $(PROJECT)_$(VERSION_MAJOR)_$(VERSION_MINOR)_$(VERSION_PATCH)

PROJECT_DIR := $(CURDIR)
BUILD_DIR := $(CURDIR)/build_$(OUTPUT)
BIN_DIR := $(BUILD_DIR)/bin
LIB_DIR := $(BUILD_DIR)/lib
DEP_DIR := $(BUILD_DIR)/dep
TOOLS_DIR := $(CURDIR)/tools

PROJECT_DIR_FORMATED := $(subst /,$(DELIM),$(PROJECT_DIR))
BUILD_DIR_FORMATED := $(subst /,$(DELIM),$(BUILD_DIR))
BIN_DIR_FORMATED := $(subst /,$(DELIM),$(BIN_DIR))
LIB_DIR_FORMATED := $(subst /,$(DELIM),$(LIB_DIR))
DEP_DIR_FORMATED := $(subst /,$(DELIM),$(DEP_DIR))
TOOLS_DIR_FORMATED := $(subst /,$(DELIM),$(TOOLS_DIR))

include config/compiler-$(TARGET)-$(COMPILER).mk
include config/exports.mk

MAKEFLAGS:= -I$(CURDIR) -I$(CURDIR)/config -I$(CURDIR)/projects/$(PROJECT) -R


.PHONY: all flash clean $(3RDPARTY_DIR) $(DRIVERS_DIR) $(MODULES_DIR)

all: banner $(BUILD_DIR) executable done


flash:
	$(LOADER) $(LOADER_FLAGS)

$(BUILD_DIR):
	-$(MKDIR) $(BIN_DIR_FORMATED)
	-$(MKDIR) $(LIB_DIR_FORMATED)
	-$(MKDIR) $(DEP_DIR_FORMATED)

executable: $(OUTPUT)

$(OUTPUT): $(3RDPARTY_DIR) $(DRIVERS_DIR) $(MODULES_DIR)

$(3RDPARTY_DIR):
	@echo Making $@
	$(MAKE) -C $@ $(MAKECMDGOALS)

$(DRIVERS_DIR):
	@echo Making $@
	$(MAKE) -C $@ $(MAKECMDGOALS)

$(MODULES_DIR):
	@echo Making $@
	$(MAKE) -C $@ $(MAKECMDGOALS)

banner:
	@echo -------------------------------------------------------------------------------
	@echo         PROJECT: $(PROJECT)
	@echo    MAKECMDGOALS: $(MAKECMDGOALS)
	@echo       TARGET_OS: $(TARGET_OS)
	@echo        COMPILER: $(CC) $(CCVERSION)
	@echo          OUTPUT: $(OUTPUT)
	@echo -------------------------------------------------------------------------------

done:
	@echo -------------------------------------------------------------------------------
	@echo Through the Force the binary files I build.
	@echo          OUTPUT: $(OUTPUT)
	@echo        RAM :
	@echo      FLASH :
	@echo May the Force be with you!!!
	@echo -------------------------------------------------------------------------------

clean:
	@echo ------------------------------------------------------------------------------
	@echo         Performing clean of $(PROJECT) ...
	-$(RM_DIR) $(BUILD_DIR_FORMATED) $(CMDQUIET)
	@echo         Done
	@echo ------------------------------------------------------------------------------
