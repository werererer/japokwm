#ifndef WRITE_FILE_H
#define WRITE_FILE_H
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <wlr/types/wlr_box.h>
#include "utils/stringUtils.h"

char *layoutBegin = "layout =";
int writeContainerToFile(char *file, struct wlr_fbox *box);
void writeArrayToFile(char *file, struct wlr_fbox ***arr, 
        size_t width, size_t height);
#endif /* WRITE_FILE_H */
