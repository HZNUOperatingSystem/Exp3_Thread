---
title: '实验三 · 线程管理'
icon: ChartNoAxesGantt
description: 从编译器指令到手搓线程池，一步步掌握多线程并行编程
---

## 实验概览

本实验围绕**多线程并行编程**展开，从使用现成工具到亲手造轮子，逐步深入操作系统的线程管理核心。你将经历一条完整的学习路线：先用编译器指令体验并行的威力，再理解图像处理流水线的评估方法，然后从零实现一个线程池，最后用它驱动真正的并行任务。

## 章节导读

| 章节 | 主题 | 你会学到 |
|:---|:---|:---|
| **第一章 · OpenMP** | 编译器级并行 | `parallel for`、reduction、schedule 策略、缓存一致性 |
| **第二章 · 图像质量评估** | 算法与度量 | 中值滤波、双边滤波、MSE / PSNR / SSIM |
| **第三章 · 动手写线程池** | 从零造轮子 | 函数指针、互斥锁、条件变量、生产者-消费者模型 |
| **第四章 · 并行流水线** | 整合实战 | 用你的线程池并行处理图像批任务 |

## 如何使用本实验？

以下命令都需要在**仓库根目录**执行。完成每一章节后, 使用`make grade-chX`命令进行自动评测, 所有章节都不需要手动编译任何代码文件。具体命令如下：

```bash
make grade-ch1
make grade-ch2
make grade-ch3
make grade-ch4
make grade-ch4x
make grade-all
```

说明：
- `make grade-all` 会依次运行并评分全部章节, 包括扩展实验 `ch4x`。
- `make run-chX` 可以直接运行对应章节, `make run-all` 会按顺序运行全部章节。
- 和图片处理相关的章节运行后, 会在根目录生成 `output/ch1/`、`output/ch2/`、`output/ch4/`、`output/ch4x/`。
- 对应的 `metrics.csv` 分别位于：
  - `output/ch1/metrics.csv`
  - `output/ch2/metrics.csv`
  - `output/ch4/metrics.csv`
  - `output/ch4x/metrics.csv`
- `ch3` 只测试线程池，不生成图片输出和 `metrics.csv`。

<Callout title="每章该改哪个文件？" type="info">

| 章节 | 主要修改文件 |
|:---|:---|
| ch1 | `src/ch1/main.c` |
| ch2 | `src/common/metrics.c` |
| ch3 | `src/ch3/thread_pool.c` |
| ch4 | `src/ch4/main.c` |
| ch4x | `src/ch4x/filter_cnn.c`、`src/ch4x/onnx_inference.c` |

**提交作业时, 使用 `make submit`, 该命令会将 `src` 文件夹统一打包成 `submit.tar.gz`**

</Callout>

<Callout title="环境要求" type="warn">

需要 `bash`、`make`、`gcc`、`git`、`pthread` 以及 OpenMP 编译支持。

macOS 下如果没有 `libomp`，请先执行：

```bash
brew install libomp
```

</Callout>

<Callout title="建议阅读顺序" type="info">

按 **第一章 → 第二章 → 第三章 → 第四章** 的顺序阅读。第一章和第二章相对独立，但第四章的代码依赖第三章实现的线程池。如果你对 OpenMP 已经熟悉，可以快速浏览第一章直接进入后续章节。

</Callout>
