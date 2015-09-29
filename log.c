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
	logs_buffer = kmalloc(LOG_BUFFER_CAPACITY * sizeof(const char*), GFP_KERNEL);
	start = end = 0;

	printk(KERN_INFO "Init log buffer\n");
}

// /proc/log
ssize_t read_log(struct file* filp, char* buffer, size_t count,
                    loff_t* offp) {
	static size_t num_left = 0;

	mutex_lock(&log_mutex);

	if(start == end) {
		num_left = 0;
		mutex_unlock(&log_mutex);
		return 0;
	}

	ssize_t total_copied = 0;

	while(start < end) {
		size_t len = strlen(logs_buffer[start]);
		size_t to_copy = num_left ? num_left : len;
		size_t copied = count < len ? count : len;
		if(copy_to_user(buffer, logs_buffer[start] + len - to_copy, copied)) {
			mutex_unlock(&log_mutex);
			return -EFAULT;
		}

		num_left = len - copied;
		if(!num_left) {
			kfree(logs_buffer[start]);
			start = (start + 1) % LOG_BUFFER_CAPACITY;
		}

		total_copied += copied;
	}

	mutex_unlock(&log_mutex);

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
		
		start++;
	}

	size_t len = strlen(logline) + 1;
	logs_buffer[end] = kmalloc(len, GFP_KERNEL);
	memcpy(logs_buffer[end], logline, len);
	end = nextIdx;

	mutex_unlock(&log_mutex);
}
