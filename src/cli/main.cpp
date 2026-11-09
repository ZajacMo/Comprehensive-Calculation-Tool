// 命令行前端：与图形界面共用 src/core 的同一套算法，便于脚本化与自动化验证。
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "core/Algebra.h"
#include "core/ComplexMath.h"
#include "core/Geometry.h"
#include "core/InputParsing.h"
#include "core/LinearAlgebra.h"
#include "core/NumberTheory.h"
#include "core/Numeric.h"
#include "core/Sequences.h"
#include "core/Statistics.h"

namespace {

void printUsage() {
    std::cout <<
        R"(综合计算工具 · 命令行前端

用法:
  calc_cli <命令> [参数...]

数论:
  factorize <n>              质因数分解（同时给出 φ、μ、约数个数与约数和）
  isprime <n>                素性判定（确定性 Miller-Rabin）

统计:
  stats <v1> <v2> ...        描述统计：均值、中位数、方差、标准差、分位数
  regress <x1> <y1> ...      线性回归，参数按 x y x y ... 交替给出

代数:
  linear <a> <b>             求解 ax + b = 0
  quadratic <a> <b> <c>      求解 ax² + bx + c = 0
  vertex <a> <h> <k>         求解 a(x - h)² + k = 0
  poly <c0> <c1> ...         多项式求根，系数按降幂排列

几何:
  triangle <x1> <y1> <z1> <x2> <y2> <z2> <x3> <y3> <z3>
  tetrahedron <x1> <y1> <z1> ... <x4> <y4> <z4>
  solid <sphere|cylinder|cone|prism|pyramid> <参数...>

线性代数:
  matrix <rows> <cols> <v1> <v2> ...      行列式、秩、转置与逆矩阵
  solve-system <b1> ... <bn> -- <a11> ...  求解线性方程组

其他:
  sequences <n>                斐波那契、卢卡斯、卡特兰数、阶乘、整数分拆数
  base-convert <value> <base>  进制转换

示例:
  calc_cli factorize 1000000000
  calc_cli quadratic 1 -5 6
  calc_cli regress 1 2 2 4 3 6
  calc_cli solid cylinder 2 5
  calc_cli matrix 2 2 1 2 3 4
)";
}

// 把参数解析成 double 列表，失败时打印原因并返回 false。
bool parseArgs(const std::vector<std::string>& args, size_t start, std::vector<double>& out) {
    for (size_t i = start; i < args.size(); ++i) {
        auto parsed = calc::parseDouble(args[i]);
        if (!parsed.ok) {
            std::cerr << "参数错误：" << parsed.error << "\n";
            return false;
        }
        out.push_back(parsed.value);
    }
    return true;
}

int commandFactorize(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cerr << "用法：calc_cli factorize <n>\n";
        return 1;
    }
    auto parsed = calc::parseInteger(args[1]);
    if (!parsed.ok) {
        std::cerr << "参数错误：" << parsed.error << "\n";
        return 1;
    }
    const calc::int64 n = parsed.value;
    if (n < 2) {
        std::cerr << "请输入大于等于 2 的整数。\n";
        return 1;
    }

    const calc::Factorization f = calc::factorize(n);
    std::cout << "n         = " << n << "\n";
    std::cout << "质因数分解 = " << f.describe() << "\n";
    std::cout << "是否素数   = " << (calc::isPrime(static_cast<calc::uint64>(n)) ? "是" : "否") << "\n";
    std::cout << "欧拉函数 φ = " << calc::eulerPhi(n) << "\n";
    std::cout << "莫比乌斯 μ = " << calc::mobius(n) << "\n";
    std::cout << "约数个数   = " << calc::divisorCount(n) << "\n";
    std::cout << "约数和     = " << calc::divisorSum(n) << "\n";
    return 0;
}

int commandIsPrime(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cerr << "用法：calc_cli isprime <n>\n";
        return 1;
    }
    auto parsed = calc::parseInteger(args[1]);
    if (!parsed.ok) {
        std::cerr << "参数错误：" << parsed.error << "\n";
        return 1;
    }
    const calc::uint64 n = static_cast<calc::uint64>(parsed.value);
    std::cout << n << (calc::isPrime(n) ? " 是素数" : " 不是素数") << "\n";
    return 0;
}

