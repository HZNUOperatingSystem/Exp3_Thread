# `common/` 公共接口说明

本实验已经把图片读写、单张图处理流程、评价指标等公共逻辑放进了 `common/` 目录。  
除非老师另有说明，学生一般不需要修改 `common/` 里的代码，而是直接调用这里提供的接口。

## 数据目录约定

程序默认使用下面的目录结构：

```text
image/
  input/
  gt/
  list.txt
```

- `image/input/`：待处理图片
- `image/gt/`：对应的真值图像
- `image/list.txt`：每行一个文件名，例如 `001.png`

程序运行后还会生成：

```text
output/
metrics.csv
```

- `output/`：滤波后的输出图片
- `metrics.csv`：每张图的 PSNR、SSIM 和状态码

## `common/dataset.h`

这一组接口负责“整理图片任务”。

### `ImageJob`

```c
typedef struct {
  char name[128];
  char input_path[256];
  char gt_path[256];
  char output_path[256];
} ImageJob;
```

一个 `ImageJob` 表示一张图片对应的一次处理任务。

- `name`：文件名，例如 `001.png`
- `input_path`：输入图片路径
- `gt_path`：真值图片路径
- `output_path`：输出图片路径

### `dataset_load_jobs(...)`

```c
int dataset_load_jobs(const char* list_path,
                      const char* input_dir,
                      const char* gt_dir,
                      const char* output_dir,
                      ImageJob jobs[],
                      int max_jobs);
```

作用：
- 读取 `list.txt`
- 根据文件名拼出输入图、真值图、输出图的完整路径
- 把结果写进 `jobs[]`

返回值：
- 成功时返回任务数
- 失败时返回 `-1`

典型用法：

```c
ImageJob jobs[MAX_IMAGE_JOBS];
int job_count = dataset_load_jobs("image/list.txt",
                                  "image/input",
                                  "image/gt",
                                  "output",
                                  jobs,
                                  MAX_IMAGE_JOBS);
```

### `dataset_ensure_directory(...)`

```c
int dataset_ensure_directory(const char* path);
```

作用：
- 确保指定目录存在

返回值：
- 成功返回 `0`
- 失败返回 `-1`

典型用法：

```c
if (dataset_ensure_directory("output") != 0) {
  return 1;
}
```

## `common/image_io.h`

这一组接口负责图片的读写和内存管理。

### `ImageBuffer`

```c
typedef struct {
  int width;
  int height;
  int channels;
  unsigned char* data;
} ImageBuffer;
```

它表示一张已经读入内存的图片。

- `width`：宽度
- `height`：高度
- `channels`：通道数
- `data`：像素数据首地址

### `image_load_png(...)`

```c
int image_load_png(const char* path, ImageBuffer* out_image);
```

作用：
- 从 PNG 文件读取图片
- 自动填好 `width`、`height`、`channels`、`data`

返回值：
- 成功返回 `0`
- 失败返回 `-1`

### `image_allocate_like(...)`

```c
int image_allocate_like(const ImageBuffer* source, ImageBuffer* out_image);
```

作用：
- 按照 `source` 的尺寸和通道数，为输出图片分配一块同样大小的内存

常用于给滤波结果分配空间。

### `image_save_png(...)`

```c
int image_save_png(const char* path, const ImageBuffer* image);
```

作用：
- 把一张 `ImageBuffer` 写成 PNG 文件

### `image_free(...)`

```c
void image_free(ImageBuffer* image);
```

作用：
- 释放图片内存
- 同时把宽高、通道数和指针清零

## `common/filter.h`

这一组接口负责实际的滤波算法。

### `FilterKind`

```c
typedef enum {
  FILTER_KIND_MEDIAN = 0,
  FILTER_KIND_BILATERAL = 1,
  FILTER_KIND_CNN = 2,
} FilterKind;
```

用于表示当前要使用哪种滤波器。

### `FilterConfig`

```c
typedef struct {
  FilterKind kind;
  int median_radius;
} FilterConfig;
```

用于存放滤波参数。  
当前框架里已经提供了中值滤波示例，因此默认配置对应中值滤波。

### `filter_default_config(...)`

```c
void filter_default_config(FilterConfig* config);
```

作用：
- 给 `FilterConfig` 写入默认值

典型用法：

```c
FilterConfig config;
filter_default_config(&config);
```

### `filter_apply(...)`

```c
int filter_apply(const ImageBuffer* input,
                 ImageBuffer* output,
                 const FilterConfig* config);
```

作用：
- 根据 `config->kind` 自动选择具体滤波器

