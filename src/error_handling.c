// error_handling.c
#include "error_handling.h"

void log_error(const char *format, ...) {
    va_list args;
    fprintf(stderr, "[ERROR]: ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void log_warning(const char *format, ...) {
    va_list args;
    fprintf(stderr, "[WARNING]: ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}