int commandStats(const std::vector<std::string>& args) {
    std::vector<double> values;
    if (!parseArgs(args, 1, values)) return 1;
    if (values.empty()) {
        std::cerr << "至少需要一个数值。\n";
        return 1;
    }
    const calc::DescriptiveStats s = calc::describe(values);
    std::cout << "样本数     = " << s.count << "\n";
    std::cout << "平均值     = " << calc::formatReal(s.mean) << "\n";
    std::cout << "中位数     = " << calc::formatReal(s.median) << "\n";
    std::cout << "方差       = " << calc::formatReal(s.variance) << "\n";
    std::cout << "标准差     = " << calc::formatReal(s.stdDev) << "\n";
    std::cout << "最小值     = " << calc::formatReal(s.min) << "\n";
    std::cout << "最大值     = " << calc::formatReal(s.max) << "\n";
    std::cout << "极差       = " << calc::formatReal(s.range) << "\n";
    std::cout << "Q1 / Q3    = " << calc::formatReal(s.q1) << " / " << calc::formatReal(s.q3) << "\n";
    std::cout << "四分位距   = " << calc::formatReal(s.iqr) << "\n";
    return 0;
}

int commandRegress(const std::vector<std::string>& args) {
    std::vector<double> values;
    if (!parseArgs(args, 1, values)) return 1;
    if (values.size() % 2 != 0) {
        std::cerr << "参数需要成对给出：x1 y1 x2 y2 ...\n";
        return 1;
    }
    std::vector<calc::Point> points;
    for (size_t i = 0; i + 1 < values.size(); i += 2) {
        points.push_back({values[i], values[i + 1]});
    }
    const calc::LinearRegression fit = calc::fitLinearRegression(points);
    if (!fit.valid) {
        std::cerr << "无法拟合：" << fit.reason << "\n";
        return 1;
    }
    std::cout << "样本数   = " << fit.sampleSize << "\n";
    std::cout << "x̄        = " << calc::formatReal(fit.meanX) << "\n";
    std::cout << "ȳ        = " << calc::formatReal(fit.meanY) << "\n";
    std::cout << "斜率 k   = " << calc::formatReal(fit.slope) << "\n";
    std::cout << "截距 b   = " << calc::formatReal(fit.intercept) << "\n";
    std::cout << "相关系数 r= " << calc::formatReal(fit.r) << "\n";
    std::cout << "决定系数 R²= " << calc::formatReal(fit.rSquared) << "\n";
    std::cout << "回归方程 = y = " << calc::formatReal(fit.slope) << "x + "
              << calc::formatReal(fit.intercept) << "\n";
    return 0;
}

void printSolution(const calc::EquationSolution& solution) {
    if (!solution.valid) {
        std::cout << solution.reason << "\n";
        return;
    }
    std::cout << "判别式 Δ = " << calc::formatReal(solution.discriminant) << "\n";
    std::cout << solution.describe() << "\n";
}

int commandLinear(const std::vector<std::string>& args) {
    std::vector<double> values;
    if (!parseArgs(args, 1, values)) return 1;
    if (values.size() != 2) {
        std::cerr << "用法：calc_cli linear <a> <b>\n";
        return 1;
    }
    printSolution(calc::solveLinear(values[0], values[1]));
    return 0;
}

int commandQuadratic(const std::vector<std::string>& args) {
    std::vector<double> values;
    if (!parseArgs(args, 1, values)) return 1;
    if (values.size() != 3) {
        std::cerr << "用法：calc_cli quadratic <a> <b> <c>\n";
        return 1;
    }
    printSolution(calc::solveQuadratic(values[0], values[1], values[2]));
    return 0;
}

int commandVertex(const std::vector<std::string>& args) {
    std::vector<double> values;
    if (!parseArgs(args, 1, values)) return 1;
    if (values.size() != 3) {
        std::cerr << "用法：calc_cli vertex <a> <h> <k>\n";
        return 1;
    }
    printSolution(calc::solveQuadraticVertex(values[0], values[1], values[2]));
    return 0;
}

