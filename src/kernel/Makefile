include config.mk

CFLAGS += -g3 -fPIC -DWLR_USE_UNSTABLE

WAYLAND_PROTOCOLS=$(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER=$(shell pkg-config --variable=wayland_scanner wayland-scanner)

PKGS = wlroots wayland-server xcb xkbcommon
CFLAGS += $(foreach p,$(PKGS),$(shell pkg-config --cflags $(p)))
LDLIBS += -ljulia
LDLIBS += $(foreach p,$(PKGS),$(shell pkg-config --libs $(p)))
CFiles = $(filter-out debug.c, $(filter-out xdg-shell-protocol.c, $(wildcard *.c)))

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

debug: debug.c
	$(CC) $(CFLAGS) $(LDLIBS) $^ $(wildcard *.o) -o $@

juliawm.so: xdg-shell-protocol.o
	$(CC) -shared $(CFLAGS) $(LDLIBS) $(CFiles) $(wildcard *.o) -o $@

recompile: clean all

all: juliawm.so debug

wayland-util.so: wayland-util.c
	$(CC) $(CFLAGS) $(LDLIBS) $(CFiles) -o $@

clean:
	rm -f juliawm.so *.o xdg-shell-protocol.h xdg-shell-protocol.c

.DEFAULT_GOAL=all
.PHONY: clean
