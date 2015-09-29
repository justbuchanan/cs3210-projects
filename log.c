#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#define LOG_BUFFER_CAPACITY 20000
char** logs_buffer;
unsigned int start, end;

struct mutex log_mutex;

unsigned int syscall_count = 0;
unsigned int log_max_len = 0;
unsigned long long log_total_len = 0;

void init_log(void) {
	mutex_init(&log_mutex);
	logs_buffer = kcalloc(LOG_BUFFER_CAPACITY, sizeof(char*), GFP_KERNEL);
	start = end = 0;

	printk(KERN_INFO "Init log buffer\n");
}

// /proc/log
ssize_t read_log(struct file* filp, char* buffer, size_t count,
                    loff_t* offp) {
	mutex_lock(&log_mutex);

	if(start == end) {
		mutex_unlock(&log_mutex);
		return 0;
	}

	ssize_t total_copied = 0;

	while(start != end) {
		size_t len = strlen(logs_buffer[start]);

		if(total_copied + len > count)
			break;

		if(copy_to_user(buffer + total_copied, logs_buffer[start], len)) {
			mutex_unlock(&log_mutex);
			return -EFAULT;
		}

		kfree(logs_buffer[start]);
		start = (start + 1) % LOG_BUFFER_CAPACITY;

		total_copied += len;
	}

	mutex_unlock(&log_mutex);

	printk(KERN_INFO "Read from the log.\n");

	return total_copied;
}

void monitor_handler(const char* logline) {
	static char printed = 0;

	mutex_lock(&log_mutex);

	unsigned int nextIdx = (end + 1) % LOG_BUFFER_CAPACITY;

	if(nextIdx == start) {
		kfree(logs_buffer[start]);

		if(!printed) {
			printk(KERN_INFO "STARTED TO FREE!\n");
			printed = 1;
		}
		
		start = (start + 1) % LOG_BUFFER_CAPACITY;
	}

	size_t len = strlen(logline) + 1;
	logs_buffer[end] = kmalloc(len, GFP_KERNEL);
	memcpy(logs_buffer[end], logline, len);
	end = nextIdx;

	mutex_unlock(&log_mutex);
}
