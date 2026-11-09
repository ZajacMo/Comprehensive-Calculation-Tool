# 综合计算工具 / Comprehensive Calculation Tool

一个面向数学计算的桌面工具，提供数论、统计、代数、线性代数与空间几何五大类功能。

v2.1 起采用**自包含 C++ 内核 + 零构建步骤 Web 前端**的架构：
所有算法集中在不依赖任何第三方库的纯 C++ 内核（`src/core/`），
通过内置 HTTP 服务暴露为 REST API，原生 HTML/CSS/JS 前端直接消费这些 API。

> **为什么不再默认依赖 Qt？**
> Qt 6 SDK 体积超过 2 GB，且只覆盖三种桌面平台。
> 现在的默认构建**只需要一个 C++17 编译器**，产出可在任意系统运行的
> Web 前端 + 后端服务，且前端不依赖 npm 与任何构建步骤。
> Qt 桌面界面作为**可选目标**保留（`-DBUILD_GUI=ON`）。

---

## 目录结构

```
综合计算工具/
├── CMakeLists.txt          # 顶层构建脚本
├── src/
│   ├── core/               # 算法内核：纯 C++17，零第三方依赖
│   │   ├── NumberTheory.*  # 质因数分解、素性判定、欧拉函数、莫比乌斯、模运算、进制转换
│   │   ├── Statistics.*    # 描述统计、最小二乘线性回归、相关系数
│   │   ├── Algebra.*       # 一次/二次方程、顶点式、多项式数值求根
│   │   ├── LinearAlgebra.* # 矩阵运算、行列式、逆矩阵、线性方程组
│   │   ├── Geometry.*      # 三维向量、三角形、四面体、常见几何体
│   │   ├── Sequences.*     # 斐波那契、卢卡斯、卡特兰数、组合数、整数分拆
│   │   ├── ComplexMath.*   # 复数幂、求根、极坐标
│   │   ├── InputParsing.*  # 输入校验（GUI / CLI / API 共用）
│   │   ├── Json.*          # 轻量 JSON 序列化与解析
│   │   ├── HttpServer.*    # 跨平台 HTTP/1.1 服务端
│   │   └── Numeric.*       # 复数、浮点容差比较、统一格式化
│   ├── api/                # REST 路由层：把内核能力映射为 JSON 接口
│   ├── server/             # Web 服务可执行文件
│   ├── cli/                # 命令行前端
│   ├── ui/                 # Qt 6 桌面界面（可选目标）
│   └── web/                # Web 前端（原生 HTML/CSS/JS，无构建步骤）
├── web/                    # Web 前端资源
│   ├── index.html
│   ├── styles.css
│   ├── app.js
│   └── favicon.svg
├── tests/                  # 单元测试 + 界面离屏渲染冒烟测试
├── scripts/                # 构建、测试与辅助脚本
└── legacy/mfc/             # v1 的 MFC 实现，仅作历史存档
```

---

## 快速开始

```bash
# 配置与构建（不需要 Qt）
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 启动 Web 服务并自动打开浏览器
./build/calc_server
```

打开 <http://127.0.0.1:8080/> 即可使用。服务默认只监听回环地址，不会暴露到局域网。

```bash
# 命令行
./build/calc_cli help

# 单元测试
ctest --test-dir build --output-on-failure
```

### 构建选项

| 选项 | 默认 | 说明 |
|---|---|---|
| `BUILD_GUI` | **OFF** | 构建 Qt 6 桌面界面（需要 Qt 6 SDK） |
| `BUILD_CLI` | ON | 构建命令行前端 |
| `BUILD_SERVER` | ON | 构建 Web 服务端 |
| `BUILD_TESTS` | ON | 构建单元测试 |

要构建 Qt 桌面界面：

```bash
cmake -S . -B build -DBUILD_GUI=ON -DCMAKE_PREFIX_PATH=<Qt6 安装路径>
cmake --build build
```

### Windows 一键脚本

```bat
scripts\build.bat            % 构建全部
scripts\build-core.bat       % 只构建内核、命令行、服务与测试（不需要 Qt）
scripts\run-tests.bat        % 运行单元测试
scripts\run-ctest.bat        % 运行全部 CTest 用例
scripts\cli.bat <命令>       % 调用命令行前端
scripts\start-server.bat     % 后台启动 Web 服务（测试用）
python scripts\api_test.py   % API 端到端测试
python scripts\screenshot.py <url> <输出.png>   % 截取前端页面
```

---

## 功能

### 数论
质因数分解（试除到 √n）、确定性 Miller-Rabin 素性判定（覆盖 64 位范围）、
欧拉函数 φ(n)、莫比乌斯函数 μ(n)、约数个数与约数和、扩展欧几里得、
模逆元与快速幂、2–36 进制互转。

### 统计
均值、中位数、样本方差、标准差、极差、四分位数与四分位距；
最小二乘线性回归给出斜率 k、截距 b、相关系数 r、决定系数 R² 与标准误；
可代入任意 x 预测 y。

### 代数
一次方程、二次方程（标准式与顶点式）、复数根、
任意次数多项式求根（Aberth–Ehrlich 数值法，系数按降幂排列）。

