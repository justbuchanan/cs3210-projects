#ifndef __LOG_H__
#define __LOG_H__

void init_log(void);

ssize_t read_log(struct file* filp, char* buffer, size_t count, loff_t* offp);

void monitor_handler(const char* logline);

#endif
