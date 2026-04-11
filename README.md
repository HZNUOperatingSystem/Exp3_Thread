# 如何使用本项目

本项目包含 `ch1`、`ch2`、`ch3`、`ch4` 四个实验。

使用方式：
- 在仓库根目录运行 `./autograde.sh`
- 只检查某一章时，使用 `./autograde.sh ch1`、`./autograde.sh ch2`、`./autograde.sh ch3` 或 `./autograde.sh ch4`
- 自动评分通过后，会输出对应的 `pass` 信息

说明：
- `autograde.sh` 必须在仓库根目录运行，不要进入某个 `ch` 目录后再执行
- 每个 `ch` 目录下都有 `README.md`，说明本章的目标、需要修改的文件和运行方式
- 相关源文件中的 `TODO` 注释给出了实现提示
