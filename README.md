# 如何使用本项目

本项目包含 `ch1`、`ch2`、`ch3`、`ch4` 四个实验，统一使用仓库根目录的构建与评分入口。

常用命令：
- `make`：构建全部章节和评分工具
- `make ch1`、`make ch2`、`make ch3`、`make ch4`：只构建某一章
- `make run-ch1`、`make run-ch2`、`make run-ch3`、`make run-ch4`：从仓库根目录直接运行对应程序
- `make path-ch1`、`make path-ch2`、`make path-ch3`、`make path-ch4`：打印对应二进制路径
- `make help`：查看统一的构建入口

自动评分：
- `./autograde.sh`：检查全部章节
- `./autograde.sh ch1`、`./autograde.sh ch2`、`./autograde.sh ch3`、`./autograde.sh ch4`：只检查某一章

补充说明：
- 学生需要修改的源码统一放在 `src/` 下，例如 `src/ch1/main.c`、`src/ch3/thread_pool.c`、`src/common/metrics.c`
- 第三方依赖统一放在 `third_party/` 下，例如 `third_party/stb/`
- 章节说明文档仍然保留在根目录的 `ch1/`、`ch2/`、`ch3/`、`ch4/`
- `make run-chX` 和 `autograde.sh` 会自动注入 `LAB_CHAPTER`，因此图片实验源码不需要显式传入章节名
- 相关源文件中的 `TODO` 注释给出了实现提示