int commandPoly(const std::vector<std::string>& args) {
    std::vector<double> coefficients;
    if (!parseArgs(args, 1, coefficients)) return 1;
    if (coefficients.size() < 2) {
        std::cerr << "用法：calc_cli poly <c0> <c1> ...（系数按降幂排列）\n";
        return 1;
    }
    const calc::PolynomialRoots result = calc::solvePolynomial(coefficients);
    if (!result.valid) {
        std::cerr << result.reason << "\n";
        return 1;
    }
    std::cout << "共 " << result.roots.size() << " 个根：\n";
    for (size_t i = 0; i < result.roots.size(); ++i) {
        std::cout << "  x" << (i + 1) << " = " << calc::formatComplex(result.roots[i]) << "\n";
    }
    return 0;
}

int commandTriangle(const std::vector<std::string>& args) {
    std::vector<double> values;
    if (!parseArgs(args, 1, values)) return 1;
    if (values.size() != 9) {
        std::cerr << "用法：calc_cli triangle <x1> <y1> <z1> <x2> <y2> <z2> <x3> <y3> <z3>\n";
        return 1;
    }
    const calc::Vec3 a{values[0], values[1], values[2]};
    const calc::Vec3 b{values[3], values[4], values[5]};
    const calc::Vec3 c{values[6], values[7], values[8]};
    const calc::TriangleResult t = calc::analyzeTriangle(a, b, c);
    if (t.degenerate) {
        std::cout << t.reason << "\n";
        return 0;
    }
    std::cout << "面积     = " << calc::formatReal(t.area) << "\n";
    std::cout << "周长     = " << calc::formatReal(t.perimeter) << "\n";
    std::cout << "三边长   = " << calc::formatReal(t.sideA) << " / "
              << calc::formatReal(t.sideB) << " / " << calc::formatReal(t.sideC) << "\n";
    std::cout << "最大高   = " << calc::formatReal(t.altitude) << "\n";
    return 0;
}

int commandTetrahedron(const std::vector<std::string>& args) {
    std::vector<double> values;
    if (!parseArgs(args, 1, values)) return 1;
    if (values.size() != 12) {
        std::cerr << "用法：calc_cli tetrahedron <x1> <y1> <z1> ... <x4> <y4> <z4>\n";
        return 1;
    }
    const calc::Vec3 a{values[0], values[1], values[2]};
    const calc::Vec3 b{values[3], values[4], values[5]};
    const calc::Vec3 c{values[6], values[7], values[8]};
    const calc::Vec3 d{values[9], values[10], values[11]};
    const calc::TetrahedronResult t = calc::analyzeTetrahedron(a, b, c, d);
    if (t.degenerate) {
        std::cout << t.reason << "\n";
        return 0;
    }
    std::cout << "体积       = " << calc::formatReal(t.volume) << "\n";
    std::cout << "表面积     = " << calc::formatReal(t.surfaceArea) << "\n";
    std::cout << "重心       = (" << calc::formatReal(t.centroid.x) << ", "
              << calc::formatReal(t.centroid.y) << ", "
              << calc::formatReal(t.centroid.z) << ")\n";
    std::cout << "外接球半径 = " << calc::formatReal(t.circumradius) << "\n";
    std::cout << "内切球半径 = " << calc::formatReal(t.inradius) << "\n";
    return 0;
}

