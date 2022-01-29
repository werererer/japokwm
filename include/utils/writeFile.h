#ifndef WRITE_FILE_H
#define WRITE_FILE_H
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <wlr/util/box.h>

#include "utils/stringUtils.h"
#include "render/render.h"

int write_to_file(int fd, const char *content);
void write_container_to_file(int fd, struct wlr_fbox box);
void write_container_array_to_file(int fd, struct wlr_fbox box[], size_t length);

struct wlr_box string_to_wlr_box(const char *str);
/* returns path to newly created directory */
#endif /* WRITE_FILE_H */
