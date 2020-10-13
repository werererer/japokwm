CFLAGS += -fPIC -DWLR_USE_UNSTABLE
PKGS = wlroots wayland-server xcb xkbcommon
CFLAGS += $(foreach p,$(PKGS),$(shell pkg-config --cflags $(p)))
LDLIBS += -lX11
LDLIBS += $(foreach p,$(PKGS),$(shell pkg-config --libs $(p)))

WAYLAND_PROTOCOLS=$(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER=$(shell pkg-config --variable=wayland_scanner wayland-scanner)
LDLIBS += $(foreach p,$(PKGS),$(shell pkg-config --libs $(p)))

xdg-shell-protocol.h:
	$(WAYLAND_SCANNER) server-header \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml $@

xdg-shell-protocol.c:
	$(WAYLAND_SCANNER) private-code \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml $@

xdg-shell-protocol.o: xdg-shell-protocol.h

wayland-util.so: xdg-shell-protocol.o wayland-util.c
	gcc -shared $(CFLAGS) $(LDLIBS) $< -o $@
clean:
	rm wayland-util.so

.DEFAULT_GOAL=wayland-util.so
.PHONY=clean
