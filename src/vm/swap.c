#include "devices/block.h"

#include "vm/swap.h"
#include "vm/frame.h"
#include "vm/page.h"

struct bitmap *swap_bitmap;

const size_t BLOCK_PER_PAGE = PGSIZE / BLOCK_SECTOR_SIZE;

void swap_init()
{
    swap_bitmap = bitmap_create(8 * 1024);
}

/*used_index의 swap slot에 저장된 데이터를 논리 주소 kaddr로 복사*/
void swap_in(size_t used_index, void *kaddr)
{
    // swap용 disk block을 가져오기
    struct block *b = block_get_role(BLOCK_SWAP);
    if (bitmap_test(swap_bitmap, used_index)) // used_index에 해당되는 swap영역이 사용되고 있다면
    {
        for (int i = 0; i < BLOCK_PER_PAGE; i++)
        {
            //(page당 block 개수 * used_index)(used_index전까지의 block 섹터 개수) + i
            block_read(b, BLOCK_PER_PAGE * used_index + i, BLOCK_SECTOR_SIZE * i + kaddr);
            // block들을 읽어서 물리 페이지에 로드함
        }
        bitmap_reset(swap_bitmap, used_index);
    }
}

/*kaddr 주소가 가리키는 페이지를 스왑 파티션에 기록*/
/*페이지를 기록한 swap slot 번호를 리턴*/
size_t swap_out(void *kaddr)
{
    struct block *swap_disk = block_get_role(BLOCK_SWAP);
    size_t swap_idx = bitmap_scan(swap_bitmap, 0, 1, false);
    if (swap_idx == BITMAP_ERROR)
        return BITMAP_ERROR;

    for (int i = 0; i < BLOCK_PER_PAGE; i++)
    {
        block_write(swap_disk, BLOCK_PER_PAGE * swap_idx + i, BLOCK_SECTOR_SIZE * i + kaddr);
    }
    bitmap_set(swap_bitmap, swap_idx, true);

    return swap_idx;
}
