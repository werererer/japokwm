#ifndef WRITE_FILE_H
#define WRITE_FILE_H
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <wlr/types/wlr_box.h>
#include "utils/stringUtils.h"
#include "utils/coreUtils.h"
#include "render/render.h"

char *layoutBegin = "layout =";
void writeContainerToFile(char *file, struct wlr_fbox *box);
void writeContainerArrayToFile(char *layout, Container **pTexture,
        size_t width, size_t height);
#endif /* WRITE_FILE_H */
