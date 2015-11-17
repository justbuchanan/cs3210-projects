#include "class_thread.h"
#include <stdio.h>
#include <sys/syscall.h>
#include "custom_syscall.h"
#include <time.h>
#include <stdint.h>

static int syscall_num = -1;

int get_syscall_num(void) {
    while (syscall_num == -1) {
        // Read from proc file
        FILE* fp = fopen("/proc/deadlock_syscall_num", "r");
        int i;
        char c[10];
        while (fgets(c, sizeof(c), fp) != NULL);
        int sysnum = atoi(c);
        if(sysnum > 0)
          syscall_num = sysnum;
        fclose(fp);
    }
    return syscall_num;
}

int allocate_mutex(class_mutex_t *cmutex)
{
  syscall(get_syscall_num(), AllocateMutex);
  return 0;
}

int allocate_cond(class_condit_ptr ccondit)
{
  // if(!ccondit->condition)
  // {
  //   ccondit->condition = malloc(sizeof(pthread_cond_t));
    
  //   if(!ccondit->condition)
  //   {
  //     fprintf(stderr, "Error: malloc failed to allocate space for condition var!\n");
  //     return -1;
  //   }
  // }
  fprintf(stderr, "Error: allocate_cond() called\n");
  exit(1);
}


int class_mutex_init(class_mutex_ptr cmutex)
{
  // if(pthread_mutex_init(&(cmutex->mutex), NULL))
  // {
  //   fprintf(stderr, "Error: pthread mutex initialization failed!\n");
  //   return -1;
  // }
  syscall(get_syscall_num(), InitMutex);
  printf("mutex init ok\n");
  return 0;
}

int class_mutex_destroy(class_mutex_ptr cmutex)
{
  // if(pthread_mutex_destroy(&cmutex->mutex))
  // {
  //   fprintf(stderr, "Error: pthread mutex destruction failed!\n");
  //   return -1;
  // }
  syscall(get_syscall_num(), DestroyMutex);

  return 0;
}

int class_cond_init(class_condit_ptr ccondit)
{
  // if(pthread_cond_init(ccondit->condition, NULL))
  // {
  //   fprintf(stderr, "Error: pthread condition initialization failed!\n");
  //   return -1;
  // }

  // return 0;
  fprintf(stderr, "Error: class_cond_init() called\n");
  exit(1);
}

int class_cond_destroy(class_condit_ptr ccondit)
{
  // if(pthread_cond_destroy(ccondit->condition))
  // {
  //   fprintf(stderr, "Error: pthread condition destruction failed!\n");
  //   return -1;
  // }

  // return 0;

  fprintf(stderr, "Error: class_cond_destroy() called\n");
  exit(1);
}

static uint64_t totalTime = 0, min = 0, max = 0;
static int count = 0;

uint64_t gettimemicros() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

int class_mutex_lock(class_mutex_ptr cmutex)
{
  uint64_t begin = gettimemicros();

  // if(pthread_mutex_lock(&cmutex->mutex))
  // {
  //   fprintf(stderr, "Error: pthread mutex lock failed!\n");
  //   return -1;
  // }
  //printf("LIBRARY: pid: %d, tid: %d, mutex lock called\n", getpid(), syscall(SYS_gettid));

  while(syscall(get_syscall_num(), LockMutex)) {
    //printf("LIBRARY: pid: %d, tid: %d, waiting...\n", getpid(), syscall(SYS_gettid));
  }

 // printf("LIBRARY: pid: %d, tid: %d, acquired mutex lock\n", getpid(), syscall(SYS_gettid));

  uint64_t end = gettimemicros();

  uint64_t diff = end - begin;
  totalTime += diff;
  if(diff < min)
    min = diff;
  if(diff > max)
    max = diff;
  count++;

  if(count == 10000) {
    printf("TOTAL TIME: %lld\n", totalTime);
    printf("MAX TIME  : %lld\n", max);
    printf("MIN TIME  : %lld\n", min);
    printf("TOTAL LOCK: %d\n", count);
    printf("AVERAGE   : %f\n", ((double)totalTime / count));

    exit(0);
  }

  return 0;
}


int class_mutex_unlock(class_mutex_ptr cmutex)
{
  // if(pthread_mutex_unlock(&cmutex->mutex))
  // {
  //   fprintf(stderr, "Error: pthread mutex unlock failed!\n");
  //   return -1;
  // }
  //printf("LIBRARY: pid: %d, tid: %d, mutex unlock called\n", getpid(), syscall(SYS_gettid));

  syscall(get_syscall_num(), UnlockMutex);

  return 0;
}


int class_thread_create(class_thread_t * cthread, void *(*start)(void *), void * arg, int * num_of_mutexes)
{
  pthread_t temp_pthread;
  if(pthread_create(&temp_pthread, NULL, start, arg))
  {
    fprintf(stderr, "Error: Failed to create pthread!\n");
    return -1;
  }

  //Hacking a bit to get everything working correctly
  memcpy(cthread, &temp_pthread, sizeof(pthread_t));

  return 0;
}

int class_thread_join(class_thread_t cthread, void ** retval)
{
  pthread_t temp_pthread;
  memcpy(&temp_pthread, &cthread, sizeof(pthread_t));

  if(pthread_join(temp_pthread, retval))
  {
    fprintf(stderr, "Error: failed to join the pthread!\n");
    return -1;
  }

  return 0;
}

int class_thread_cond_wait(class_condit_ptr ccondit, class_mutex_ptr cmutex)
{

  if(pthread_cond_wait(ccondit->condition, &cmutex->mutex))
  {
    fprintf(stderr, "Error: pthread cond wait failed!\n");
    return -1;
  }

  return 0;
}

int class_thread_cond_signal(class_condit_ptr ccondit)
{
  
  // if(pthread_cond_signal(ccondit->condition))
  // {
  //   fprintf(stderr, "Error: pthread cond signal failed!\n");
  //   return -1;
  // }

  // return 0;

  fprintf(stderr, "Error: class_thread_cond_signal() called\n");
  exit(1);
}

