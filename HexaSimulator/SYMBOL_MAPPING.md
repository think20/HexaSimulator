# 论文符号 ↔ 代码变量对照表 | Symbol–Code Cross-Reference

> 本表把论文中使用的数学符号与 HexaSimulator / FractureHollower 源代码中的变量名一一对应，
> 便于读者复现、也便于后续维护。论文正文只使用左列的符号；右列的代码标识符**不出现在论文中**。
> 
> 维护规则：**新增或改名代码变量时，必须同步更新本表与论文的符号表（Nomenclature）**。

---

## 1. 几何与试件 Geometry & specimen

| 论文符号 | 代码变量 | 定义位置 | 含义 | 取值/单位 |
|---|---|---|---|---|
| $\theta$ | `TwistAngle` | Load.cpp:227 | 纤维离轴（扭转）角 | deg |
| $R_0$ | `R0` | TestManager.cpp:160（CSV 纤维行） | 纤维半径 | 51 μm |
| $v_f$ | `Vf` | TestManager.cpp:238（CSV 复合材料行） | 纤维体积分数 | 0.45 / 0.43 / 0.36 |
| $D$ | `SpecimenRadious` | TestManager.cpp:238 | 试件半径 | 1500 / 1750 μm |
| $L$ | `L` | TestManager.cpp:305（CSV 测试行） | 纤维段长度（轴向节点间距） | 100 μm |
| $k_\theta$ | `OffsetK` | Material.h:11 | 扭转角沿径向的斜率 | 0.2529 等 |
| $b_\theta$ | `OffsetB` | Material.h:12 | 扭转角沿径向的截距 | −0.6441 等 |
| $\alpha$ | `g_Alpha` | FractureHollower/main.cpp | 仿真与 XCT 断面的配准角 | F2 146°，F3 70° |

## 2. 材料参数 Material parameters

| 论文符号 | 代码变量 | 定义位置 | 含义 | 取值/单位 |
|---|---|---|---|---|
| $\sigma$_0$ | `Sigma0` | TestManager.cpp:160 | 标距 $L_g$ 下的纤维特征强度 | 3160 / 4600 MPa |
| $m_f$ | `mf` | TestManager.cpp:160 | Weibull 模量（制备态） | 14.89 |
| $L_g$ | `Lg` | TestManager.cpp:160 | 强度测量的标距长度 | 25.4 mm |
| $E_f$ | `Ef` | TestManager.cpp:160 | 纤维弹性模量 | 429 GPa |
| $\nu$_f$ | `Niuf` | TestManager.cpp:160 | 纤维泊松比 | 0.25 |
| $\beta$_\sigma$ | `StrengthDecay` | TestManager.cpp:160 | 制备致强度折减系数 | 0.80 |
| $\beta$_m$ | `SctDecay` | CSV 纤维行（第 10 列） | 制备致 Weibull 模量折减 | 0.455 |
| $\sigma$_{m,\max}$ | `Sigmams` | TestManager.cpp:202 | 基体屈服强度 | 550 MPa |
| $E_m$ | `Em` | TestManager.cpp:202 | 基体弹性模量 | 88.3 GPa |
| $\nu$_m$ | `Nium` | TestManager.cpp:202 | 基体泊松比 | 0.33 |
| $\tau$ | `SlidingStrength0` | TestManager.cpp:245 | 界面滑移强度（内部 ×1e6 转 Pa） | 100 MPa |

## 3. 裂纹扩展 Crack growth