返回值：
- 成功返回 `0`
- 失败返回 `-1`

### `filter_apply_median(...)`

```c
int filter_apply_median(const ImageBuffer* input,
                        ImageBuffer* output,
                        int radius);
```

作用：
- 对整张图执行中值滤波

这一版已经由教师实现，主要用作示例和基线。

### `filter_apply_bilateral(...)`

```c
int filter_apply_bilateral(const ImageBuffer* input, ImageBuffer* output);
```

作用：
- 对整张图执行双边滤波

当前仓库里这个函数还是占位实现，后续可由老师或学生补完。

### `filter_apply_cnn(...)`

```c
int filter_apply_cnn(const ImageBuffer* input, ImageBuffer* output);
```

作用：
- 对整张图执行 CNN 滤波

当前仓库里这个函数也是占位实现，用于挑战任务接口。

## `common/metrics.h`

这一组接口负责评价指标。

### `metrics_compute_psnr(...)`

```c
int metrics_compute_psnr(const ImageBuffer* lhs,
                         const ImageBuffer* rhs,
                         double* out_value);
```

作用：
- 计算两张同尺寸图片之间的 PSNR

返回值：
- 成功返回 `0`
- 失败返回 `-1`

### `metrics_compute_ssim(...)`

```c
int metrics_compute_ssim(const ImageBuffer* lhs,
                         const ImageBuffer* rhs,
                         double* out_value);
```

作用：
- 计算两张同尺寸图片之间的 SSIM

说明：
- 当前实现是教学用的简化版本
- 学生通常不需要直接调用它，而是通过 `pipeline_process_one_image(...)` 间接使用

## `common/pipeline.h`

这一组接口负责“单张图的完整顺序处理流程”。

### `ImageResult`

```c
typedef struct {
  double psnr_before;
  double psnr_after;
  double ssim_before;
  double ssim_after;
  int status_code;
} ImageResult;
```

它表示一张图处理完成后的评价结果。

- `psnr_before`：输入图与真值图的 PSNR
- `psnr_after`：输出图与真值图的 PSNR
- `ssim_before`：输入图与真值图的 SSIM
- `ssim_after`：输出图与真值图的 SSIM
- `status_code`：处理状态码，`0` 表示成功

### `pipeline_process_one_image(...)`

```c
int pipeline_process_one_image(const ImageJob* job,
                               const FilterConfig* config,
                               ImageResult* out_result);
```

作用：
- 读取一张输入图和对应真值图
- 分配输出图内存
- 计算滤波前指标
- 执行滤波
- 保存输出图
- 计算滤波后指标

这是 `ch1` 和 `ch3` 最关键的接口。  
对学生来说，它相当于“单张图的顺序基线已经写好了”，你们只需要考虑如何并行地多次调用它。

典型用法：

```c
pipeline_process_one_image(&jobs[i], &config, &results[i]);
```

### `pipeline_write_metrics_csv(...)`

```c
int pipeline_write_metrics_csv(const char* path,
                               const ImageJob jobs[],
                               const ImageResult results[],
                               int count);
```

作用：
- 把所有图片的处理结果统一写进 `metrics.csv`

典型用法：

```c
pipeline_write_metrics_csv("metrics.csv", jobs, results, job_count);
```

## `common/stb_impl.c`

这个文件不是给学生直接调用的业务接口，它的作用是：

- 为 `stb_image.h` 提供真正的实现
- 为 `stb_image_write.h` 提供真正的实现

因为 `stb` 是单头文件库，必须有一个 `.c` 文件专门写：

```c
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
```

所以 `common/stb_impl.c` 的作用就是把 `stb` 读图、写图相关函数真正编译进项目里。

## 一次完整处理的典型流程

下面是 `ch1` 和 `ch3` 都会用到的基本流程：

```c
ImageJob jobs[MAX_IMAGE_JOBS];
ImageResult results[MAX_IMAGE_JOBS] = {0};
FilterConfig config;
int job_count;
int i;

filter_default_config(&config);

job_count = dataset_load_jobs("image/list.txt",
                              "image/input",
                              "image/gt",
                              "output",
                              jobs,
                              MAX_IMAGE_JOBS);
dataset_ensure_directory("output");

for (i = 0; i < job_count; ++i) {
  pipeline_process_one_image(&jobs[i], &config, &results[i]);
}

pipeline_write_metrics_csv("metrics.csv", jobs, results, job_count);
```

区别只在于：

- `ch1` 用 OpenMP 把这个 `for` 循环并行化
- `ch3` 用自己实现的线程池把这些图片任务并行化
