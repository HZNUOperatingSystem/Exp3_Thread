# ex4os

推荐优先使用自动评分命令：

```bash
make grade-ch1
make grade-ch2
make grade-ch3
make grade-ch4
make grade-all
```

说明：

- `make grade-chX` 会先运行该章节程序，再执行该章节的自动评分。
- `make grade-all` 会依次运行并评分四个章节。

如果只想看程序输出、不跑评分，可以使用：

```bash
make run-ch1
make run-ch2
make run-ch3
make run-ch4
```

说明：

- `make run-chX` 不会告诉你是否通过。
- 图片章节手动运行后会在根目录生成 `output/ch1/`、`output/ch2/`、`output/ch4/`。
- 对应的 `metrics.csv` 分别位于：
  - `output/ch1/metrics.csv`
  - `output/ch2/metrics.csv`
  - `output/ch4/metrics.csv`
- `ch3` 只测试线程池，不生成图片输出和 `metrics.csv`。

四个章节对应的主要修改文件：

- `ch1`：`src/ch1/main.c`
- `ch2`：`src/common/metrics.c`
- `ch3`：`src/ch3/thread_pool.c`
- `ch4`：`src/ch4/main.c`

环境要求：

- `bash`
- `make`
- `gcc`
- `git`
- `pthread`
- OpenMP 编译支持

macOS 下如果没有 `libomp`，请先执行：

```bash
brew install libomp
```
