# See LICENSE file for copyright and license details.

include config.mk

objects = btdwm.o config.o draw.o functions.o layouts.o msg.o xcb.o

all: btdwm

%.o: %.c
	echo -e "  CC      $<"
	$(CC) $(CFLAGS) -c $< -o $@

PHONY += btdwm
btdwm: $(objects)
	echo -e "  LD      $<"
	$(CC) -o $@ $(objects) $(LDFLAGS)

PHONY += clean
clean:
	echo -e "  RM      btdwm"
	rm -f btdwm
	#find . -type f -name '*.o' -delete -exec sh -c "echo '  RM      `echo '{}' | cut -c 3-`'" \;
	find . -type f -name '*.o' -delete -exec sh -c "echo '  RM      {}'" \;

PHONY += install
install: btdwm
	echo Installing executable file to $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f btdwm $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/btdwm

PHONY += uninstall
uninstall:
	echo Removing executable file from $(DESTDIR)$(PREFIX)/bin
	rm -f $(DESTDIR)$(PREFIX)/bin/btdwm

.PHONY: $(PHONY)
