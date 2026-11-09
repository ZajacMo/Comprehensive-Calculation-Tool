// 代数核心：一次/二次方程求根、顶点式转换、多项式数值求根。
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Numeric.h"

namespace calc {

// 方程求解结果。
struct EquationSolution {
    bool valid = false;
    std::string reason;       // 无效原因（中文）
    bool hasRealRoots = false;
    std::vector<Complex> roots;
    double discriminant = 0.0;

    // 形如 "x1 = 1, x2 = -2" 的可读结果。
    std::string describe(int precision = 6) const;
};

// 求解 ax + b = 0。
EquationSolution solveLinear(double a, double b);

// 求解 ax² + bx + c = 0。a == 0 时退化为一次方程。
EquationSolution solveQuadratic(double a, double b, double c);

// 由顶点式 a(x - h)² + k = 0 求解。内部展开为标准式后复用 solveQuadratic。
EquationSolution solveQuadraticVertex(double a, double h, double k);

// 多项式求根（数值法，Durand-Kerner 法）。
// 系数按降幂排列，coeff[0] 为 x^n 的系数。
struct PolynomialRoots {
    bool valid = false;
    std::string reason;
    std::vector<Complex> roots;
};

// 用 Durand-Kerner（Weierstrass）算法求多项式的全部复根。
PolynomialRoots solvePolynomial(const std::vector<double>& coefficients);

// 计算 x^n - 1 的所有单位根，用于演示与测试。
std::vector<Complex> rootsOfUnity(int n);

}  // namespace calc
