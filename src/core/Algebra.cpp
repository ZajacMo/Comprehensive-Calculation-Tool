#include "Algebra.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <sstream>

namespace calc {
namespace {

// 用 Horner 法计算多项式在 z 处的值。
Complex evaluatePolynomial(const std::vector<double>& coefficients, const Complex& z) {
    Complex result(0.0, 0.0);
    for (double c : coefficients) {
        result = result * z + c;
    }
    return result;
}

}  // namespace

std::string EquationSolution::describe(int precision) const {
    if (!valid) return reason;
    if (!hasRealRoots) {
        return "原方程无实根（判别式 Δ = " + formatReal(discriminant, precision) + "）";
    }
    if (roots.size() == 1) {
        return "x₁ = x₂ = " + formatComplex(roots[0], precision);
    }
    std::ostringstream out;
    out << "x₁ = " << formatComplex(roots[0], precision)
        << " ,  x₂ = " << formatComplex(roots[1], precision);
    return out.str();
}

EquationSolution solveLinear(double a, double b) {
    EquationSolution result;
    if (nearlyEqual(a, 0.0)) {
        result.reason = nearlyEqual(b, 0.0) ? "恒等式，任意实数均为解。"
                                             : "方程无解（a 与 b 均为 0 时无解）。";
        return result;
    }
    result.valid = true;
    result.hasRealRoots = true;
    result.discriminant = 0.0;
    result.roots.push_back(Complex(-b / a, 0.0));
    return result;
}

EquationSolution solveQuadratic(double a, double b, double c) {
    // a == 0 时不再是二次方程，交给一次方程处理。
    if (nearlyEqual(a, 0.0)) {
        EquationSolution linear = solveLinear(b, c);
        if (linear.valid && linear.roots.size() == 1) {
            linear.reason.clear();
        }
        return linear;
    }

    EquationSolution result;
    result.valid = true;
    result.discriminant = b * b - 4.0 * a * c;

    if (result.discriminant < 0.0) {
        // 判别式为负：给出共轭复根，hasRealRoots 保持 false。
        double sqrtNeg = std::sqrt(-result.discriminant);
        result.roots.push_back(Complex(-b / (2.0 * a), sqrtNeg / (2.0 * a)));
        result.roots.push_back(Complex(-b / (2.0 * a), -sqrtNeg / (2.0 * a)));
        result.hasRealRoots = false;
        return result;
    }

    result.hasRealRoots = true;
    if (nearlyEqual(result.discriminant, 0.0)) {
        result.roots.push_back(Complex(-b / (2.0 * a), 0.0));
        return result;
    }

    // 求根公式：x = (-b ± √Δ) / (2a)。
    const double sqrtD = std::sqrt(result.discriminant);
    const double denominator = 2.0 * a;
    result.roots.push_back(Complex((-b + sqrtD) / denominator, 0.0));
    result.roots.push_back(Complex((-b - sqrtD) / denominator, 0.0));
    return result;
}

EquationSolution solveQuadraticVertex(double a, double h, double k) {
    // a(x - h)² + k = 0  =>  a x² - 2ah x + (a h² + k) = 0
    const double b = -2.0 * a * h;
    const double c = a * h * h + k;
    return solveQuadratic(a, b, c);
}

PolynomialRoots solvePolynomial(const std::vector<double>& coefficients) {
    PolynomialRoots result;

    // 系数按降幂排列：coefficients[0] 是 x^n 的系数。
    // 去掉最高次项的零系数后，真正的次数是 size - start - 1。
    size_t start = 0;
    while (start < coefficients.size() && nearlyEqual(coefficients[start], 0.0)) {
        ++start;
    }
    if (start == coefficients.size()) {
        result.reason = "所有系数均为 0，多项式恒为 0，无有限个根。";
        return result;
    }

    const std::vector<double> poly(coefficients.begin() + start, coefficients.end());
    const size_t degree = poly.size() - 1;

    if (degree == 0) {
        result.reason = "多项式退化为常数，非零常数没有根。";
        return result;
    }
    if (degree == 1) {
        result.valid = true;
        const double root = -poly[0] / poly[1];
        result.roots.push_back(Complex(root, 0.0));
        return result;
    }

    // 用 Aberth-Ehrlich 迭代求根，比 Durand-Kerner 收敛更快也更稳定：
    //   z_i <- z_i - w_i / (1 - w_i * Σ_{j≠i} 1/(z_i - z_j)),  w_i = p(z_i)/p'(z_i)
    const double leading = poly[0];
    // Cauchy 根界：所有根都落在半径 R 的圆内。
    double ratioMax = 1.0;
    for (size_t i = 1; i < poly.size(); ++i) {
        ratioMax = std::max(ratioMax, std::fabs(poly[i] / leading));
    }
    const double radius = 1.0 + ratioMax;

    std::vector<Complex> roots(degree);
    for (size_t i = 0; i < degree; ++i) {
        const double angle = 2.0 * kPi * static_cast<double>(i) / static_cast<double>(degree);
        roots[i] = Complex(radius * std::cos(angle), radius * std::sin(angle));
    }
    // 轻微扰动初始点，避免对称分布导致迭代停滞。
    for (size_t i = 0; i < degree; ++i) {
        roots[i] += Complex(0.1 * radius * static_cast<double>(i + 1) /
                                static_cast<double>(degree + 1),
                            0.0);
    }

    constexpr int maxIterations = 2000;
    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        double maxDelta = 0.0;
        for (size_t i = 0; i < degree; ++i) {
            const Complex value = evaluatePolynomial(poly, roots[i]);

            // 一阶导数 p'(z_i)。poly 按降幂排列，p'(z) 的系数为
            // (n·poly[0], (n-1)·poly[1], …, 1·poly[n-1])，用 Horner 法求值。
            Complex derivative(static_cast<double>(degree) * poly[0], 0.0);
            for (size_t k = 1; k < degree; ++k) {
                derivative = derivative * roots[i] +
                             poly[k] * static_cast<double>(degree - k);
            }
            if (std::abs(derivative) < 1e-300) continue;

            const Complex w = value / derivative;

            Complex sum(0.0, 0.0);
            for (size_t j = 0; j < degree; ++j) {
                if (i == j) continue;
                const Complex difference = roots[i] - roots[j];
                if (std::abs(difference) < 1e-300) continue;
                sum += Complex(1.0, 0.0) / difference;
            }

            const Complex denominator = Complex(1.0, 0.0) - w * sum;
            if (std::abs(denominator) < 1e-300) continue;

            const Complex delta = w / denominator;
            roots[i] -= delta;
            maxDelta = std::max(maxDelta, std::abs(delta));
        }
        if (maxDelta < 1e-15) break;
    }

    result.valid = true;
    result.roots = std::move(roots);
    return result;
}

std::vector<Complex> rootsOfUnity(int n) {
    std::vector<Complex> roots;
    if (n <= 0) return roots;
    roots.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        const double angle = 2.0 * kPi * static_cast<double>(i) / static_cast<double>(n);
        roots.push_back(Complex(std::cos(angle), std::sin(angle)));
    }
    return roots;
}

}  // namespace calc
