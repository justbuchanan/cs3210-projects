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
	// static char copied = 0;

	// if(copied) {
	// 	copied = 0;

	// 	return 0;
	// }

	// copied = 1;

	// mutex_lock(&log_mutex);

	// printk(KERN_INFO "read_log was called!\n");

	// char string[50];
	// snprintf(string, sizeof(string), "Total syscalls: %u, Log Max: %d, Avg: %llu\n",
	// 								 syscall_count, log_max_len, log_total_len / syscall_count);

	// size_t len = strlen(string);
	// size_t to_copy = count < len ? count : len;
	// if(copy_to_user(buffer, string, to_copy)) {
	// 	mutex_unlock(&log_mutex);
	// 	return -EFAULT;
	// }

	// mutex_unlock(&log_mutex);

	// return to_copy;

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
	// syscall_count++;

	// size_t len = strlen(logline);
	// if(len > log_max_len)
	// 	log_max_len = len;

	// log_total_len += len;
    // printk(KERN_INFO "Monitor: %s\n", logline);

	mutex_lock(&log_mutex);

	unsigned int nextIdx = (end + 1) % LOG_BUFFER_CAPACITY;

	if(nextIdx == start) {
		kfree(logs_buffer[start]);
		start++;
	}

	logs_buffer[end] = kmalloc(strlen(logline), GFP_KERNEL);
	memcpy(logs_buffer[end], logline, strlen(logline) + 1);
	end = nextIdx;

	mutex_unlock(&log_mutex);
}
