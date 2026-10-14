# SiCf/Ti 渐进失效模型 —— 归档复现包

本目录用于复现论文中报告的强度、寿命与可见断点空间分布结果。

- 代码版本：HexaSimulator 08bb79d（启用 TwistModel = 1，cos^3.3 theta）
- 二进制：HexaSimulator.exe（2026-10-10 19:47:53，晚于 Load.cpp 5 秒编译，与源码一致）
- 求解路径：Load.cpp:137 调度器按裂纹指针分流；全部运行日志均打印 FAIL|SideCrack_LoadRatio 或
  FAIL|SideCrack_MaxCrackLen，确认执行的是 SideCrackPlaneLoadDistribute（边裂纹路径）。

## 归档输入

TestDataC_FINAL.csv 取自 TestDataC_backup.csv，复合材料行补入退化参数：

| 参数 | F2 | F3 | BLFHT |
|---|---|---|---|
| vf | 0.45 | 0.43 | 0.36 |
| OffsetK / OffsetB | 0.2529 / -0.6441 | 0.3363 / 2.2076 | 0.3363 / 2.2076 |
| DegSigN | 2000 | 2000 | 2000 |
| DegSigD | 0.12 | 0.12 | 0.12 |
| DegT | 773 | 773 | 823 |
| SmallCrackCB | 0.5 | 0.5 | 0.5 |

测试行参数：TipSCF=2.0、TipZone=50、WakeC0=200、LLSEff=0.60、LSM=0（LLS）；
ICL 按应力分段：不低于 84% UTS 取 100 um，更低应力取 5~20 um。

## 复现步骤

    1) copy TestDataC_FINAL.csv TestDataC.csv
    2) HexaSimulator.exe > repro_final.log
    3) copy Result.csv REPRO_Result.csv ; copy Fractures.csv REPRO_Fractures.csv
    4) copy REPRO_Fractures.csv Fractures.csv
    5) FractureHollower.exe F2Report 110 146  -> rpF2_110.csv
       FractureHollower.exe F3Report 130  70  -> rpF3_130.csv

ZOffset 取 0.19 * mean(z_surface)（F2 约 110 um，F3 约 130 um）。

## 复现结果

| 指标 | 论文 | 本包复现 | 偏差 |
|---|---|---|---|
| T1 预测 UTS | 1450 MPa | 1450 MPa | 一致 |
| BT1 预测 UTS | 1700 MPa | 1700 MPa | 一致 |
| F2 寿命 | 4801 | 4801 | 一致 |
| F3 寿命 | 4382 | 4378 | -0.09% |
| F2 可见断点 | 72.2 | 71.1 | -1.5% |
| F3 可见断点 | 86.5 | 88.5 | +2.3% |
| F2 (r/d0)^2 | 94.9 | 90.4 | -4.7% |
| F3 (r/d0)^2 | 122.3 | 114.8 | -6.1% |
