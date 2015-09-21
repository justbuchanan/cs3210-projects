#pragma once

// Callback function type.  Implementations must copy the contents of @logline
// if they need it, because it may be written over as soon as the callback returns.
typedef void (*MonitorEventHandler)(const char* logline);

int monitor_init(MonitorEventHandler handler);
void monitor_cleanup(void);

// Get and set the uid that is being monitored.  Syscalls made under other uids
// are ignored.
void monitor_set_uid(int uid);
int monitor_get_uid(void);
