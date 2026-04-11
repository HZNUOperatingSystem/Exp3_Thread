# ch2

目标：
- 补全简化版 `SSIM` 计算

修改文件：
- `common/metrics.c`

要做的事：
- `images_match(...)` 和 `grayscale_value(...)` 已经提供
- 完成 `metrics_compute_ssim(...)`
- 开始实现时，先删掉 `common/metrics.c` 里为消除告警留下的 `(void)` 占位语句

运行：

```bash
./autograde.sh ch2
```
