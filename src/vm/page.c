#include <stdbool.h>
#include <hash.h>
#include <string.h>
#include "vm/page.h"
#include "filesys/file.h"
#include "userprog/syscall.h"

void vm_init(struct hash *vm)
{
    /* hash_init()으로 해시테이블 초기화 */
    /* 인자로 해시 테이블과 vm_hash_func과 vm_less_func 사용 */
    hash_init(vm, vm_hash_func, vm_less_func, NULL);
}

static unsigned vm_hash_func(const struct hash_elem *e, void *aux)
{
    /* hash_entry()로 element에 대한 vm_entry 구조체 검색 */
    /* hash_int()를 이용해서 vm_entry의 멤버 vaddr에 대한 해시값을 구하고 반환 */
    struct vm_entry *ve = hash_entry(e, struct vm_entry, elem);
    return hash_int(ve->vaddr);
}

static bool vm_less_func(const struct hash_elem *a, const struct hash_elem *b)
{
    /* hash_entry()로 각각의 element에 대한 vm_entry 구조체를 얻은
    후 vaddr 비교 (b가 크다면 true, a가 크다면 false */
    struct vm_entry *ve_a = hash_entry(a, struct vm_entry, elem);
    struct vm_entry *ve_b = hash_entry(b, struct vm_entry, elem);

    return ve_a->vaddr < ve_b->vaddr;
}

bool insert_vme(struct hash *vm, struct vm_entry *vme)
{
    /* hash_insert()함수 사용 */
    return hash_insert(vm, &(vme->elem)) == NULL;
}

bool delete_vme(struct hash *vm, struct vm_entry *vme)
{
    struct vm_entry *target;
    struct hash_elem *e;

    e = hash_find(vm, &(vme->elem));
    if (e == NULL)
        return false;

    target = hash_entry(e, struct vm_entry, elem);
    hash_delete(vm, e);
    free(target);
    return true;
}

struct vm_entry *find_vme(void *vaddr)
{
    /* pg_round_down()으로 vaddr의 페이지 번호를 얻음 */
    /* hash_find() 함수를 사용해서 hash_elem 구조체 얻음 */
    /* 만약 존재하지 않는다면 NULL 리턴 */
    /* hash_entry()로 해당 hash_elem의 vm_entry 구조체 리턴 */
    struct vm_entry search_entry;
    search_entry.vaddr = pg_round_down(vaddr);
    struct hash_elem *elem = hash_find(&(thread_current()->vm), &(search_entry.elem));

    if (!elem)
        return NULL;

    return hash_entry(elem, struct vm_entry, elem);
}

void vm_destroy(struct hash *vm)
{
    /* hash_destroy()으로 해시테이블의 버킷리스트와 vm_entry들을 제거 */
    hash_destroy(vm, NULL);
}

bool load_file(void *kaddr, struct vm_entry *vme)
{
    /* Using file_read() + file_seek() */
    /* 오프셋을 vm_entry에 해당하는 오프셋으로 설정(file_seek()) */
    /* file_read로 물리페이지에 read_bytes만큼 데이터를 씀*/
    /* zero_bytes만큼 남는 부분을 ‘0’으로 패딩 */
    /* file_read 여부 반환 */

    file_seek(vme->file, vme->offset);

    if (file_read(vme->file, kaddr, vme->read_bytes) != (int)(vme->read_bytes))
        return false;

    memset(kaddr + vme->read_bytes, 0, vme->zero_bytes);
    return true;
}