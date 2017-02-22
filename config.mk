CC		:= gcc

PREFIX		:= /opt/btdwm

PKGLIST		= xcb-aux xcb-cursor xcb-ewmh xcb-icccm xcb-keysyms xcb-xinerama xcb pangocairo pango libnotify xau x11 xdmcp

MAKEFLAGS	:= -s

CFLAGS		:= -Wall -Wextra -Wno-unused-parameter -std=gnu89 `pkg-config --cflags $(PKGLIST)` -Os #-g
LDFLAGS		:= `pkg-config --libs $(PKGLIST)` -Os -s #-g