| 论文符号 | 代码变量 | 定义位置 | 含义 | 取值/单位 |
|---|---|---|---|---|
| $b$, $c$ | `b`, `c` | Crack.cpp:170–171 | 厚度 / 宽度方向裂纹尺寸 | μm（内部 m） |
| $a_0$ | `ICLCentral` / `ICLSide` / `ICLRect` | TestManager.cpp:305 | 初始裂纹长度 | 100 μm（F1 20，BLFHT 5） |
| $k_0$ | `k0` | Crack.cpp:178 | 裂纹前沿折线斜率（式 10） | 无量纲 |
| $b_0$ | `b0` | Crack.cpp:179 | 裂纹前沿折线截距 | $b-R$ |
| $K$, $\Delta K$ | `Kb`, `Kc` / `DeltaKb`, `DeltaKc` | Crack.cpp:34–47 | 应力强度因子及其幅值 | MPa$\sqrt{\mathrm{m}}$ |
| $K_{IC}$ | `KIC` | Material.h（constexpr，CSV 基体行第 17 列可覆盖） | 基体断裂韧性 | 70 MPa$\sqrt{\mathrm{m}}$ |
| $K_{\mathrm{low}}$ | `KLow` | Material.h | 短/长裂纹段分界 | 2.5 MPa$\sqrt{\mathrm{m}}$ |
| $C_1, m_1$ | `Cm[0]`, `Mm[0]` | Crack.cpp:41–42 | 短裂纹段 Paris 常数 | — |
| $C_2, m_2$ | `Cm[1]`, `Mm[1]` | Crack.cpp:46–47 | 长裂纹段 Paris 常数 | $m_2$ = 3.1256 |
| $f_b$, $f_c$ | `dbFactor`, `dcFactor` | Crack.cpp:170–171 | b / c 向扩展倍率 | 1.6 / 1.0 |
| $\lambda$ | `SmallCrackCB` | Crack.cpp:173 | 萌生阶段形状参数（$c=\lambda b$） | 0.5 |
## 4. 尾流、过载与级联 Wake, overload and LLS

| 论文符号 | 代码变量 | 定义位置 | 含义 | 取值/单位 |
|---|---|---|---|---|
| $\xi$ | `Dis` | Load.cpp:505 附近 | 到裂纹前沿的有符号距离（式 12） | μm |
| $h_w$ | `WakeC0` | Load.cpp:438 | 尾流深度尺度（过渡区插值） | 150 μm（默认） |
| $\kappa$_t$ | `TipSCF` | Load.cpp:512–515 | 裂纹尖端过载峰值（式 22） | 4.0 |
| $\ell$_t$ | `TipZone` | Load.cpp:467 | 过载带半宽 | 50 μm |
| $\eta$ | `LLSEff` | Load.cpp:707–708 | 局部载荷分担效率 | 0.79 |
| $S$ | `CrackArea` | Crack.cpp | 开裂面积（式 14） | m² |
| $\sigma$_{f,\mathrm{intact}}$ | `SigmaIntact` | Load.cpp | 完整区纤维应力（式 17） | MPa |
| $\sigma$_{f,\mathrm{wake}}$ | `SigmaWake` | Load.cpp | 深尾流纤维应力 $\sigma$_c/v_f$ | MPa |

## 5. 强度退化 Strength degradation

| 论文符号 | 代码变量 | 定义位置 | 含义 | 取值/单位 |
|---|---|---|---|---|
| $\alpha$_\sigma$ | `DegSigD` | Load.cpp:197–198 | 离轴纤维饱和退化幅度 | 0.020 |
| $N_d$ | `DegSigN` | Load.cpp:197–198 | 退化特征循环数 | 2000 |
| $\theta$_{th}$ | `ThetaThresh` | Load.cpp:224 | 退化的阈值角 | 7° |
| $\theta$_{ref}$ | `ThetaRef` | Load.cpp:225 | 退化的参考角 | 4.8° |
| $S_{f0}$, $S_f(N)$ | `Sigma0` / 节点 `Strength` | Load.cpp:210 附近 | 初始 / 循环退化后的纤维强度 | MPa |

## 6. 失效判据 Failure criteria

| 论文符号 | 代码变量 | 定义位置 | 含义 | 取值/单位 |
|---|---|---|---|---|
| $\Lambda$ | `LoadRatio` | Load.cpp:525, 742 | 截面纤维载荷比（瞬时） | — |
| $\Lambda$_c$ | `MaxLoadRatio` | Load.cpp:525, 742 | 载荷比临界值 | 0.48 |
| $K_{IC}$ 判据 | `Kb > KIC` | Crack.cpp:34 | 裂纹失稳 | — |

