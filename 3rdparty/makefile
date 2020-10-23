include 3rdparty.mk

.PHONY: all $(USED_3RDPARTY)

all: $(USED_3RDPARTY)

$(USED_3RDPARTY):
	$(MAKE) -C $@ $(MAKEGOALS)

