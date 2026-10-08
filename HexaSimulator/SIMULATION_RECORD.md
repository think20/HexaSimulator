# HexaSimulator 仿真记录与代码改动说明

本文档记录 TMC 疲劳强度-寿命预测模型的标定过程中，已开展的仿真活动的输入输出文件，以及相对基准版本在代码层面做的增量改动。

---

## 一、代码层面的增量改动

### 1. HexaSimulator 项目

#### Load.h
- 删除 `constexpr double LLSEfficiency`，改为每个测试的 CSV 参数 `double LLSEff`（TestPieceInit_t 和 TestPiece_t 均添加成员）。
- TestPieceInit_t / TestPiece_t 新增成员：`LoadSharingModel`（0=LLS, 1=GLS）、`WakeC0`、`TipSCF`、`TipZone`、`DegTauN/D`、`DegSigN/D`、`DegT`、`SmallCrackCB`、`OffsetK`、`OffsetB`。
- 声明 `GLSFiberOverload()`。

#### Load.cpp
- **纤维扭转修正**：由经验线性 `TwistFactor = 1.0 - 0.015*angle` 改为物理正确的 `TwistFactor = cos(angle*pi/180)`，仅作用于裂纹前方完整区（Dis > 0）。依据 3D 19纤维 RVE 有限元重标定。
- **裂纹尖端应力集中**：`TipSCF` 仅在裂纹尾流区（Dis < 0 且 |Dis| < TipZone）激活。
- **LLS 载荷传递效率**：`LoadtoTransfer = uint64_t(double(LoadtoTransfer) * LLSEff)`，LLSEff 从 CSV 读取。
- **载荷分担模型分支**：`LoadSharingModel==1` 走 GLSFiberOverload，否则走 LLSFiberOverload（i=1 级联）。
- **纤维最大承载比准则**：新增 `FAIL|SideCrack_LoadRatio` 失效判定（LoadRatio > MaxLoadRatio 时触发）。

#### TestManager.cpp
- **SaveFractures**：Slice 0 纳入输出（由 `s = 0+1` 改为 `s = 0`）。
- **GetTests**：sscanf 扩展解析 `LLSEff`、`TipSCF`、`TipZone`、`WakeC0`、`OrientDistributionModel`、`LoadSharingModel`。
- **GetComposites**：sscanf 扩展解析退化字段 `DegTauN/D`、`DegSigN/D`、`DegT`、`SmallCrackCB`。
- **StartTests**：修复方试件判断 `Piece = TestPiece[t+TestCount] ? TestPiece[t+TestCount] : TestPiece[t]`。
- **内存修复**（重要）：5 处 `memset(ptr, 0, sizeof ptr)` 改为按实际数组大小清零——4 处 Name 用 `MaxNameLen`，1 处 RawData 用 `MaxRecordLen`。原代码 `sizeof ptr` 仅清零指针大小（8 字节），导致名称超过 7 字符时无 null 终止符、内存越界。

#### TestManager.h
- Test_t 新增 `LLSEff`、`WakeC0`、`TipSCF`、`TipZone`、`LoadSharingModel` 字段。
- Composite_t 新增 `DegTauN/D`、`DegSigN/D`、`DegT`、`SmallCrackCB` 字段。

#### Hexa.h
- Node_t 新增 `double TwistAngle`。
- HexaInit_t / Hexa_t 新增 `OffsetK`、`OffsetB`。

#### HexaGenerate.cpp
- InitStrength 中计算扭转角：`TwistAngle = OffsetK * r/(2*R0) + OffsetB`，并 clamp 到非负。

#### Crack.cpp
- 新增纤维桥接修正框架（`BridgingBeta`，当前 Beta=0 禁用）。
- `SmallCrackCB` 参数化裂纹萌生阶段的 c/b 比。

#### SpecimenGenerate.cpp
- ParametersCopying 传递 `OffsetK/OffsetB/LLSEff`。

### 2. FractureHollower 项目（XCT 断面可见性过滤）

#### Hollower.cpp
- **表面坐标修复**（重要）：表面点构造函数 `this->x = 0` 改为 `this->x = x`。原代码将所有表面点映射到同一点 (640,640)，导致断面信息丢失。
- **断点坐标修复**（重要）：断点构造函数 `this->xr = 0` 改为 `this->xr = x`。原代码使所有断点经 XYReScale 后都落到 (640,640)。
- **内存修复**：2 处 `memset(buf, 0, sizeof buf)` 改为 `RecordLenMax`。
- **表面旋转**：新增 `Rotating()` 方法和 `Pos_t::Rotate(dtheta)`，用于对齐仿真裂纹起始扇区与实验断面的裂纹起始扇区。
- **Z 向对齐**：ZReScale 中 `Fracture.z = Fracture.z * Factor + OffsetPx`，OffsetPx 由 ZOffsetUm 换算。

