切换线程步骤：

1. 如果原线程是用户态线程，增加原进程虚拟线程数
2. 保存上下文
3. 将线程放置到阻塞/调度链表中
=== 1. 2. 步骤顺序可互换 ====
=== switch\_to() 函数做以下步骤 ===
4. 如果新线程是用户态线程，减少原进程虚拟线程数
5. 对比新老线程的进程，不同则切换%cr3
6. 切换到新线程执行

4. 如果没有新线程，增加空闲处理器数量，处理器进入空闲状态

empty\_ 前缀：不增加空闲处理器数量/帮你减少空闲处理器数量

\_interrupt后缀：同无后缀版本，只是会wrmsr写入EOI

```c
switch_to    // 主动切换线程 thrd_yield
switch_to_empty  // 线程结束/阻塞 mtx_lock cnd_wait
switch_to_interrupt  // 中断，抢占式切换线程   时钟中断
switch_to_empty_interrupt   // 线程被kill    时钟中断
empty_switch_to_interrupt   // 空线程被中断唤醒，忙起来了   I/O中断唤醒中断线程
empty_switch_to_empty_interrupt // 空线程忙起来了，尝试找事做   空线程时钟中断
interrupt_switch_to_interrupt // 由中断服务程序调用，想要切换线程：先判断当前是否有线程，有则返回，无（空线程）则运行empty_switch_to_interrupt    I/O中断
```
