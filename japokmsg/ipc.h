#ifndef _SWAY_IPC_H
#define _SWAY_IPC_H

#define event_mask(ev) (1 << (ev & 0x7F))

enum ipc_command_type {
    // i3 command types - see i3's I3_REPLY_TYPE constants
    IPC_COMMAND = 0,
};

#endif