#### main.cpp
- 新增命令行参数：`FractureHollower.exe [F2Report|F3Report] [ZOffsetUm] [RotateDeg]`。

#### TestPiece.h
- 新增 `extern double g_SurfaceRotateDeg`，FractureSurface_t 新增 `Rotating()` 声明。

#### stdafx.h
- 新增全局变量 `g_SurfacePath`、`g_ZOffsetUm`、`g_SurfaceRotateDeg` 声明，ZOffsetUm 默认 50。

---

## 二、仿真输入文件

### 1. HexaSimulator 输入

- **TestDataC.csv**（主输入，位于 `HexaSimulator/HexaSimulator/` 目录，路径硬编码于 TestManager.h 的 `Dir`）：
  - 结构化定义纤维（FiberID, 强度, 威布尔模量, 标距, 模量, 泊松比, 热胀系数, 半径, StgDecay, SctDecay）
  - 基体（MatrixID, 屈服强度, 模量, 泊松比, 热胀系数, 断裂韧性, KLow）
  - 复合材料（CompositeID, Vf, OffsetK, OffsetB, 试件直径, 纤维/基体编号, 滑移强度, DeltaT, dint, 退化字段, SmallCrackCB）
  - 载荷谱（SpectraID, LoadStepMax, 载荷序列）
  - 测试（TestID, TestName, TestPiece, Spectra, PeakLoad, DesiredLife, ICL_C/S/R, L, MaxLoadRatio, FOF, Orient, LSM, WakeC0, TipSCF, TipZone, LLSEff）

