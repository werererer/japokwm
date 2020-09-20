#include <X11/Xlib.h>
#include <sys/types.h>
#include <xkbcommon/xkbcommon-keysyms.h>

u_int32_t StringToKeysym(char* str) {
    return XStringToKeysym(str);
}
