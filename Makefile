# See LICENSE file for copyright and license details.

include config.mk

objects = btdwm.o config.o functions.o layouts.o libnotify.o pango.o xcb.o

all: btdwm

%.o: %.c
	echo -e "  CC      $<"
	@$(CC) $(CFLAGS) -c $< -o $@

PHONY += btdwm
btdwm: $(objects)
	echo -e "  LD      $<"
	@$(CC) $(LDFLAGS) -o $@ $(objects)

PHONY += clean
clean:
	@echo cleaning
	@rm -f btdwm $(objects)

PHONY += install
install: btdwm
	@echo Installing executable file to $(DESTDIR)$(PREFIX)/bin
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp -f btdwm $(DESTDIR)$(PREFIX)/bin
	@chmod 755 $(DESTDIR)$(PREFIX)/bin/btdwm

PHONY += uninstall
uninstall:
	@echo Removing executable file from $(DESTDIR)$(PREFIX)/bin
	@rm -f $(DESTDIR)$(PREFIX)/bin/btdwm

.PHONY: $(PHONY)
