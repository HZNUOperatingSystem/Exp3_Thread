# ch4x part-1

在本部分中，你将尝试使用预训练 ONNX 模型替代传统滤波器处理图片。

你需要补全 `src/ch4x/` 中与 ONNX 推理相关的代码。

建议先阅读并修改：

- `src/ch4x/filter_cnn.c`
- `src/ch4x/onnx_inference.c`

运行方式

```bash
./autograde.sh grade-ch4x
```

你需要自行决定：

- 模型文件路径如何组织
- ONNX Runtime session 如何初始化
- 输入输出 tensor 如何转换