int commandSolid(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << "用法：calc_cli solid <sphere|cylinder|cone|prism|pyramid> <参数...>\n";
        return 1;
    }
    const std::string kind = args[1];
    std::vector<double> values;
    if (!parseArgs(args, 2, values)) return 1;

    calc::SolidResult solid;
    if (kind == "sphere") {
        if (values.size() != 1) { std::cerr << "sphere 需要 1 个参数：半径\n"; return 1; }
        solid = calc::analyzeSphere(values[0]);
    } else if (kind == "cylinder") {
        if (values.size() != 2) { std::cerr << "cylinder 需要 2 个参数：半径 高\n"; return 1; }
        solid = calc::analyzeCylinder(values[0], values[1]);
    } else if (kind == "cone") {
        if (values.size() != 2) { std::cerr << "cone 需要 2 个参数：半径 高\n"; return 1; }
        solid = calc::analyzeCone(values[0], values[1]);
    } else if (kind == "prism") {
        if (values.size() != 3) { std::cerr << "prism 需要 3 个参数：底面外接圆半径 高 边数\n"; return 1; }
        solid = calc::analyzePrism(values[0], values[1], static_cast<int>(values[2]));
    } else if (kind == "pyramid") {
        if (values.size() != 3) { std::cerr << "pyramid 需要 3 个参数：底面外接圆半径 高 边数\n"; return 1; }
        solid = calc::analyzePyramid(values[0], values[1], static_cast<int>(values[2]));
    } else {
        std::cerr << "未知的几何体类型：" << kind << "\n";
        return 1;
    }

    if (!solid.valid) {
        std::cerr << solid.reason << "\n";
        return 1;
    }
    std::cout << "表面积 = " << calc::formatReal(solid.surfaceArea) << "\n";
    std::cout << "体积   = " << calc::formatReal(solid.volume) << "\n";
    return 0;
}

int commandMatrix(const std::vector<std::string>& args) {
    // 用法：calc_cli matrix <rows> <cols> <v1> <v2> ...
    if (args.size() < 4) {
        std::cerr << "用法：calc_cli matrix <rows> <cols> <v1> <v2> ...\n";
        return 1;
    }
    const auto rowsParsed = calc::parseInteger(args[1]);
    const auto colsParsed = calc::parseInteger(args[2]);
    if (!rowsParsed.ok || !colsParsed.ok || rowsParsed.value <= 0 || colsParsed.value <= 0) {
        std::cerr << "行数与列数必须是正整数。\n";
        return 1;
    }
    std::vector<double> values;
    if (!parseArgs(args, 3, values)) return 1;

    const size_t rows = static_cast<size_t>(rowsParsed.value);
    const size_t cols = static_cast<size_t>(colsParsed.value);
    if (values.size() != rows * cols) {
        std::cerr << "元素个数应为 " << (rows * cols) << "，实际为 " << values.size() << "。\n";
        return 1;
    }

    calc::Matrix matrix(rows, cols);
    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            matrix.set(r, c, values[r * cols + c]);
        }
    }

    std::cout << "矩阵     = " << matrix.describe() << "\n";
    std::cout << "行列式   = " << calc::formatReal(matrix.determinant()) << "\n";
    std::cout << "秩       = " << matrix.rank() << "\n";
    std::cout << "是否对称 = " << (matrix.isSymmetric() ? "是" : "否") << "\n";
    const auto inverse = matrix.inverse();
    if (inverse.has_value()) {
        std::cout << "逆矩阵   = " << inverse->describe() << "\n";
    } else {
        std::cout << "逆矩阵   = 不存在（矩阵奇异）\n";
    }
    return 0;
}

int commandSolveSystem(const std::vector<std::string>& args) {
    // 用法：calc_cli solve-system <b1> <b2> ... -- <a11> <a12> ...
    // 第一个参数之后用 -- 分隔系数矩阵与右端项。
    std::vector<std::string> rightSide;
    std::vector<std::string> coefficients;
    bool afterSeparator = false;
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--") {
            afterSeparator = true;
            continue;
        }
        (afterSeparator ? coefficients : rightSide).push_back(args[i]);
    }

    std::vector<double> b;
    std::vector<double> a;
    if (!parseArgs(rightSide, 0, b) || !parseArgs(coefficients, 0, a)) return 1;

    const size_t n = b.size();
    if (n == 0 || a.size() != n * n) {
        std::cerr << "用法：calc_cli solve-system <b1> ... <bn> -- <a11> <a12> ...\n";
        std::cerr << "系数矩阵应有 " << (n * n) << " 个元素。\n";
        return 1;
    }

    calc::Matrix matrix(n, n);
    for (size_t r = 0; r < n; ++r) {
        for (size_t c = 0; c < n; ++c) {
            matrix.set(r, c, a[r * n + c]);
        }
    }

    const auto result = calc::solveLinearSystemDetailed(matrix, b);
    if (!result.valid) {
        std::cerr << result.reason << "\n";
        return 1;
    }
    if (result.hasInfiniteSolutions) {
        std::cout << "有无穷多解，自由变量：";
        for (int index : result.freeVariables) std::cout << "x" << (index + 1) << " ";
        std::cout << "\n其中一个解：";
        for (double v : result.solution) std::cout << calc::formatReal(v) << " ";
        std::cout << "\n";
    } else {
        std::cout << "唯一解：";
        for (double v : result.solution) std::cout << calc::formatReal(v) << " ";
        std::cout << "\n";
    }
    return 0;
}

