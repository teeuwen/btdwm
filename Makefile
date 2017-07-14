#
# Makefile
#

PREFIX		:= /opt/btdwm

CC		:= gcc
LD		:= gcc

PKGLIST		= xcb-aux xcb-cursor xcb-ewmh xcb-icccm xcb-keysyms xcb-xinerama xcb pangocairo pango xau x11 xdmcp

MAKEFLAGS	:= -s

CFLAGS		:= -Wall -Wextra -Wno-unused-parameter -std=gnu89 `pkg-config --cflags $(PKGLIST)`
LDFLAGS		:= `pkg-config --libs $(PKGLIST)`

objects = btdwm.o draw.o functions.o layouts.o xcb.o

PHONY += all
all: release

PHONY += clean
clean:
	[ -f btdwm ] && echo -e "  RM      btdwm" || true
	rm -f btdwm
	find . -type f -name '*.o' -delete -exec sh -c "echo '  RM      {}'" \;

config.h:
	cp config.h.def config.h

%.o: %.c
	echo -e "  CC      $<"
	$(CC) $(CFLAGS) -c $< -o $@

PHONY += btdwm
btdwm: config.h $(objects)
	echo -e "  LD      $<"
	$(LD) -o $@ $(objects) $(LDFLAGS)

PHONY += release
release: CFLAGS+=-Os
release: LDFLAGS+=-Os -s
release: btdwm

PHONY += debug
debug: CFLAGS+=-g
debug: btdwm

PHONY += install
install: release
	echo -e "  INSTALL $(PREFIX)/bin/btdwm"
	mkdir -p $(PREFIX)/bin/
	cp -f btdwm $(PREFIX)/bin/.
	chmod 755 $(PREFIX)/bin/btdwm

PHONY += uninstall
uninstall:
	echo -e "  RM      $(PREFIX)/bin/btdwm"
	rm -f $(PREFIX)/bin/btdwm

.PHONY: $(PHONY)
