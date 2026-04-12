# 写在开始前

恭喜来到 `ch2`。不用担心, 此文档没有理解难度, 相信我

做完 `ch1` 之后，大家可能会好奇：`ch1` 的 `main.c` 到底在做什么？  
```c
#include "common/pipeline.h"

static int execute_jobs(const ImageJob jobs[], const FilterConfig* config, ImageResult results[],
                        int job_count) {
  int i;

  for (i = 0; i < job_count; ++i) {
    pipeline_process_one_image(&jobs[i], config, 1, &results[i]);
  }

  return 0;
}

int main(void) {
  return pipeline_run_image_batch(execute_jobs);
}
```

我们可以看到，`pipeline_process_one_image()` 被当作一个 job，通过
`pipeline_run_image_batch()` 统一调度。

`pipeline_process_one_image()` 这个函数，究竟是怎么 “process one image” 的？

如果你在 IDE 里按住 `Ctrl` 再点击 `pipeline_process_one_image()`，一般就能直接跳到它的定义位置，也就是 `pipeline.c`。

在 `ch2` 里，我们主要关心两部分：

- `metrics_compute_psnr()`
- 当 `compute_ssim` 为非零时执行的 `metrics_compute_ssim()`

不过在进入 `SSIM` 之前，我们先把整条处理流程顺一遍。

## 整体流程

`pipeline_process_one_image()` 大致会做这些事：

1. 检查输入参数是否合法。
2. 清空输出结果结构，避免旧数据污染结果。
3. 读取输入图片和真值图片。
4. 检查两张图片的尺寸和通道数是否一致。
5. 为输出图片分配缓冲区。
6. 先计算一次滤波前的指标。
7. 对图片执行滤波。
8. 保存滤波后的图片。
9. 再计算一次滤波后的指标。

前半部分对应的代码大致是：

```c
if (job == NULL || config == NULL || out_result == NULL) {
  return -1;
}

pipeline_reset_result(out_result);

if (image_load_png(job->input_path, &input) != 0 ||
    image_load_png(job->gt_path, &gt) != 0) {
  out_result->status_code = PIPELINE_STATUS_LOAD;
  image_free(&input);
  image_free(&gt);
  return -1;
}

if (input.width != gt.width || input.height != gt.height ||
    input.channels != gt.channels) {
  out_result->status_code = PIPELINE_STATUS_SHAPE;
  image_free(&input);
  image_free(&gt);
  return -1;
}

if (image_allocate_like(&input, &output) != 0) {
  out_result->status_code = PIPELINE_STATUS_LOAD;
  image_free(&input);
  image_free(&gt);
  return -1;
}
```

你会发现，读代码其实没有那么可怕。  
很多时候，只要先从函数名、参数和返回值入手，就已经能大致看懂一段代码在做什么。至于函数内部更细的实现，等需要时再深挖就够了。

## 为什么要算两次 PSNR

代码里会执行两次 `metrics_compute_psnr()`：

- 第一次：

```c
metrics_compute_psnr(&input, &gt, &out_result->psnr_before);
```

- 第二次：

```c
metrics_compute_psnr(&output, &gt, &out_result->psnr_after);
```

两者长得很像，但第三个参数不同：

- 第一次写入 `psnr_before`
- 第二次写入 `psnr_after`

这中间发生了什么？  
中间其实就做了两件重要的事：

```c
if (filter_apply(&input, &output, config) != 0) {
  out_result->status_code = PIPELINE_STATUS_FILTER;
  image_free(&input);
  image_free(&gt);
  image_free(&output);
  return -1;
}

if (image_save_png(job->output_path, &output) != 0) {
  out_result->status_code = PIPELINE_STATUS_SAVE;
  image_free(&input);
  image_free(&gt);
  image_free(&output);
  return -1;
}
```

`filter_apply(&input, &output, config) != 0` 的意思是：

- 如果 `filter_apply()` 返回 `0`，说明滤波执行成功。
- 如果返回值不是 `0`，就说明出错了，于是程序会做清理工作，并提前返回。

