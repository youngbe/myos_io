struct Cond
{
    _Atomic(struct Thread *) end;
    _Atomic(struct Thread *) head;
    spin_mtx_t spin_mutex;
};


cnd_wait()
{
    // 检查锁的类型，判断是否需要解锁
    struct Thread *current_thread;
    save_context() && cli();
    // 将锁的信息也保存在struct Thread中
    struct Thread *old_end = atomic_exchange_explicit(&cond->end, current_thread);
    if (old_end == NULL) {
        修改head为自己
    } else {
        old_end->next = 自己；
    }
    // 此时栈已经不可用
    如果需要，解锁
    切换至新线程/空线程
}

cnd_wake()
{
    cli();
    spin_lock();
    struct Thread *end = atomic_load_explicit(&cond->end, memory_order_relaxed);
    if (end == NULL) {
        spin_unlock();
        sti();
        return thrd_success;
    }

    struct Thread *head;
    while ((head = atomic_load_explicit(&cond->head, memory_order_relaxed)) == NULL) {}
    if (head != end) {
        struct Thread *new_head;
        while ((new_head = atomic_load_explicit(head->next, memory_order_relaxed)) == NULL) {}
        atomic_store_explicit(&cond->head, new_head, memory_order_relaxed);
    } else {
        if (atomic_compare_exchang_strong_explicit(&cond->end, &end, NULL)) {
            cond->head = NULL;
        } else
            goto label1;
    }
    spin_unlock();
    sti();

}

cnd_wake()
{
    struct Thread *end = atomic_load_explicit(&cond->end, memory_order_relaxed);
    if (end == NULL)
        return thrd_success;

    struct Thread *head;
    while ((head = atomic_load_explicit(&cond->head, memory_order_relaxed)) == NULL) {}
label1:
    if (head != end) {
        struct Thread *new_head;
        while ((new_head = atomic_load_explicit(head->next, memory_order_relaxed)) == NULL) {}
        atomic_store_explicit(&cond->head, new_head, memory_order_relaxed);
    } else {
        if (atomic_compare_exchang_strong_explicit(&cond->end, &end, NULL)) {
            cond->head = NULL;
        } else
            goto label1;
    }
    //
}
