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
  uint8_t syscall_num = *((uint8_t*)(f->esp));
  switch (syscall_num) {
    case SYS_HALT:
      halt();
      ASSERT(false);
      break;

    case SYS_EXIT:
      exit(1);
      ASSERT(false);
      break;

    case SYS_EXEC:
      exec("echo");
      ASSERT(false);
      break;

    case SYS_WAIT:
      printf("SYS_WAIT is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_CREATE:
      printf("SYS_CREATE is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_REMOVE:
      printf("SYS_REMOVE is not implemented.\n");
      ASSERT(false);
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
      printf("SYS_READ is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_WRITE:
      printf("SYS_WRITE is not implemented.\n");
      //write();
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

    default:
      printf("Unknown system call: %d\n", syscall_num);
      ASSERT(false);
      break;
  }
  thread_exit ();
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
  printf("%s: exit(%d)", t->name, t->exit_status);
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
  if(pagedir_get_page(thread_current()->pagedir,addr) != NULL) exit(-1);
}