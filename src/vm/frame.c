#include "vm/frame.h"

void lru_list_init(void)
{
    list_init(&lru_list);
    lock_init(&lru_list_lock);
    lru_clock = NULL;
}
void add_page_to_lru_list(struct page *page)
{
    lock_acquire(&lru_list_lock);
    list_push_back(&lru_list, &(page->lru));
    lock_release(&lru_list_lock);
}
void del_page_from_lru_list(struct page *page)
{
    lock_acquire(&lru_list_lock);
    list_remove(&(page->lru));
    lock_release(&lru_list_lock);
}
static struct list_elem *get_next_lru_clock()
{
    if (list_empty(&lru_list))
        return NULL;

    struct list_elem *ret = list_next(lru_clock);
    if (ret == list_end(&lru_list))
        ret = NULL;
    return ret;
}
void try_to_free_pages(enum palloc_flags flags)
{
}
struct page *alloc_page(enum palloc_flags flags)
{
    void *kaddr;
    struct page *p;
    if ((p = (struct page *)malloc(sizeof(struct page))) == NULL)
    {
        return NULL;
    }
    if ((kaddr = palloc_get_page(flags)) == NULL)
    {
        // swap 시도
        try_to_free_pages(flags);
    }
    p->kaddr = kaddr;
    p->thread = thread_current();
    add_page_to_lru_list(p);
    ASSERT(p != NULL)
    return p;
}
void free_page(void *kaddr)
{
    struct list_elem *e;

    for (e = list_begin(&lru_list); e != list_end(&lru_list); e = list_next(e))
    {
        struct page *p = list_entry(e, struct page, lru);

        if (p->kaddr == kaddr)
        {
            __free_page(p);
            return;
        }
    }
    return;
}
void __free_page(struct page *page)
{
    // 리스트 제거
    del_page_from_lru_list(page);
    palloc_free_page(page->kaddr);
    free(page);
}