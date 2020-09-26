include config.mk

CFLAGS += -g3 -I. -fPIC -DWLR_USE_UNSTABLE

WAYLAND_PROTOCOLS=$(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER=$(shell pkg-config --variable=wayland_scanner wayland-scanner)

PKGS = wlroots wayland-server xcb xkbcommon
CFLAGS += $(foreach p,$(PKGS),$(shell pkg-config --cflags $(p)))
LDLIBS += -ljulia
LDLIBS += $(foreach p,$(PKGS),$(shell pkg-config --libs $(p)))
CFiles = $(filter-out xdg-shell-protocol.c, $(wildcard *.c))
OUTFILE = juliawm

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

$(OUTFILE): xdg-shell-protocol.o
	$(CC) $(CFLAGS) $(LDLIBS) $(CFiles) $(wildcard *.o) -o $(OUTFILE)

recompile: clean $(OUTFILE)

clean:
	rm -f $(OUTFILE) *.o xdg-shell-protocol.h xdg-shell-protocol.c

.DEFAULT_GOAL=recompile
# .DEFAULT_GOAL=$(OUTFILE)
.PHONY: clean
