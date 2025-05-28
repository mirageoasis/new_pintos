#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/process.h"
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include <lib/kernel/stdio.h>
#include <filesys/filesys.h>

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //hex_dump(f->esp, f->esp, 100, true);
  check_address(f->esp);
  uint8_t syscall_num = *((uint8_t*)(f->esp));
  int arg[10];
  switch (syscall_num) {
    case SYS_HALT:
      halt();
      break;

    case SYS_EXIT:
      get_argument(f->esp, arg, 1);
      exit(arg[0]);
      break;

    case SYS_EXEC:
      get_argument(f->esp, arg, 1);
      exec((const char*)arg[0]);
      break;

    case SYS_WAIT:
      printf("SYS_WAIT is not implemented.\n");
      ASSERT(false);
      break;
    
    case SYS_CREATE:
      get_argument(f->esp, arg, 2);
      f->eax=create((const char*) arg[0], (unsigned int) arg[1]);
      break;

    case SYS_REMOVE:
      get_argument(f->esp, arg, 1);
      f->eax=remove((const char*) arg[0]);
      break;

    case SYS_OPEN:
      printf("SYS_OPEN is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_FILESIZE:
      printf("SYS_FILESIZE is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_READ:
      get_argument(f->esp, arg, 3);
      f->eax=read(arg[0], (const void *)arg[1], (unsigned int)arg[2]);
      ASSERT(false);
      break;

    case SYS_WRITE:
      get_argument(f->esp, arg, 3);
      f->eax=write(arg[0], (const void *)arg[1], (unsigned int)arg[2]);
      break;

    case SYS_SEEK:
      printf("SYS_SEEK is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_TELL:
      printf("SYS_TELL is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_CLOSE:
      printf("SYS_CLOSE is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_MMAP:
      printf("SYS_MMAP is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_MUNMAP:
      printf("SYS_MUNMAP is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_CHDIR:
      printf("SYS_CHDIR is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_MKDIR:
      printf("SYS_MKDIR is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_READDIR:
      printf("SYS_READDIR is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_ISDIR:
      printf("SYS_ISDIR is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_INUMBER:
      printf("SYS_INUMBER is not implemented.\n");
      ASSERT(false);
      break;
    
    case SYS_FIBO:
      get_argument(f->esp, arg, 1);
      f->eax = fibonacci(arg[0]);
      break;
    
    case SYS_FOUR:
      get_argument(f->esp, arg, 4);
      f->eax = max_of_four_int(arg[0], arg[1], arg[2], arg[3]);
      break;      

    default:
      exit(-1);
      break;
  }
}

int write (int fd, const void *buffer, unsigned length){
  if(fd == 1){
    putbuf(buffer, length);
    return length;
  }else{
    printf("write fd without 1 is not implemented!\n");
    ASSERT(false);
  }
}

void halt(){
  shutdown_power_off();
}

void exit(int status){
  struct thread* t = thread_current();
  t->exit_status = status;
  printf("%s: exit(%d)\n", t->name, t->exit_status);
  thread_exit();
}

bool create(const char *file , unsigned initial_size)
{
  return filesys_create(file, initial_size);
}

bool remove(const char *file){
  return filesys_remove(file);
}

int read(int fd, void *buffer, unsigned length){
  if(fd == 0){
    uint8_t character;
    int count=0;
    while((character = input_getc()) != NULL){
      *(char*)buffer =character;
      buffer+=sizeof(uint8_t);
      count+=1;
    }
    return count;
  }else{
    printf("read with fd non-zero is not implemented!\n");
    ASSERT(false);
  }
}

pid_t exec(const char *cmd_line){
  return process_execute(cmd_line);
}

void check_address(void *addr)
{
  /* addr must not be null */
  /* addr must be null */
  /* addr must be within now process's page table list */
  if(addr == NULL) exit(-1);
  if(!is_user_vaddr(addr)) exit(-1);
  if(pagedir_get_page(thread_current()->pagedir,addr) == NULL) exit(-1);
}

void get_argument(void *esp, int *arg , int count){
  for(int i = 0; i < count; i++){
    uintptr_t offset = (i + 1) * sizeof(uintptr_t);
    check_address(esp + offset);
    // 값 넣어주기
    arg[i] = *(int*)(esp + offset);
  }
}

int max_of_four_int(int a, int b, int c, int d){
  //printf("max four: %d %d %d %d\n", a, b, c, d);
  int ret = a;
  if (b > ret)
    ret=b;
  if (c > ret)
    ret=c;
  if (d > ret)
    ret=d;
  return ret;
}

int fibonacci(int n){
  if (n == 0 || n == 1) {return 1;};
  return fibonacci(n - 1) + n;
}