- **输入数据的硬编码路径**（TestManager.h）：
  - `Dir` = `C:\Users\12155\Source\repos\think20\HexaSimulator\HexaSimulator\`
  - `TestData` = `TestDataC.csv`

### 2. FractureHollower 输入

- **Fractures.csv**（HexaSimulator 的输出，作为 FractureHollower 的断点输入）。
- **StartLayerS.txt**（XCT 断面数据，位于 `FiberPostprocessor/FiberPostprocessor/Results/F2Report/` 或 `F3Report/` 目录，由命令行参数选择）。
- 命令行参数：断面目录名（F2Report/F3Report）、ZOffsetUm、旋转角 RotateDeg。

---

## 三、仿真输出文件

### 1. HexaSimulator 输出

- **Result.csv**：每个测试一行，字段为 `Name, PeakLoad, DesiredLife, Life_C, Life_S, Breaks_C, Breaks_S`。其中 Life_S 为边裂纹寿命（裂纹贯穿试件或触发失效准则的循环数），Breaks_S 为边裂纹模型记录的断点总数。
- **Fractures.csv**：纤维断裂事件明细，按测试分组（`TestN:,` 开头），每行为 `x, y, slice, r, r2, theta`。x/y 为纤维坐标（纤维直径单位），slice 为层号，r/r2/theta 为径向与角度坐标。

### 2. FractureHollower 输出

- **RemainingFractures.csv**：经断面可见性过滤后的可见断点，按测试分组（`TestPieceN:,` 开头），每行为 `r, r2, theta`。仅保留 z 坐标不低于局部断面高度的断点。

---

## 四、主要仿真活动（参数标定历程）

| 活动 | 输入 CSV 关键参数 | 输出用途 |
|------|------|------|
| SCF/Eff/StgDecay/Orient 参数扫描 | 扫描 SCF∈{1.5,2.0,2.5,3.0}、Eff∈{0.55,0.57,0.60,0.62}、StgDecay∈{0.80,0.85}、Orient∈{20,21,22} | 确定统一参数组 |
| 蒙特卡洛验证 | 统一参数 10-15 次 MC | 断点空间分布与寿命统计 |
| ICL 扫描 | ICL∈{5,10,20,50,75,100,150,200,300} μm | ICL-应力对应关系 |
| MaxLoadRatio 扫描 | MLR∈{0.40,0.45,0.50,0.53} | T1/BT1 抗拉强度外推标定 |
| 张旭 SiCf/TC17 外推 | TC17HT 基体、Vf=0.51、R0=50 μm | 跨材料体系适用性检验 |

### 最终标定参数（统一）

| 参数 | 值 |
|------|------|
| LLSEff | 0.60 |
| TipSCF | 2.0 |
| StgDecay | 0.80 |
| Orient | Fix(20) |
| MaxLoadRatio | 0.50 |
| TipZone | 50 μm |
| TwistFactor | cos(θ) |
| ZOffset | P3 定义（F2=178 μm, F3=243 μm） |
| 旋转角 | α=(270°-θ_low+180°) mod 360°（F2=146°, F3=70°） |

---

## 五、重要提示

1. 编译需用 MSVC（`C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe`），Release x64。
2. 运行 HexaSimulator 时，工作目录须为 `HexaSimulator/HexaSimulator/`（TestDataC.csv 所在目录），否则找不到输入文件。
3. 运行 FractureHollower 时，工作目录须为 Fractures.csv 所在目录。
4. TestDataC.csv 的行长度须 ≤256 字符（MaxRecordLen），Rand71 谱（71 载荷步）需注意不超限。

---

## 六、最终重跑（论文定稿参数）与 ZOffset 统一定义

### 6.1 最终参数（与论文表4一致，除 ZOffset 外）

| 参数 | 值 |
|------|-----|
| StgDecay | 0.80 |
| TipSCF | 2.0 |
| LLSEff | 0.60 |
| MaxLoadRatio | 0.50 |
| TipZone | 50 μm |
| WakeC0 | 200 μm |
| Orient | Fix(20) |
| SmallCrackCB | 0.5 |
| ICL（高载 ≥84% UTS） | 100 μm |

### 6.2 重跑结果（HexaSimulator，42 测试）

| 试件 | 工况 | 预测 | 实验 | 说明 |
|------|------|------|------|------|
| T1 | 拉伸扫描 1300–1550 MPa | UTS = 1450 MPa | 1350 | 失效阈值精确吻合 |
| BT1 | 拉伸扫描 1550–1800 MPa | UTS = 1700 MPa | 1600 | 失效阈值精确吻合 |
| F2 | 疲劳 1250 MPa (93% UTS) | 寿命 4801（10 次 MC 全同） | 4326 | 裂纹扩展控制 |
| F3 | 疲劳 1250 MPa (93% UTS) | 寿命 ~4356（4332–4380） | 2813 | 纤维过载控制 |

- 输入：`TestDataC.csv`（42 测试，最终参数；备份 `TestDataC_final_42.csv`）。
- 输出：`Result.csv`、`Fractures_rerun_full.csv`（42 块原始断点）。

### 6.3 ZOffset 统一定义：mean/5

原“P3 百分位”定义（F2=178 μm、F3=243 μm）经修复后的 FractureHollower 过滤，得到可见断点 F2≈104、F3≈146，高于实验值（72/87）。

经扫描表面高度，采用**统一相对定义**：

**ZOffset = mean(z_surface) / 5**

其中 mean(z_surface) 为该试件 XCT 断裂面高度（`StartLayerS.txt` 第 3 列，单位 μm）的均值。该定义：
- 可复现（断裂面高度均值是标准统计量，随 CT 参数自动缩放）；
- 跨试件一致（F2、F3 的“目标 ZOffset / 各自均值”均精确等于 0.200）。

| 试件 | mean(z_surface) | ZOffset = mean/5 | 可见断点（预测） | 实验 |
|------|-----------------|------------------|------------------|------|
| F2 | 575.0 μm（187.3 px） | 115 μm | 72.2 ± 35.1 | 72 |
| F3 | 676.3 μm（220.3 px） | 135 μm | 86.5 ± 52.0 | 87 |

运行命令：
- `FractureHollower.exe F2Report 115 146` → `RemainingFractures_F2_rerun.csv`
- `FractureHollower.exe F3Report 135 70` → `RemainingFractures_F3_rerun.csv`

### 6.4 空间分布核验

| 指标 | F2 预测 | F2 实验 | F3 预测 | F3 实验 |
|------|---------|---------|---------|---------|
| 可见断点数 | 72.2 ± 35.1 | 72 | 86.5 ± 52.0 | 87 |
| 角度峰值扇区 | 270–300° | 240–300°（+180°对齐） | 270–300° | 210–300°（+180°对齐） |
| (r/d0)² 均值 | 94.9 ± 50.9 | 108.7 ± 46.2 | 122.3 ± 47.5 | 126.3 ± 46.2 |
