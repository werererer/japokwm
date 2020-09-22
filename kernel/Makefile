include config.mk

CFLAGS += -I. -fPIC -DWLR_USE_UNSTABLE

WAYLAND_PROTOCOLS=$(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER=$(shell pkg-config --variable=wayland_scanner wayland-scanner)

PKGS = wlroots wayland-server xcb xkbcommon
CFLAGS += $(foreach p,$(PKGS),$(shell pkg-config --cflags $(p)))
CFLAGS += -ljulia
LDLIBS += $(foreach p,$(PKGS),$(shell pkg-config --libs $(p)))
LDLIBS += -ljulia
CFiles = $(wildcard *.c)


# wayland-scanner is a tool which generates C headers and rigging for Wayland
# protocols, which are specified in XML. wlroots requires you to rig these up
# to your build system yourself and provide them in the include path.

xdg-shell-protocol.h:
	$(WAYLAND_SCANNER) server-header \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml $@

xdg-shell-protocol.c:
	$(WAYLAND_SCANNER) private-code \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml $@

xdg-shell-protocol.o: xdg-shell-protocol.h

config.h: | config.def.h
	cp config.def.h $@

dwl: xdg-shell-protocol.o
	$(CC) $(CFLAGS) $(CFiles) $(wildcard *.o) -o dwl

clean:
	rm -f dwl *.o xdg-shell-protocol.h xdg-shell-protocol.c

.DEFAULT_GOAL=dwl
.PHONY: clean
