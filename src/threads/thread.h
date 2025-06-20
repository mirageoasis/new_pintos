#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "threads/synch.h"
#include <hash.h>

/* States in a thread's life cycle. */
enum thread_status
{
   THREAD_RUNNING, /* Running thread. */
   THREAD_READY,   /* Not running but ready to run. */
   THREAD_BLOCKED, /* Waiting for an event to trigger. */
   THREAD_DYING    /* About to be destroyed. */
};

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) - 1) /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0      /* Lowest priority. */
#define PRI_DEFAULT 31 /* Default priority. */
#define PRI_MAX 63     /* Highest priority. */

#define FILE_DESCRIPTOR_MAX UINT8_MAX

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
{
   /* Owned by thread.c. */
   tid_t tid;                 /* Thread identifier. */
   enum thread_status status; /* Thread state. */
   char name[16];             /* Name (for debugging purposes). */
   uint8_t *stack;            /* Saved stack pointer. */
   int priority;              /* Priority. */
   int init_priority;         /* Init Priority. for priority donation */

   struct lock *wait_on_lock;      /* save address of locks which this threads holds*/
   struct list donations;          /* list of donation_elem which points which I received prioirty donation*/
   struct list_elem donation_elem; /* pointer for donations */

   struct list_elem allelem; /* List element for all threads list. */

   int64_t wakeup_tick; /*user-defined variable to set wake up tick for blocked thread*/

   /* Shared between thread.c and synch.c. */
   struct list_elem elem; /* List element. */

   /*data for mlfq*/
   int nice;
   int recent_cpu;
   int exit_status;

   struct thread *parent;
   /* child process */
   struct list child_processes;
   struct list_elem child_process_elem;

   /*wait*/
   // seamphore for waiting child exit
   struct semaphore exit_sema;

   // seamphore for waiting child load into memeory
   struct semaphore load_sema;

   /*create success*/
   bool is_loaded;
   bool is_teminated;

   /*file*/
   struct file *fd_table[FILE_DESCRIPTOR_MAX];
   /*is not bigger then 255*/
   uint8_t fd_max_index;

   struct file *run_file;
#ifdef USERPROG
   /* Owned by userprog/process.c. */
   uint32_t *pagedir; /* Page directory. */

#endif

   /*hash table for paging*/
   struct hash vm;

   /* Owned by thread.c. */
   unsigned magic; /* Detects stack overflow. */

   struct list mmap_list;
   int next_mapid; // 다음으로 mapping될 mapid
};

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init(void);
void thread_start(void);

void thread_tick(void);
void thread_print_stats(void);

typedef void thread_func(void *aux);
tid_t thread_create(const char *name, int priority, thread_func *, void *);

void thread_block(void);
void thread_unblock(struct thread *);

struct thread *thread_current(void);
tid_t thread_tid(void);
const char *thread_name(void);

void thread_exit(void) NO_RETURN;
void thread_yield(void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func(struct thread *t, void *aux);
void thread_foreach(thread_action_func *, void *);

int thread_get_priority(void);
void thread_set_priority(int);

int thread_get_nice(void);
void thread_set_nice(int);
int thread_get_recent_cpu(void);
int thread_get_load_avg(void);

/*
user-defined function
*/

void thread_sleep(int64_t ticks);              /* make running thread to sleep */
void thread_awake(int64_t ticks);              /* remove thread from sleep queue */
void update_next_tick_to_awake(int64_t ticks); /*save least thread's tick to awake*/
int64_t get_next_tick_to_awake(void);          /* return thread.c's next_tick_to_awake */

/*
schedule running thread and highest prioirty thread by comparing priroirty
*/
void test_max_priority(void);

/* compare element a and b*/
bool cmp_priority(const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);

/*user-defined function for priority donation*/
void donate_priority(void);
void remove_with_lock(struct lock *lock);
void refresh_priority(void);

bool cmp_lock_priority(const struct list_elem *, const struct list_elem *, void *);

/*user-defined function for multi-level feedback queue*/
void mlfqs_priority(struct thread *t);
void mlfqs_recent_cpu(struct thread *t);
void mlfqs_load_avg(void);
void mlfqs_increment(void);
void mlfqs_recalc(void);

#endif /* threads/thread.h */
