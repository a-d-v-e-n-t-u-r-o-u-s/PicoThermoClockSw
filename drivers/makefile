include drivers.mk

.PHONY: all $(USED_DRIVERS)

all: $(USED_DRIVERS)

$(USED_DRIVERS):
	$(MAKE) -C $@ $(MAKEGOALS)