### 线性代数
矩阵加减乘、转置、行列式、秩、对称性判定、逆矩阵；
高斯-约当消元求解线性方程组，并区分唯一解、无解与无穷多解。

### 空间几何
三维三角形（面积、周长、边长、最大高，含退化检测）；
四面体（体积、表面积、重心、外接球球心与半径、内切球半径）；
球、圆柱、圆锥、正棱柱、正棱锥的表面积与体积。

### 数列与组合
斐波那契、卢卡斯、卡特兰数、阶乘、整数分拆数、组合数、排列数。

### 复数
整数次幂、n 次根、模与辐角、极坐标与直角坐标互转。

---

## Web API

所有接口返回 JSON。完整列表见 `src/api/ApiRoutes.cpp`。

| 端点 | 参数 | 说明 |
|---|---|---|
| `GET /api/health` | — | 服务状态与版本 |
| `GET /api/factorize` | `n` | 质因数分解与相关函数 |
| `GET /api/isprime` | `n` | 素性判定 |
| `GET /api/base-convert` | `value`, `base` | 进制转换 |
| `GET /api/modular` | `a`, `m` | 最大公约数与模逆元 |
| `GET /api/stats` | `values` | 描述统计（逗号分隔） |
| `GET /api/regress` | `points`, `x?` | 线性回归与预测 |
| `GET /api/linear` | `a`, `b` | 一次方程 |
| `GET /api/quadratic` | `a`, `b`, `c` | 二次方程 |
| `GET /api/vertex` | `a`, `h`, `k` | 顶点式二次方程 |
| `GET /api/polynomial` | `coefficients` | 多项式求根 |
| `GET /api/complex` | `real`, `imag`, `power?`, `roots?` | 复数运算 |
| `GET /api/matrix` | `rows`, `cols`, `values` | 矩阵运算 |
| `GET /api/solve-system` | `a`, `b` | 线性方程组 |
| `GET /api/triangle` | `points` | 三角形（9 个坐标） |
| `GET /api/tetrahedron` | `points` | 四面体（12 个坐标） |
| `GET /api/solid` | `kind`, `radius`, `height?`, `sides?` | 常见几何体 |
| `GET /api/sequences` | `n` | 各种数列 |
| `GET /api/combinatorics` | `n`, `k` | 组合数与排列数 |

示例：

```console
$ curl "http://127.0.0.1:8080/api/quadratic?a=1&b=-5&c=6"
{"a":1,"b":-5,"c":6,"discriminant":1,"hasRealRoots":true,"ok":true,
 "roots":[{"imag":0,"real":3},{"imag":0,"real":2}],
 "rootsText":"x₁ = 3.000000 ,  x₂ = 2.000000","valid":true}
```

---

## 命令行

```bash
./build/calc_cli help
```

| 命令 | 说明 |
|---|---|
| `factorize <n>` / `isprime <n>` | 质因数分解 / 素性判定 |
| `base-convert <value> <base>` | 进制转换 |
| `stats <v1> <v2> ...` | 描述统计 |
| `regress <x1> <y1> ...` | 线性回归 |
| `linear` / `quadratic` / `vertex` / `poly` | 方程求解 |
| `triangle` / `tetrahedron` / `solid` | 几何计算 |
| `matrix <rows> <cols> <v...>` | 矩阵运算 |
| `solve-system <b...> -- <a...>` | 线性方程组 |
| `sequences <n>` | 各种数列 |

---

## 测试

```bash
ctest --test-dir build --output-on-failure
```

- **`calc_tests`** — 104 个单元测试用例，覆盖全部内核模块（含 JSON 与 HTTP 层）。
  每个 v1 缺陷都有一条对应的回归测试。
- **`api_test`** — 75 项端到端 API 检查：真实启动服务、调用每个端点、校验数值。
- **`gui_smoke`** — Qt 界面的离屏渲染冒烟测试（仅在构建 Qt 目标时存在）。

单元测试框架是 `tests/test_framework.h` —— 一个约 100 行、零外部依赖的断言库，
因此单元测试不依赖 Qt，可在任意 C++17 工具链下运行。

Web 前端另有两个辅助脚本：`scripts/api_test.py`（端到端接口测试）与
`scripts/screenshot.py`（用 headless Edge 截图，便于人工核对排版与中文显示）。

---

## v1 的问题与修复

v1 的 MFC 实现保留在 `legacy/mfc/`。重写过程中发现并修复了其中 6 处**真实的算法错误**，
逐条记录在 `legacy/mfc/README.md`，每条都有对应的回归测试。摘要：

- 二次方程求根公式写错（`-0.5*(b±√Δ)/c`），导致**所有非平凡二次方程的解都是错的**
- 顶点式展开时符号错误（`+2a` 应为 `-2a`）
- 线性回归分页收集数据点时漏算/多算（数据点数为 5 的倍数时）
- 回归用 `(0,0)` 判定空行，合法的原点数据点被跳过
- 质因数分解 O(n) 试除，10⁹ 附近素数会让界面卡死十余秒
- 三角形缺少退化检测，三点共线时静默输出面积 0

---

## 许可证

本项目为学习用途开发，仅供个人学习和参考使用。
