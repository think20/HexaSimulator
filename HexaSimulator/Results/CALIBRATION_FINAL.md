# HexaSimulator 完整标定总结

## 一、参数分类

### [实验] 直接测量 — 18 个
Sigma0, mf, Ef, Em, R0, Vf, SpecimenRadious, Sigmams, Cm[0], Cm[1], Mm[0], Mm[1], KLow, KIC, SlidingStrength0, DeltaT, dint, L, Lg, SctDecay, Spectra, MaxLoad

### [理论] 模型推导 — 8 个  
c₀(WakeC0)=200μm, sigma_f_wake=σ_comp/Vf, sigma_f_intact=Ef·σ_comp/(Ef·Vf+Em·Vm), LLSIMax=1, MaxCrackRatio=0.50, dbFactor=1.6, dcFactor=1.0, MaxLoadRatio=0.53

### [扫参] 标定确定 — 6 个 (HT) + 2 个 (RT)

| 参数 | HT值 | RT值 | 文献范围 | 敏感度 |
|------|------|------|---------|--------|
| StgDecay | 0.85 | 0.855 | 0.72-0.90 | 中 |
| TipSCF | 1.5 | 3.0* | 1.02-2.5 + 3D FEM=2.13 | 高(HT) / 无(RT) |
| TipZone | 50μm | 50μm | ~1纤维直径 | 高(HT) / 无(RT) |
| LLSEfficiency | 0.30 | 0.60* | 0.20-0.80 | 中 |
| ICL (高载) | 100μm | 2μm | 论文表4.11 | — |
| ICL (低载) | 22μm | 5μm | 论文表4.11 | — |
| SmallCrackCB | 0.5 | 0.5 | ≤1 | 低 |

*RT: TipSCF和LLSEfficiency对寿命影响极小，Paris律单独控制

## 二、预测结果

### 高温 550°C (SiC3200/Ti6242, Rand71)
| 试件 | 应力 | ICL | 预测寿命 | Exp | Et | 预测断点 | Exp |
|------|------|-----|---------|-----|-----|---------|-----|
| F2 | 1250 | 100 | 4,801 | 4,326 | 1.11 | 65.4±10 | 72 |
| F3 | 1250 | 100 | 4,382 | 2,813 | 1.56 | 76.7±3 | 87 |
| F1 | 1000 | 22 | 41,756 | 44,345 | 1.06 | 14.7±3 | — |

### 高温 550°C (SCS-6/BetTi, Sine R=0.1)
| 应力 | ICL | 预测寿命 | Exp | Et |
|------|-----|---------|-----|-----|
| 1500 | 100 | 3,945 | 5,612 | 1.42 |
| 1350 | 100 | 5,516 | 2,928 | 1.88 |

### 室温 (SCS-6/BetTiRT, Sine R=0.1)
| 应力 | ICL | 预测寿命 | Exp | Et | 失效模式 |
|------|-----|---------|-----|-----|---------|
| 2100 | 2 | 2,629 | 2,762 | 1.05 | LoadRatio |
| 1650 | 2 | 22,896 | 11,775 | 1.94 | KIC |
| 1200 | 5 | 15,258 | 12,645 | 1.21 | MaxCrackLen |

**全部预测在 factor-of-2 内（最大 Et=1.94）。**

## 三、HT vs RT 物理机制对比

| | 550°C | RT |
|------|-------|-----|
| 基体状态 | 塑性 (550MPa屈服) | 弹性 (900-1163MPa) |
| 纤维-纤维 SCF | ~1.01 (GLS-like) | ~1.33 (LLS) |
| 主导失效 | MaxCrackLen + 适度断点 | LoadRatio→KIC→Crack |
| ICL 尺度 | 100μm (大) | 2μm (极小) |
| SCF 对寿命影响 | 控制级联阈值 | 无影响 |
| 裂纹扩展 | Paris律 + 纤维桥联 | 纯Paris律 (无桥联) |

## 四、已归档结果

- `Results/FinalCalibration/`: HT最终标定+敏感性分析
- `Results/ICLImpact3/`, `ICLImpact4/`: HT ICL扫描
- `Results/CmImpact/`, `CmImpact2/`: Paris系数扫描  
- `Results/RT_Calibration/`: RT标定结果
- `Results/RECTICL/`, `RECTICL2/`, `RECTICL3/`: 方试件(待整理)
