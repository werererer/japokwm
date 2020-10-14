include config.mk

CFLAGS += -g3 -fPIC -DWLR_USE_UNSTABLE

WAYLAND_PROTOCOLS=$(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER=$(shell pkg-config --variable=wayland_scanner wayland-scanner)

PKGS = wlroots wayland-server xcb xkbcommon
CFLAGS += $(foreach p,$(PKGS),$(shell pkg-config --cflags $(p)))
LDLIBS += -ljulia
LDLIBS += $(foreach p,$(PKGS),$(shell pkg-config --libs $(p)))
CFiles = $(filter-out debug.c, $(filter-out xdg-shell-protocol.c, $(wildcard *.c)))

TOPLEVEL = $(shell git rev-parse --show-toplevel)
PROTOCOL = $(TOPLEVEL)/protocols
LAYER_SHELL_PROTOCOL = $(PROTOCOL)/wlr-layer-shell-unstable-v1.xml

# wayland-scanner is a tool which generates C headers and rigging for Wayland
# protocols, which are specified in XML. wlroots requires you to rig these up
# to your build system yourself and provide them in the include path.

xdg-shell-protocol.h:
	$(WAYLAND_SCANNER) server-header \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml $@

xdg-shell-protocol.c:
	$(WAYLAND_SCANNER) private-code \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml $@

wlr-layer-shell-unstable-v1-protocol.h: wlr-layer-shell-unstable-v1-protocol.c
	$(WAYLAND_SCANNER) server-header \
		$(LAYER_SHELL_PROTOCOL) $@

wlr-layer-shell-unstable-v1-protocol.c:
	$(WAYLAND_SCANNER) private-code \
		$(LAYER_SHELL_PROTOCOL) \$@

xdg-shell-protocol.o: xdg-shell-protocol.h

debug: debug.c
	$(CC) $(CFLAGS) $(LDLIBS) $^ $(wildcard *.o) -o $@

juliawm.so: $(wildcard *.o) $(CFiles)
	$(CC) -shared $(CFLAGS) $(LDLIBS) $^ -o $@

recompile: clean all

all: juliawm.so debug wlr-layer-shell-unstable-v1-protocol.h

wayland-util.so: wayland-util.c
	$(CC) $(CFLAGS) $(LDLIBS) $(CFiles) -o $@

clean:
	rm -f juliawm.so *.o xdg-shell-protocol.h xdg-shell-protocol.c

.DEFAULT_GOAL=all
.PHONY: clean
