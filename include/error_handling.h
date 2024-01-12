// error_handling.h
#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <stdio.h>
#include <stdarg.h>

// Funktion zur Protokollierung von Fehlermeldungen
void log_error(const char *format, ...);

// Funktion zur Protokollierung von Warnmeldungen
void log_warning(const char *format, ...);

#endif // ERROR_HANDLING_H
