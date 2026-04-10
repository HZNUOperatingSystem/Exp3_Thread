# `ch3` 说明

## 这章做什么

用你在 `ch2` 里写好的线程池，并发处理多张图片。  
这章的重点不是重写线程池，而是把线程池接到图片任务上。

## 你需要改的文件

- `main.c`
- `thread_pool.c`

更准确地说：

- `thread_pool.c` 建议直接把你在 `ch2` 的完成版 `cherry-pick` 过来
- `main.c` 需要你把串行循环改成线程池版本

## 推荐步骤

先完成 `ch2`，提交代码。然后：

```bash
git switch ch3
git cherry-pick <你在ch2的提交号>
```

之后修改 `main.c`。

## 你要做的事

当前 `main.c` 里有一个串行基线循环。你要把它换成：

- `thread_pool_init(...)`
- 填好每个 `tasks[i]`
- `thread_pool_submit(...)`
- `thread_pool_wait(...)`
- `thread_pool_destroy(...)`

每个任务处理一张图片。

## 怎么跑

```bash
git switch ch3
./autograde.sh DATASET_DIR REFERENCE_OUTPUT_DIR
```

需要自定义超时时间时：

```bash
./autograde.sh DATASET_DIR REFERENCE_OUTPUT_DIR 60
```

`DATASET_DIR` 需要包含：

- `input/`
- `gt/`
- `list.txt`

`REFERENCE_OUTPUT_DIR` 里放参考输出图片。

## 自动评分做什么

`autograde.sh` 会：

- 编译程序
- 检查 `main.c` 是否真的调用了线程池 API
- 运行程序
- 检查 `metrics.csv`
- 逐像素比较输出图片和参考图片

如果你保留串行版本不改，自动评分会直接失败。

## 通过标准

- 能编译
- `main.c` 里确实用了线程池
- 所有输出图片正确
- `metrics.csv` 格式正确

## 常见错误

- 没把 `ch2` 的线程池实现同步过来
- 只改了 `thread_pool.c`，没改 `main.c`
- 提交完任务后忘了 `thread_pool_wait(...)`
- 用了循环里的临时变量当任务参数
