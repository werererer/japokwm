#ifndef KEYBINDING_H
#define KEYBINDING_H
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

bool buttonPressed(int mod, int sym);
bool keyPressed(int mod, int sym);
#endif /* KEYBINDING_H */
