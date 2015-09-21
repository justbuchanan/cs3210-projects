#pragma once

// Callback function type.
typedef void (*MonitorEventHandler)(const char* logline);

int monitor_init(MonitorEventHandler handler);
void monitor_cleanup(void);

// Set the ID of the user to monitor syscalls for
// void monitor_set_uid(int uid);
