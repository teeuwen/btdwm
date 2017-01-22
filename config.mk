CC		:= gcc

PREFIX		:= /opt/btdwm

PKGLIST		= xcb-aux xcb-cursor xcb-ewmh xcb-icccm xcb-keysyms xcb-xinerama xcb pangocairo pango libnotify

MAKEFLAGS	:= -s

CFLAGS		:= -Wall -std=gnu89 `pkg-config --cflags $(PKGLIST)` -g #-Wextra #-Os
LDFLAGS		:= `pkg-config --libs $(PKGLIST)` -g