## 7. 可见性过滤 Visibility filter（FractureHollower）

| 论文符号 | 代码变量 | 定义位置 | 含义 | 取值/单位 |
|---|---|---|---|---|
| $z_0$ | `g_ZOffsetUm` / `ZOffsetUm` | FractureHollower/stdafx.h, main.cpp | 断面对齐深度 $0.49\sqrt{h d_0}$ | F2 118，F3 128 μm |
| $d_0$ | `FiberDiameter` | FractureHollower/stdafx.h | 纤维间距（=2$R_0$） | 102 μm |
| $h$ | `MeanHeight` | FractureHollower | XCT 断面高度均值 | F2 572，F3 676 μm |
| — | `PixelperUm` | FractureHollower/stdafx.h | 像素—长度换算 | 1/3.07 px/μm |
| — | `LayerDistance` | FractureHollower/stdafx.h | 层距（Z 向步长） | 100 μm |

## 8. 数值控制 Numerics

| 常数 | 代码变量 | 定义位置 | 含义 | 取值 |
|---|---|---|---|---|
| — | `MaxCycle` | stdafx.h:30 | 单算例最大循环数 | 2×10⁷ |
| — | `EM6` | stdafx.h | μm → m 换算 | 1e-6 |
| — | `E6` | stdafx.h | MPa → Pa 换算 | 1e6 |

## 9. CSV 输入列对照（TestDataC.csv）

模型由 `TestDataC.csv` 驱动，各行的列序如下（解析见 `TestManager.cpp`）。

| 行类型 | 列号（从 0 计） | 论文符号 | 代码字段 | 说明 |
|---|---|---|---|---|
| 纤维 | 1 | $\sigma$_0$ | `Sigma0` | 特征强度 |
| 纤维 | 2 | $m_f$ | `mf` | Weibull 模量 |
| 纤维 | 3 | $L_g$ | `Lg` | 标距 |
| 纤维 | 7 | $R_0$ | `R0` | 半径 |
| 基体 | 2 | $\sigma$_{m,\max}$ | `Sigmams` | 屈服强度 |
| 基体 | 5 | $E_m$ | `Em` | 弹性模量 |
| 基体 | 16 | $K_{IC}$ | `KIC` | 断裂韧性（可扫描） |
| 复合材料 | 2 | $v_f$ | `Vf` | 体积分数 |
| 复合材料 | 3 | $k_\theta$ | `OffsetK` | 扭转角斜率 |
| 复合材料 | 4 | $b_\theta$ | `OffsetB` | 扭转角截距 |
| 复合材料 | 5 | $D$ | `SpecimenRadious` | 试件半径 |
| 复合材料 | 11 | $\tau$ | `SlidingStrength0` | 界面滑移强度 |
| 复合材料 | 17 | $\alpha$_\sigma$ | `DegSigD` | 退化幅度 |
| 复合材料 | 19 | $\lambda$ | `SmallCrackCB` | 短裂纹形状参数 |
| 测试 | 4–6 | $a_0$ | `ICLCentral`/`Side`/`Rect` | 初始裂纹长度 |
| 测试 | 10 | $\Lambda$_c$ | `MaxLoadRatio` | 载荷比临界值 |
| 测试 | 15 | $\kappa$_t$ | `TipSCF` | 尖端过载 |
| 测试 | 16 | $\ell$_t$ | `TipZone` | 过载带半宽 |
| 测试 | 17 | $\eta$ | `LLSEff` | 载荷分担效率 |

---

## 10. 与论文的对应检查清单 Checklist

- [ ] 论文正文中**不出现**任何右列的代码标识符（`TipSCF`、`LLSEff`、`DegSigD` 等）。
- [ ] 论文符号表（Nomenclature）与本表的左列一一对应。
- [ ] 代码中若重命名变量，同步更新本表第 1–8 节与论文符号表。
- [ ] `K_IC`、`KLow` 可从 CSV 覆盖；其余常数为 `constexpr`，扫描需重新编译。

> 生成时间：2026-11-16 ｜ 对应论文：`journal_paper_final_M.md`（Elsevier EFM 投稿版）