这里的清理工作主要就是 `image_free(...)`。如果感兴趣，也可以继续点进去看看图片缓冲区是怎么释放的。

## 中值滤波是什么

当前这套默认配置里，`filter_apply()` 最终会走到中值滤波。

那什么是中值滤波？

首先有一个参数叫 `median_radius`，也就是“半径”。  
如果 `median_radius = 1`，那么对某个像素来说，我们就会看它周围半径为 1 的区域，也就是一个 `3x3` 的窗口，总共 9 个点。

既然叫“中值”滤波，那么做法就是：

1. 取出这 9 个点的像素值。
2. 排序。
3. 取排序后的中间值。
4. 用这个中间值替换中心像素。

为什么这样能去噪？

因为噪点往往和周围像素差异很大。比如在暗光环境拍照时，常常会出现一些特别亮或者特别暗的点。它们在局部区域里往往是“特别大”或者“特别小”的离群值，不太可能落在中位数附近。

例如下面这个 `3x3` 区域：

```text
10  11  12
12 250  13
11  12  10
```

这里中间的 `250` 很像一个噪点。  
排序后取中间值，中心像素就可能变成 `12`，结果大概可以理解成：

```text
10  11  12
12  12  13
11  12  10
```

这样噪点被压下去了。

当然，中值滤波也有代价：  
它会降低一些细节和锐度。因为它对整张图每个像素都做类似操作，所以有些本来正常的细节也可能被“磨平”。

## 双边滤波是什么

双边滤波想解决的问题，正是中值滤波容易损失细节这一点。

它不会简单地“排序然后取中间值”，而是会给邻域里的每个像素一个权重，最后按加权平均得到结果。可以粗略理解成：

```text
新的像素值 = Σ(邻域像素值 × 对应权重) / Σ(所有权重)
```

直觉上可以这样理解：

- 如果某个像素离中心像素比较远，那么它的权重会更小。
- 如果某个像素和中心像素差异比较大，那么它的权重也会更小。
- 如果某个像素既离得近、又和中心像素更接近，那么它的权重就会更大。

所以双边滤波更擅长做的事情是：

- 平滑掉一部分噪声
- 同时尽量保住原本的边缘和层次感

它和中值滤波最大的不同，不是“去掉离群值的方式不同”这么简单，而是它会尽量避免把边缘也一起抹平。

所以更准确地说：

- 中值滤波对椒盐噪声这类离群点通常更有效
- 双边滤波更强调“去噪的同时保边”

这也是为什么双边滤波通常比中值滤波“更自然”，但实现和计算都会更复杂一些。

## 为什么 RGB 最后变成了一个值

可能会有人疑惑：“据我了解，一个像素不是有 `RGB` 三个通道吗？为什么这里最后只剩下一个值？”

因为我们在这个实验里，拿来做 `SSIM` 计算的不是完整彩色信息，而是**灰度值**。也就是把一个像素从
“红、绿、蓝三个通道”，压缩成“从黑到白的一个亮度值”。

常见的灰度化写法大致是：

$$
\mathrm{gray} = 0.299R + 0.587G + 0.114B
$$

这一步当然会损失一部分颜色信息，但对这个实验来说是值得的。因为我们现在的重点不是研究颜色空间，而是先把 `SSIM`
的基本计算逻辑走通。

如果真的要对彩色图做更完整的相似度计算，也不是没有办法。比如：

- 分别对 `R`、`G`、`B` 三个通道计算，再把结果合并；
- 或者先转换到别的颜色空间，再重点比较亮度通道。

这些都属于很好的延伸问题。如果你好奇，可以继续问 AI：

“如果不想把 `RGB` 直接压成一个灰度值，还能怎么计算两张彩色图的相似度？”

这就是很典型的探索型问题。

## 为什么需要 SSIM

回到主题。我们现在有：

- 一张带噪声的输入图；
- 一张经过滤波后的输出图；
- 一张没有噪声的真值图，也就是 `gt`。

那我们怎么判断“滤波后是不是更好了”？

只靠肉眼看，有时会很主观。所以我们需要一个比较稳定的指标，去衡量：

