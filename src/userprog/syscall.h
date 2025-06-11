#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "stdbool.h"

typedef int pid_t;

void syscall_init(void);
void halt(void);
void exit(int status);
pid_t exec(const char *file);
int wait(pid_t);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
int filesize(int fd);
int read(int fd, void *buffer, unsigned length);
int write(int fd, const void *buffer, unsigned length);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);
int fibonacci(int n);
int max_of_four_int(int a, int b, int c, int d);

struct vm_entry *check_address(void *addr);

extern struct lock filesys_lock;

#endif /* userprog/syscall.h */