int commandSequences(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        std::cerr << "用法：calc_cli sequences <n>\n";
        return 1;
    }
    const auto parsed = calc::parseInteger(args[1]);
    if (!parsed.ok || parsed.value < 0) {
        std::cerr << "n 必须是不小于 0 的整数。\n";
        return 1;
    }
    const calc::uint64 n = static_cast<calc::uint64>(parsed.value);

    const auto fib = calc::fibonacci(n);
    const auto luc = calc::lucas(n);
    const auto cat = calc::catalan(n);
    const auto fact = calc::factorial(n);
    const auto part = calc::integerPartition(n);

    std::cout << "n            = " << n << "\n";
    std::cout << "斐波那契 F(n) = " << (fib.has_value() ? std::to_string(*fib) : "超出 64 位范围") << "\n";
    std::cout << "卢卡斯 L(n)   = " << (luc.has_value() ? std::to_string(*luc) : "超出 64 位范围") << "\n";
    std::cout << "卡特兰数 C(n) = " << (cat.has_value() ? std::to_string(*cat) : "超出范围") << "\n";
    std::cout << "阶乘 n!       = " << (fact.has_value() ? std::to_string(*fact) : "超出范围") << "\n";
    std::cout << "分拆数 p(n)   = " << (part.has_value() ? std::to_string(*part) : "超出范围") << "\n";
    return 0;
}

int commandBaseConvert(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        std::cerr << "用法：calc_cli base-convert <value> <base>\n";
        return 1;
    }
    const auto baseParsed = calc::parseInteger(args[2]);
    if (!baseParsed.ok || baseParsed.value < 2 || baseParsed.value > 36) {
        std::cerr << "进制必须在 2 到 36 之间。\n";
        return 1;
    }
    const auto value = calc::fromBase(args[1], static_cast<int>(baseParsed.value));
    if (!value.has_value()) {
        std::cerr << "'" << args[1] << "' 不是合法的 " << baseParsed.value << " 进制整数。\n";
        return 1;
    }
    std::cout << "十进制 = " << *value << "\n";
    for (int base : {2, 8, 10, 16, 36}) {
        std::cout << std::setw(2) << base << " 进制 = " << calc::toBase(*value, base).value_or("") << "\n";
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty()) {
        printUsage();
        return 0;
    }

    const std::string& command = args[0];
    if (command == "help" || command == "--help" || command == "-h") {
        printUsage();
        return 0;
    }
    if (command == "factorize") return commandFactorize(args);
    if (command == "isprime") return commandIsPrime(args);
    if (command == "stats") return commandStats(args);
    if (command == "regress") return commandRegress(args);
    if (command == "linear") return commandLinear(args);
    if (command == "quadratic") return commandQuadratic(args);
    if (command == "vertex") return commandVertex(args);
    if (command == "poly") return commandPoly(args);
    if (command == "triangle") return commandTriangle(args);
    if (command == "tetrahedron") return commandTetrahedron(args);
    if (command == "solid") return commandSolid(args);
    if (command == "matrix") return commandMatrix(args);
    if (command == "solve-system") return commandSolveSystem(args);
    if (command == "sequences") return commandSequences(args);
    if (command == "base-convert") return commandBaseConvert(args);

    std::cerr << "未知命令：" << command << "\n\n";
    printUsage();
    return 1;
}
