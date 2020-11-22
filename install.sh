#!/bin/bash
DESTDIR=/etc/juliawm/
sudo mkdir -p "${DESTDIR}"
sudo cp -R ${MESON_SOURCE_ROOT}/config/* "${DESTDIR}"
