struct Mutex
{
    _Atomic(struct Thread *) blocked_end;
    // 仅用来判断是不是自己，为NULL可能也有其它线程在占用
    // 但是不为NULL，一定有其他线程在占用
    _Atomic(struct Thread *) owner;
    size_t count;
};

try_lock (struct Mutex *mutex)
{
    struct Thread *current_thread;
    if (atomic_load_explicit(&owner, memory_order_relaxed) == current_thread) {
        if (mutex->count >= 1) {
            if (mutex->count == SIZE_MAX)
                return thrd_busy;
            ++mutex->count;
            return thrd_success;
        }
        return thrd_error;
    }
    current_thread->temp0 = NULL;
    struct Thread *temp = NULL;
    if (atomic_compare_exchange_strong_explicit(&mutex->blocked_end, &temp, current_thread, memory_order_acquire, memory_order_relaxed)) {
        atomic_store_expicit(&mutex->owner, current_thread, memory_order_acq_rel);
        return thrd_success;
    }
    return thrd_busy;
}

lock (struct Mutex *mutex)
{
    try_lock();
    save_context(); && cli();
    current_thread->temp0 = NULL;
    struct Thread *old_end;
    atomic_exchange_strong(&mutex->blocked_end, &old_last);
    if (old_end == NULL) {
        sti();
        atomic_store_expicit(&mutex->owner, current_thread, memory_order_acq_rel);
        pop_cntext();
        return thrd_success;
    }
    old_last->next = current_thread;
    // 不能用栈
    切换至新线程运行();
}

unlock()
{
    owner设为NULL;
    if (atomic_compare_exchange_strong(&mutex->blocked_end, NULL)) {
        return thrd_success;
    }
    自旋等待temp0改
    将temp0设为可调度

}
