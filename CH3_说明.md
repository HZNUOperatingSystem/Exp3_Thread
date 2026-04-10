# `ch3` 说明

## 本章目标

本章的目标是：

- 继续使用 `pthread`
- 复用你在 `ch2` 中写好的线程池
- 把 30 张图片作为 30 个任务，并发处理

也就是说，本章的重点不是再写一个新的线程池，而是把“你自己的线程池”真正用到图片批处理中。

## 本章和前两章的关系

- `ch1`：体验 OpenMP 图片级并行
- `ch2`：自己实现线程池
- `ch3`：用自己实现的线程池并发处理图片

因此，完成本章前，建议你先完成 `ch2`。

## 需要修改的文件

本章通常需要改动：

- `main.c`
- `thread_pool.c`

其中：

- 如果你已经在 `ch2` 完成了线程池实现，那么 `thread_pool.c` 最推荐的做法是直接把 `ch2` 的提交 `cherry-pick` 到本分支
- 在本分支里真正要新写的是 [main.c](/home/dia/ex4os/main.c) 中“如何提交图片任务到线程池”

一般不需要改动：

- `common/` 目录
- `thread_pool.h`
- `autograde.sh`
- `Makefile`

## 教师已提供的内容

教师已经提供了：

- 数据集任务读取接口 `dataset_load_jobs(...)`
- 单张图顺序处理接口 `pipeline_process_one_image(...)`
- 批量结果写出接口 `pipeline_write_metrics_csv(...)`
- 任务结构体 `BatchTask`
- 图片任务函数 `image_job_worker(...)`

也就是说：

- 单张图怎么处理，教师已经写好
- 你要做的是把“很多张图”的处理提交给线程池并行完成

## 推荐完成顺序

推荐按下面的顺序做：

1. 先在 `ch2` 完成线程池
2. 在 `ch2` 中提交并保存你的代码
3. 切到 `ch3`
4. 使用 `git cherry-pick` 把 `ch2` 的线程池实现带过来
5. 修改 `main.c`，把串行基线改成线程池版本
6. 运行 `autograde.sh`

示例：

```bash
git switch ch2
git log --oneline
```

记下你完成线程池的 commit 号后：

```bash
git switch ch3
git cherry-pick <你的ch2提交号>
```

## `main.c` 里已经有什么

[main.c](/home/dia/ex4os/main.c) 中已经提供了：

- `BatchTask` 结构体
- `image_job_worker(void* arg)`，它内部会调用：

```c
pipeline_process_one_image(task->job, task->config, task->result);
```

这意味着每个线程池任务本质上就是：

- 处理一张图片
- 把结果写到对应的 `results[i]`

## 你需要完成什么

当前 `main.c` 中保留了一段串行基线：

```c
for (i = 0; i < job_count; ++i) {
  tasks[i].job = &jobs[i];
  tasks[i].config = &config;
  tasks[i].result = &results[i];
  image_job_worker(&tasks[i]);
}
```

你需要把它替换成线程池版本。  
推荐流程如下：

1. 声明一个 `ThreadPool pool;`
2. 调用 `thread_pool_init(&pool, 4, job_count);`
3. 填充每个 `tasks[i]`
4. 调用 `thread_pool_submit(&pool, image_job_worker, &tasks[i]);`
5. 所有任务提交完后，调用 `thread_pool_wait(&pool);`
6. 最后调用 `thread_pool_destroy(&pool);`

也就是说，本章真正要做的是把“串行 for 循环”改成“线程池提交任务”。

## 为什么 `tasks[]` 要放在 `main` 里

`tasks[i]` 会被多个工作线程异步访问。  
因此：

- 不能把任务参数写成循环里的临时局部变量
- 最安全的做法就是像当前 starter 一样，直接在 `main` 里声明一个 `BatchTask tasks[MAX_IMAGE_JOBS];`

这样这些任务参数在整个线程池执行期间都始终有效。

## 如何运行

推荐直接运行：

```bash
git switch ch3
./autograde.sh DATASET_DIR REFERENCE_OUTPUT_DIR
```

如果需要调整超时时间：

```bash
./autograde.sh DATASET_DIR REFERENCE_OUTPUT_DIR 60
```

其中：

- `DATASET_DIR` 必须包含：
  - `input/`
  - `gt/`
  - `list.txt`
- `REFERENCE_OUTPUT_DIR` 必须包含参考输出图像

## `autograde.sh` 会检查什么

本章的 `autograde.sh` 会：

1. 调用 `make` 编译 `pool_batch`
2. 静态检查 `main.c` 是否真的调用了：
   - `thread_pool_init(...)`
   - `thread_pool_submit(...)`
   - `thread_pool_wait(...)`
   - `thread_pool_destroy(...)`
3. 把数据集复制到临时目录
4. 在临时目录运行程序
5. 检查是否生成 `metrics.csv`
6. 检查 `metrics.csv` 的行数和格式
7. 把输出图片与参考输出逐像素比较

注意：

- 如果你保留 starter 里的串行版本，`autograde.sh` 会因为静态检查不通过而失败

## 通过标准

你需要满足：

- 程序可以正常编译
- `main.c` 里确实使用了线程池 API
- 所有图片都被正确处理
- 输出图片与参考结果一致
- `metrics.csv` 格式正确

## 常见错误

- 忘记先把 `ch2` 的线程池实现同步到 `ch3`
- `main.c` 里保留了串行循环，没有真正调用线程池
- `tasks[i]` 没有正确指向 `jobs[i]` 和 `results[i]`
- 提交完任务后忘记调用 `thread_pool_wait(...)`
- 销毁线程池时顺序不对
- 使用了循环里的临时变量作为任务参数，导致线程访问悬空地址

## 建议

- 先确保 `ch2` 的线程池已经稳定，再做本章
- 本章不要重写单张图处理逻辑，直接复用 `pipeline_process_one_image(...)`
- 如果你对 `common/` 里的公共接口不熟悉，可以先阅读 [COMMON_API_说明.md](/home/dia/ex4os/COMMON_API_说明.md)
