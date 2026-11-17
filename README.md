# HexaSimulator

A discrete fibre-element simulator for the high-temperature progressive failure of
continuous-fibre-reinforced titanium matrix composites (SiCf/Ti).

The model discretises every fibre into a chain of 100 μm segments, samples each
segment strength from a Weibull distribution, advances a bridged matrix crack by a
two-stage Paris law, and resolves the fibre-break cascade in the crack wake by
local load sharing (LLS). Competing failure criteria — the maximum fibre
load-carrying ratio (Λc) and the matrix fracture toughness (K_IC) — decide the
failure mode and the life.

This is the code used to produce the results of:

> Hu J., Xiao H., Pan L., Niu X., Sun Z., Song Y.
> *Prediction of the spatial distribution of fatigue fibre breaks and a unified
> strength–life model for SiCf/Ti composites.*
> (submitted to Engineering Fracture Mechanics)

## Build

Visual Studio 2019/2022, x64 Release:

```
msbuild HexaSimulator.sln /p:Configuration=Release /p:Platform=x64
```

Compile-time switches (see `stdafx.h`):

| macro | default | meaning |
|---|---|---|
| `KIC_ONLY` | 0 | 1 = control model that keeps only the K_IC criterion (used for the comparison in Fig. 19 of the paper) |
| `VERBOSE_TRACE` | off | print per-cycle progress and LLS non-convergence notices to stdout |

## Run

The executable reads `TestDataC.csv` from its **current working directory** and
writes `Result.csv` and `Fractures.csv` next to it.

```
mkdir run && copy TestDataC.csv run && cd run && ..\x64\Release\HexaSimulator.exe
```

The input file has five blocks: fibre types, matrix types, composites, load
spectra and the test list. See `REPRO/` for a complete worked example, and
`SYMBOL_MAPPING.md` for the correspondence between the symbols used in the paper
and the variable names in the code.

### Test list columns

```
TestID, TestName, TestPiece, Spectra, PeakLoad, DesiredLife,
ICL_C, ICL_S, ICL_R, L, MaxLoadRatio, FOF, Orient, LSM,
WakeC0, TipSCF, TipZone, LLSEff
```

| column | meaning |
|---|---|
| `TestPiece` | index into the composite block |
| `Spectra` | index into the spectrum block (0 = Rand71, 1 = sinusoidal ramp) |
| `MaxLoadRatio` | Λc |
| `WakeC0` | h_w, wake depth scale (μm) |
| `TipSCF` | κt, crack-tip overload factor |
| `TipZone` | ℓt, half-width of the overloaded band (μm) |
| `LLSEff` | η, local load-sharing efficiency |

### Output

`Result.csv`: `Name, PeakLoad, DesiredLife, Life_C, Life_S, Breaks_C, Breaks_S, Mode`;
`Mode` is 6 when failure is controlled by the fracture-toughness criterion and
4/5 when the load-carrying-ratio criterion triggers first.

`Fractures.csv`: one block per test (`TestN:`, a count, then the break records
`x, y, z, r, r², θ`), consumed by the companion tool
[FractureHollower](https://github.com/think20/FractureHollower) for
fracture-surface visibility filtering.

## License

MIT — see [LICENSE](LICENSE).