- 输入图和真值图有多接近；
- 输出图和真值图又有多接近。

如果滤波后的结果更接近真值图，那么就说明这次滤波整体上是有效的。

## SSIM 是什么

`SSIM` 的全称是 `Structural Similarity Index Measure`，一般翻译成“结构相似性指标”。

它的常见公式写成：

$$
\mathrm{SSIM}(x, y) =
\frac{(2\mu_x\mu_y + C_1)(2\sigma_{xy} + C_2)}
{(\mu_x^2 + \mu_y^2 + C_1)(\sigma_x^2 + \sigma_y^2 + C_2)}
$$

第一次看到这个式子觉得吓人很正常。先别急着背，先看每个符号是什么意思：

- $\mu_x$、$\mu_y$：两张图的平均值，可以粗略理解成整体亮度；
- $\sigma_x^2$、$\sigma_y^2$：两张图的方差，可以粗略理解成对比度变化；
- $\sigma_{xy}$：两张图的协方差，可以粗略理解成结构相似性；
- $C_1$、$C_2$：两个稳定常数，用来避免分母太小甚至为零。

在 8-bit 图像里，常见写法是：

$$
C_1 = (0.01 \times 255)^2 \approx 6.5025,\qquad
C_2 = (0.03 \times 255)^2 \approx 58.5225
$$

## 先理解三个关键词

### 平均值

平均值反映的是一组数据整体偏亮还是偏暗。

如果把一张灰度图看成很多个像素值，那么平均值大，通常说明整张图整体更亮；平均值小，通常说明整张图整体更暗。

公式是：

$$
\mu_x = \frac{1}{N} \sum_{i=1}^{N} x_i
$$

### 方差

方差衡量的是“一组数据波动得厉不厉害”。

放到图像里，可以粗略理解成：像素值变化越剧烈，方差往往越大；像素值越集中，方差往往越小。

所以你也可以把它理解成一种“对比度强弱”的近似描述。

公式是：

$$
\sigma_x^2 = \frac{1}{N} \sum_{i=1}^{N} (x_i - \mu_x)^2
$$

### 协方差

协方差衡量的是“两组数据是不是在一起变化”。

放到图像里，就是看两张图对应位置的像素，是否有相近的变化趋势：

- 协方差大于 `0`：通常说明两张图在很多位置上“同涨同跌”；
- 协方差接近 `0`：通常说明两张图之间关系不强；
- 协方差小于 `0`：通常说明两张图在一些位置上的变化趋势相反。

公式是：

$$
\sigma_{xy} = \frac{1}{N} \sum_{i=1}^{N} (x_i - \mu_x)(y_i - \mu_y)
$$

如果把上面三件事合在一起看，你就会发现：

- 平均值更像在比较“亮度”；
- 方差更像在比较“对比度”；
- 协方差更像在比较“结构”。

所以 `SSIM` 不只是简单比较两个像素差了多少，它是在同时比较“亮度、对比度、结构”这三方面。

## 放回到代码里看

在 `ch2` 里，我们已经把 `SSIM` 的整体框架和最终公式提供好了。你真正需要补的，主要就是中间那两轮统计：

1. 第一轮遍历，累计 `mean_x` 和 `mean_y`。这里的 `mean` 就是“平均值”的意思。
2. 第二轮遍历，累计 `var_x`、`var_y` 和 `cov_xy`。其中 `var` 表示方差，`cov` 表示协方差。

也就是说，这一章最重要的关注点还是：

- `metrics_compute_ssim()`
- 里面的 `TODO`

别的代码不是强制要求去读，感兴趣当然可以继续点进去看。

## 最后补一句

我们实验里的 `SSIM` 是一个**简化版本**：

- 先把彩色图转成灰度图；
- 再按整张图去统计平均值、方差和协方差。

更标准、也更常见的做法，通常会在局部窗口上计算这些量，有时还会配合高斯核等方式来做得更细、更稳。

不过对当前这个实验来说，简化版已经足够帮助大家理解 `SSIM` 的核心思想了。
