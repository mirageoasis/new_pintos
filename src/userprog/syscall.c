#include <stdio.h>
#include <lib/user/syscall.h>
#include <syscall-nr.h>
#include "userprog/process.h"
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include <lib/kernel/stdio.h>
#include <filesys/filesys.h>
#include <filesys/file.h>

#include "vm/page.h"

static void syscall_handler(struct intr_frame *);
struct vm_entry *check_address(void *addr);
struct lock filesys_lock;

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&filesys_lock);
}

static void
syscall_handler(struct intr_frame *f UNUSED)
{
  // hex_dump(f->esp, f->esp, 100, true);
  check_address(f->esp);
  uint8_t syscall_num = *((uint8_t *)(f->esp));
  // printf("syscall number: %d\n", syscall_num);
  int arg[10];
  switch (syscall_num)
  {
  case SYS_HALT:
    halt();
    break;

  case SYS_EXIT:
    get_argument(f->esp, arg, 1);
    exit(arg[0]);
    break;

  case SYS_EXEC:
    get_argument(f->esp, arg, 1);
    check_valid_string((void *)arg[0]);
    f->eax = exec((const char *)arg[0]);
    break;

  case SYS_WAIT:
    get_argument(f->esp, arg, 1);
    f->eax = wait(arg[0]);
    break;

  case SYS_CREATE:
    get_argument(f->esp, arg, 2);
    check_valid_string((void *)arg[0]);
    f->eax = create((const char *)arg[0], (unsigned int)arg[1]);
    break;

  case SYS_REMOVE:
    get_argument(f->esp, arg, 1);
    check_valid_string((void *)arg[0]);
    f->eax = remove((const char *)arg[0]);
    break;

  case SYS_OPEN:
    get_argument(f->esp, arg, 1);
    check_valid_string((void *)arg[0]);
    f->eax = open((const char *)arg[0]);
    break;

  case SYS_FILESIZE:
    get_argument(f->esp, arg, 1);
    f->eax = filesize(arg[0]);
    break;

  case SYS_READ:
    get_argument(f->esp, arg, 3);
    check_valid_buffer((const void *)arg[1], (unsigned int)arg[2], true);
    f->eax = read(arg[0], (const void *)arg[1], (unsigned int)arg[2]);
    break;

  case SYS_WRITE:
    get_argument(f->esp, arg, 3);
    check_valid_buffer((const void *)arg[1], (unsigned int)arg[2], false);
    f->eax = write(arg[0], (const void *)arg[1], (unsigned int)arg[2]);
    break;

  case SYS_SEEK:
    get_argument(f->esp, arg, 2);
    seek(arg[0], (unsigned int)arg[1]);
    break;

  case SYS_TELL:
    get_argument(f->esp, arg, 1);
    tell(arg[0]);
    break;

  case SYS_CLOSE:
    get_argument(f->esp, arg, 1);
    close(arg[0]);
    break;

  case SYS_MMAP:
    get_argument(f->esp, arg, 2);
    f->eax = mmap((mapid_t)arg[0], (const void *)arg[1]);
    break;

  case SYS_MUNMAP:
    get_argument(f->esp, arg, 1);
    munmap((mapid_t)arg[0]);
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

int write(int fd, const void *buffer, unsigned length)
{
  int ret;
  file_descriptor_range(fd);
  lock_acquire(&filesys_lock);
  if (fd == 1)
  {
    putbuf(buffer, length);
    ret = length;
  }
  else
  {
    struct file *file_ptr;
    if (!(file_ptr = process_get_file(fd)))
    {
      lock_release(&filesys_lock);
      exit(-1);
    }
    ret = file_write(file_ptr, buffer, length);
  }
  lock_release(&filesys_lock);
  return ret;
}

void halt()
{
  shutdown_power_off();
}

void exit(int status)
{
  struct thread *t = thread_current();
  t->exit_status = status;
  printf("%s: exit(%d)\n", t->name, t->exit_status);
  thread_exit();
}

bool create(const char *file, unsigned initial_size)
{
  if (file == NULL)
    exit(-1);
  return filesys_create(file, initial_size);
}

bool remove(const char *file)
{
  return filesys_remove(file);
}

int read(int fd, void *buffer, unsigned length)
{
  int ret;
  file_descriptor_range(fd);
  check_address(buffer);
  // printf("file read %d %u\n", fd, length);
  lock_acquire(&filesys_lock);
  if (fd == 0)
  {
    // printf("fd is zero!\n");
    uint8_t character;
    int count = 0;
    while ((character = input_getc()) != NULL)
    {
      *(char *)buffer = character;
      buffer += sizeof(uint8_t);
      count += 1;
    }
    ret = count;
  }
  else
  {
    struct file *file_ptr;
    if (!(file_ptr = process_get_file(fd)))
    {
      lock_release(&filesys_lock);
      exit(-1);
    }
    ret = file_read(file_ptr, buffer, length);
  }
  lock_release(&filesys_lock);
  return ret;
}

pid_t exec(const char *cmd_line)
{
  pid_t ret = process_execute(cmd_line);
  if (ret == -1)
  {
    return -1;
  }
  struct thread *child = get_child_process(ret);
  if (!child)
  {
    exit(-1);
  }
  sema_down(&(child->load_sema));
  if (!(child->is_loaded))
  {
    return -1;
  }
  return ret;
}

int wait(pid_t pid)
{
  return process_wait(pid);
}

int open(const char *file)
{
  /* 파일을 open */
  /* 해당 파일 객체에 파일 디스크립터 부여 */
  /* 파일 디스크립터 리턴 */
  /* 해당 파일이 존재하지 않으면 -1 리턴 */
  if (!file)
    return -1;
  lock_acquire(&filesys_lock);
  struct file *file_ptr;
  if (!(file_ptr = filesys_open(file)))
  {
    lock_release(&filesys_lock);
    return -1;
  }
  int ret = process_add_file(file_ptr);
  lock_release(&filesys_lock);
  return ret;
}

int filesize(int fd)
{
  /* 파일 디스크립터를 이용하여 파일 객체 검색 */
  /* 해당 파일의 길이를 리턴 */
  /* 해당 파일이 존재하지 않으면 -1 리턴 */
  file_descriptor_range(fd);
  struct file *file_ptr;
  if (!(file_ptr = process_get_file(fd)))
    return -1;

  return file_length(file_ptr);
}

void seek(int fd, unsigned position)
{
  /* 파일 디스크립터를 이용하여 파일 객체 검색 */
  /* 해당 열린 파일의 위치(offset)를 position만큼 이동 */
  file_descriptor_range(fd);
  struct file *file_ptr = process_get_file(fd);
  ASSERT(file_ptr);
  ASSERT(position >= 0);
  file_seek(file_ptr, position);
}

unsigned tell(int fd)
{
  /* 파일 디스크립터를 이용하여 파일 객체 검색 */
  /* 해당 열린 파일의 위치를 반환 */
  file_descriptor_range(fd);
  struct file *file_ptr = process_get_file(fd);
  ASSERT(file_ptr);
  return file_tell(file_ptr);
}

void close(int fd)
{
  /* 해당 파일 디스크립터에 해당하는 파일을 닫음 */
  /* 파일 디스크립터 엔트리 초기화 */
  if (fd == 0 || fd == 1)
    exit(-1);
  file_descriptor_range(fd);
  struct thread *t = thread_current();

  struct file *file_ptr;
  if (!(file_ptr = process_get_file(fd)))
    exit(-1);

  file_close(file_ptr);
  t->fd_table[fd] = NULL;
}

mapid_t mmap(int fd, void *addr)
{
  struct file *new_file_ptr;
  int mapid;
  if (!is_user_vaddr(addr) || pg_ofs(addr) != 0 || !addr || process_get_file(fd) == NULL)
    return -1;

  if (find_vme(addr))
    return -1;

  if (!(new_file_ptr = file_reopen(process_get_file(fd))))
  {
    return -1;
  }
  struct mmap_file *m_file;
  struct vm_entry *vme;

  if ((m_file = ((struct mmap_file *)malloc(sizeof(struct mmap_file)))) == NULL)
  {
    return -1;
  }

  // init mmap
  m_file->file = new_file_ptr;
  m_file->mapid = thread_current()->next_mapid++;
  list_init(&(m_file->vme_list));
  list_push_front(&(thread_current()->mmap_list), &(m_file->elem));

  int read_bytes = file_length(m_file->file); // 읽어야할 바이트의 수
  int ofs = 0;
  while (read_bytes > 0)
  {

    /* Calculate how to fill this page.
       We will read PAGE_READ_BYTES bytes from FILE
       and zero the final PAGE_ZERO_BYTES bytes. */
    size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
    size_t page_zero_bytes = PGSIZE - page_read_bytes;

    // vm entry 생성(malloc 사용)
    struct vm_entry *vme = malloc(sizeof(struct vm_entry));
    // vm_entry 멤버들 설정, 가상페이지가 요구될 때 읽어야할 파일의 오프셋과 사이즈, 마지막에 패딩할 제로바이트 등등
    vme->type = VM_FILE;
    vme->vaddr = addr;
    vme->writable = true;
    vme->is_loaded = false;
    vme->file = m_file->file;
    vme->offset = ofs;
    vme->read_bytes = page_read_bytes;
    vme->zero_bytes = page_zero_bytes;
    list_push_back(&(m_file->vme_list), &vme->mmap_elem);
    insert_vme(&thread_current()->vm, vme);

    /* Advance. */
    read_bytes -= page_read_bytes;
    ofs += page_read_bytes;
    addr += PGSIZE;
  }

  return m_file->mapid;
}

void munmap(mapid_t mapid)
{
  struct thread *cur = thread_current();

  struct mmap_file *m_file = NULL;
  struct list_elem *e;

  for (e = list_begin(&(cur->mmap_list)); e != list_end(&(cur->mmap_list)); e = list_next(e))
  {
    struct mmap_file *m_file = list_entry(e, struct mmap_file, elem);
    ASSERT(m_file->mapid != NULL)
    if (mapid == 0)
    {
      do_munmap(m_file);
    }
    else if (m_file->mapid == mapid)
    {
      ASSERT(mapid != 0)
      do_munmap(m_file);
      return;
    }
  }
  return;
}

struct vm_entry *check_address(void *addr)
{
  if (!addr)
  {
    exit(-1);
  }

  if (addr < (void *)0x08048000 || addr >= PHYS_BASE)
  {
    exit(-1);
  }
  /*addr이 vm_entry에 존재하면 vm_entry를 반환하도록 코드 작성 */
  /*find_vme() 사용*/
  return find_vme(addr);
}

void check_valid_buffer(void *buffer, unsigned size, bool to_write)
{
  /* 인자로 받은 buffer부터 buffer + size까지의 크기가 한 페이지의
  크기를 넘을 수도 있음 */
  /* check_address를 이용해서 주소의 유저영역 여부를 검사함과 동시
  에 vm_entry 구조체를 얻음 */

  void *ptr = pg_round_down(buffer);
  for (; ptr < buffer + size; ptr += PGSIZE)
  {
    struct vm_entry *vme = check_address(ptr);
    if (vme == NULL)
    {
      exit(-1);
    }
    if (to_write && !(vme->writable))
    {
      exit(-1);
    }
  }
}

void check_valid_string(void *str)
{
  // str에 대한 vm_entry의 존재 여부를 확인`
  // check address를 사용
  struct vm_entry *vme = check_address(str);
  if (vme == NULL)
    exit(-1);

  int size = 0;
  while (((char *)str)[size] != '\0')
    size++;
  void *ptr = pg_round_down(str);
  // 인자로 받은 str도 str+ size까지의 크기가 한 페이지의 크기를 넘길 수 도 있음
  for (; ptr < str + size; ptr += PGSIZE)
  {
    vme = check_address(ptr);
    if (vme == NULL)
      exit(-1);
  }
}

void get_argument(void *esp, int *arg, int count)
{
  for (int i = 0; i < count; i++)
  {
    uintptr_t offset = (i + 1) * sizeof(uintptr_t);
    check_address(esp + offset);
    // 값 넣어주기
    arg[i] = *(int *)(esp + offset);
  }
}

int max_of_four_int(int a, int b, int c, int d)
{
  // printf("max four: %d %d %d %d\n", a, b, c, d);
  int ret = a;
  if (b > ret)
    ret = b;
  if (c > ret)
    ret = c;
  if (d > ret)
    ret = d;
  return ret;
}

int fibonacci(int n)
{
  if (n == 0 || n == 1)
  {
    return 1;
  };
  return fibonacci(n - 1) + n;
}

void file_descriptor_range(int fd)
{
  if (fd < 0 || fd >= FILE_DESCRIPTOR_MAX)
    exit(-1);
}