# ch1

目标：
- 用 OpenMP 并行处理多张图片

修改文件：
- `src/ch1/main.c`

要做的事：
- 把 `IMAGE_PARALLEL_FOR` 改成 OpenMP 版本
- 这一章只检查 `PSNR`
- `output/ch1/metrics.csv` 里的 `SSIM` 应该保持为 `N/A`
- `output/ch1/metrics.csv` 里的数据怎么看会在 `ch2` 讲解

运行：

```bash
./autograde.sh ch1
```
