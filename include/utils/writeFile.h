#ifndef WRITE_FILE_H
#define WRITE_FILE_H
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <wlr/types/wlr_box.h>

#include "utils/stringUtils.h"
#include "render/render.h"

void writeContainerToFile(int fd, Container box);
void writeContainerArrayToFile(int fd, Container box[], size_t length);
/* returns path to newly created directory */
#endif /* WRITE_FILE_H */
