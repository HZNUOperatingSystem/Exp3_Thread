# ch3

目标：
- 用 `pthread` 实现线程池

修改文件：
- `src/ch3/thread_pool.c`

要做的事：
- `thread_pool_init(...)` 里的参数检查、分配内存和 `pthread_create(...)` 循环已经提供
- 完成 `thread_pool_worker(...)`
- 完成 `thread_pool_submit(...)` 里剩下的队列逻辑
- 完成 `thread_pool_wait(...)`
- 完成 `thread_pool_destroy(...)`
- 开始实现时，先删掉 starter 里为消除告警留下的 `(void)` 占位语句

运行：

```bash
./autograde.sh ch3
```
