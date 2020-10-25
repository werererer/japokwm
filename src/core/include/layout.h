#ifndef LAYOUT_H
#define LAYOUT_H

#include <julia.h>

struct layout {
    char *symbol;
    jl_function_t *arrange;
};

#endif /* LAYOUT_H */
