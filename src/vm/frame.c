#include "vm/frame.h"
#include "userprog/pagedir.h"
#include "vm/swap.h"
#include "filesys/file.h"

struct list lru_list;
struct lock lru_list_lock;
struct list_elem *lru_clock;

void lru_list_init(void)
{
    list_init(&lru_list);
    lock_init(&lru_list_lock);
    lru_clock = NULL;
}
void add_page_to_lru_list(struct page *page)
{
    list_push_back(&lru_list, &(page->lru));
    if (lru_clock == NULL)
        lru_clock = get_next_lru_clock();
}
void del_page_from_lru_list(struct page *page)
{
    if (lru_clock == &(page->lru))        // lru clock이 가리키고 있는 page를 삭제하는 경우
        lru_clock = get_next_lru_clock(); // lru clock을 다음 것으로 바꾸어 줌
    list_remove(&(page->lru));
    if (lru_clock == NULL) // 만약 lru clock을 할당한 것이 null인 경우 list의 end인 경우일 수 있으므로 다시한번 get_next_lru_clock을 통해 체크해줌
        lru_clock = get_next_lru_clock();
}
static struct list_elem *get_next_lru_clock()
{
    if (list_empty(&lru_list))
        return NULL;

    if (lru_clock == NULL)
        return list_begin(&lru_list);

    struct list_elem *ret = list_next(lru_clock);
    if (ret == list_end(&lru_list))
        ret = NULL;
    return ret;
}
void try_to_free_pages(enum palloc_flags flags)
{
    lru_clock = get_next_lru_clock();
    if (lru_clock == NULL) // lru_clock이 초기화가 안되어 있는 경우 초기화 시켜줌
        lru_clock = get_next_lru_clock();

    if (lru_clock == NULL)
    {
        printf("lru clock NULL\n");
        struct list_elem *e;
        for (e = list_begin(&lru_list); e != list_end(&lru_list); e = list_next(e))
        {
            struct page *p = list_entry(e, struct page, lru);

            printf("page addr: %p, vage vaddr: %p, thread name: %s\n", p->kaddr, p->vme->vaddr, p->thread->name);
        }
        ASSERT(false)
    }

    // 제거할 페이지
    struct page *p = list_entry(lru_clock, struct page, lru);

    while (pagedir_is_accessed(p->thread->pagedir, p->vme->vaddr)) // 최근에 access 된경우에는 victim이 될 수 없음
    {
        pagedir_set_accessed(p->thread->pagedir, p->vme->vaddr, false); // 해당 page의 access 여부를 false로 바꾸어주고
        lru_clock = get_next_lru_clock();                               // lru clock을 다음 것으로 바꾸어 줌
        if (lru_clock == NULL)                                          // 만약 lru clock을 할당한 것이 null인 경우 list의 end인 경우일 수 있으므로 다시한번 get_next_lru_clock을 통해 체크해줌
            lru_clock = get_next_lru_clock();
        p = list_entry(lru_clock, struct page, lru);
    }

    struct vm_entry *vme = p->vme;
    switch (vme->type)
    {
    case VM_BIN:
        if (pagedir_is_dirty(p->thread->pagedir, vme->vaddr))
        {
            vme->swap_slot = swap_out(vme->vaddr);
            vme->type = VM_ANON;
        }
        break;
    case VM_FILE:
        if (pagedir_is_dirty(p->thread->pagedir, vme->vaddr))
            file_write_at(vme->file, vme->vaddr, vme->read_bytes, vme->offset);
        break;
    case VM_ANON:
        vme->swap_slot = swap_out(p->kaddr);
    }
    vme->is_loaded = false;
    __free_page(p);
}
struct page *alloc_page(enum palloc_flags flags)
{
    void *kaddr;
    struct page *p;
    // printf("ALLOC PAGE!!!\n");
    if ((p = (struct page *)malloc(sizeof(struct page))) == NULL)
    {
        return NULL;
    }
    lock_acquire(&lru_list_lock);
    while ((kaddr = palloc_get_page(flags)) == NULL)
    {
        // swap 시도
        try_to_free_pages(flags);
    }
    p->kaddr = kaddr;
    p->thread = thread_current();
    add_page_to_lru_list(p);
    ASSERT(p != NULL)
    lock_release(&lru_list_lock);
    return p;
}
void free_page(void *kaddr)
{
    struct list_elem *e;
    lock_acquire(&lru_list_lock);
    for (e = list_begin(&lru_list); e != list_end(&lru_list); e = list_next(e))
    {
        struct page *p = list_entry(e, struct page, lru);

        if (p->kaddr == kaddr)
        {
            __free_page(p);
            lock_release(&lru_list_lock);
            return;
        }
    }
    lock_release(&lru_list_lock);
    return;
}
void __free_page(struct page *page)
{
    // 리스트 제거
    del_page_from_lru_list(page);
    pagedir_clear_page(page->thread->pagedir, pg_round_down(page->vme->vaddr)); // 페이지 테이블에서 upage에 해당하는 주소의 엔트리를 제거
    palloc_free_page(page->kaddr);
    free(page);
}