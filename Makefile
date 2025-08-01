# Makefile
#
# Copyright (C) 2025, Charles Chiou

MAKEFLAGS =	--no-print-dir

PICO_SDK_PATH :=	$(realpath pico-sdk)
export PICO_SDK_PATH

TARGETS +=	build/picows.uf2

.PHONY: default clean distclean $(TARGETS)

default: $(TARGETS)

clean:
	@test -f build/Makefile && $(MAKE) -C build clean

distclean:
	rm -rf build/

.PHONY: picows

picows: build/picows.uf2

build/picows.uf2: build/Makefile
	@$(MAKE) -C build

build/Makefile: CMakeLists.txt
	@mkdir -p build
	@cd build && cmake -DPICO_BOARD=pico_w ..

.PHONY: release

release: build/Makefile
	@rm -f build/version.h
	@$(MAKE) -C build
