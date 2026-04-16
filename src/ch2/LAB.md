# ch2

目标：
- 补全简化版 `SSIM` 计算

修改文件：
- `src/common/metrics.c`

要做的事：
- `images_match(...)` 和 `grayscale_value(...)` 已经提供
- 变量声明、基本检查、第一轮循环骨架、灰度转换、`pixel_count`、`C1/C2`、归一化和最终 `SSIM` 公式已经提供
- 完成 `metrics_compute_ssim(...)` 里的两轮统计
- 开始实现时，先删掉第一轮循环里为消除告警留下的 `(void)` 占位语句

运行：

```bash
make grade-ch2
```
