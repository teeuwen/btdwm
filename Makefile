#
# Makefile
#

PREFIX		:= /opt/btdwm

CC		:= gcc
LD		:= gcc

PKGLIST		= xcb-aux xcb-cursor xcb-ewmh xcb-icccm xcb-keysyms xcb-xinerama xcb pangocairo pango xau x11 xdmcp

MAKEFLAGS	:= -s

CFLAGS		:= -Wall -Wextra -Wno-unused-parameter -Wpedantic -std=c99 `pkg-config --cflags $(PKGLIST)`
LDFLAGS		:= `pkg-config --libs $(PKGLIST)`

include src/Makefile

PHONY += all
all: release

PHONY += clean
clean:
	[ -f btdwm ] && echo -e "  RM      btdwm" || true
	rm -f btdwm
	find . -type f -name '*.o' -delete -exec sh -c "echo '  RM      {}'" \;

cfg/config.h:
	mkdir -p cfg
	cp src/include/config.h.def cfg/config.h

%.o: %.c
	echo -e "  CC      $<"
	$(CC) $(CFLAGS) -c $< $(btdwm-i) -o $@

PHONY += btdwm bin/btdwm
btdwm: bin/btdwm
bin/btdwm: cfg/config.h $(btdwm-o)
	mkdir -p bin
	echo -e "  LD      $<"
	$(LD) -o $@ $(btdwm-o) $(LDFLAGS)

PHONY += release
release: CFLAGS+=-Os
release: LDFLAGS+=-Os -s
release: btdwm

PHONY += debug
debug: CFLAGS+=-g
debug: bin/btdwm

PHONY += install
install: release
	echo -e "  INSTALL $(PREFIX)/bin/btdwm"
	mkdir -p $(PREFIX)/bin/
	cp -f bin/btdwm $(PREFIX)/bin/.
	chmod 755 $(PREFIX)/bin/btdwm

PHONY += uninstall
uninstall:
	echo -e "  RM      $(PREFIX)/bin/btdwm"
	rm -f $(PREFIX)/bin/btdwm

.PHONY: $(PHONY)
