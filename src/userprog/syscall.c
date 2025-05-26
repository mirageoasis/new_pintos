#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  hex_dump(f->esp, f->esp, 100, true);
  uint8_t syscall_num = *((uint8_t*)(f->esp));
  switch (syscall_num) {
    case SYS_HALT:
      printf("SYS_HALT is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_EXIT:
      printf("SYS_EXIT is not implemented.\n");
      ASSERT(false);
      break;

    case SYS_EXEC:
      printf("SYS_EXEC is not implemented.\n");
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
      ASSERT(false);
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
