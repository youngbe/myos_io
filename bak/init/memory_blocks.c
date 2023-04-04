#include "init.h"


int init_free_pages_iterator(struct Free_Pages_Iterator *iterator, const struct Free_Memory_Block *const head)
{
    if (head == NULL)
        return -1;

    assert(head->size != 0 && head->size % 0x200000 == 0);
    assert(head->addr % 0x200000 == 0);

    iterator->head = iterator->block = head;
    iterator->addr = head->addr;
    iterator->addr_max = head->addr + head->size;
    return 0;
}

const void *get_free_page(struct Free_Pages_Iterator *const it)
{
    if (it->addr < it->addr_max) {
label1:;
        void *const ret = (void *)it->addr;
        it->addr += 0x200000;
        return ret;
    }
    const struct Free_Memory_Block *next = it->block->next;

    assert(next != NULL);
    assert(next->size != 0 && next->size % 0x200000 == 0);
    assert(next->addr % 0x200000 == 0);

    if (next == it->head)
        return NULL;
    it->block = next;
    it->addr = next->addr;
    it->addr_max = next->addr + next->size;
    goto label1;
}
