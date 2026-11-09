#include "test_framework.h"

#include "core/Algebra.h"

using namespace calc;

TEST(Algebra, LinearEquation) {
    auto result = solveLinear(2.0, -4.0);
    CHECK(result.valid);
    CHECK(result.hasRealRoots);
    CHECK_EQ(result.roots.size(), size_t(1));
    CHECK_NEAR(result.roots[0].real(), 2.0, 1e-12);
}

TEST(Algebra, LinearEquationDegenerate) {
    auto noSolution = solveLinear(0.0, 5.0);
    CHECK(noSolution.valid == false);

    auto infinite = solveLinear(0.0, 0.0);
    CHECK(infinite.valid == false);
}

TEST(Algebra, QuadraticTwoRealRoots) {
    // x² - 5x + 6 = 0  =>  x = 2, 3
    // 判别式 Δ = b² - 4ac = 25 - 24 = 1
    // 原始实现写成了 -0.5*(b±√Δ)/c，根是错的。
    auto result = solveQuadratic(1.0, -5.0, 6.0);
    CHECK(result.valid);
    CHECK(result.hasRealRoots);
    CHECK_NEAR(result.discriminant, 1.0, 1e-12);
    CHECK_EQ(result.roots.size(), size_t(2));

    auto sorted = result.roots;
    std::sort(sorted.begin(), sorted.end(),
              [](const Complex& a, const Complex& b) { return a.real() < b.real(); });
    CHECK_NEAR(sorted[0].real(), 2.0, 1e-9);
    CHECK_NEAR(sorted[1].real(), 3.0, 1e-9);
}

TEST(Algebra, QuadraticDoubleRoot) {
    // (x - 1)² = x² - 2x + 1  =>  x = 1
    auto result = solveQuadratic(1.0, -2.0, 1.0);
    CHECK(result.valid);
    CHECK(result.hasRealRoots);
    CHECK_NEAR(result.discriminant, 0.0, 1e-12);
    CHECK_NEAR(result.roots[0].real(), 1.0, 1e-9);
}

TEST(Algebra, QuadraticNoRealRoots) {
    // x² + 1 = 0  =>  无实根
    auto result = solveQuadratic(1.0, 0.0, 1.0);
    CHECK(result.valid);
    CHECK(result.hasRealRoots == false);
    CHECK(result.discriminant < 0.0);
    // 仍然给出共轭复根。
    CHECK_EQ(result.roots.size(), size_t(2));
    CHECK_NEAR(result.roots[0].real(), 0.0, 1e-12);
    CHECK_NEAR(std::fabs(result.roots[0].imag()), 1.0, 1e-12);
    CHECK_NEAR(result.roots[0].imag(), -result.roots[1].imag(), 1e-12);
}

TEST(Algebra, QuadraticRootsSatisfyEquation) {
    // 把求出的根代回原方程，残差应接近 0。
    const double a = 3.0, b = -7.0, c = 2.0;
    auto result = solveQuadratic(a, b, c);
    CHECK(result.valid);
    for (const auto& root : result.roots) {
        const double residual = a * root.real() * root.real() + b * root.real() + c;
        CHECK_NEAR(residual, 0.0, 1e-9);
    }
}

TEST(Algebra, QuadraticDegeneratesToLinear) {
    // a = 0 时应退化为一次方程 bx + c = 0。
    auto result = solveQuadratic(0.0, 2.0, -6.0);
    CHECK(result.valid);
    CHECK(result.hasRealRoots);
    CHECK_NEAR(result.roots[0].real(), 3.0, 1e-12);
}

TEST(Algebra, VertexForm) {
    // a(x - h)² + k = 0，a=1, h=2, k=-9  =>  (x-2)² = 9  =>  x = 5, -1
    // 原始实现把 m[1] 乘了 +2a（符号错误），展开结果完全不对。
    auto result = solveQuadraticVertex(1.0, 2.0, -9.0);
    CHECK(result.valid);
    CHECK(result.hasRealRoots);

    auto sorted = result.roots;
    std::sort(sorted.begin(), sorted.end(),
              [](const Complex& a, const Complex& b) { return a.real() < b.real(); });
    CHECK_NEAR(sorted[0].real(), -1.0, 1e-9);
    CHECK_NEAR(sorted[1].real(), 5.0, 1e-9);
}

TEST(Algebra, VertexFormMatchesStandardForm) {
    // 顶点式与展开后的标准式必须给出相同的根。
    auto vertex = solveQuadraticVertex(2.0, -3.0, 4.0);
    auto standard = solveQuadratic(2.0, -2.0 * 2.0 * (-3.0), 2.0 * 9.0 + 4.0);
    CHECK(vertex.valid);
    CHECK(standard.valid);
    CHECK_EQ(vertex.roots.size(), standard.roots.size());
    for (size_t i = 0; i < vertex.roots.size() && i < standard.roots.size(); ++i) {
        CHECK_NEAR(vertex.roots[i].real(), standard.roots[i].real(), 1e-9);
    }
}

TEST(Algebra, VertexFormDoubleRoot) {
    // (x - 4)² = 0  =>  x = 4
    auto result = solveQuadraticVertex(1.0, 4.0, 0.0);
    CHECK(result.valid);
    CHECK_NEAR(result.roots[0].real(), 4.0, 1e-9);
}

TEST(Algebra, PolynomialLinear) {
    // 系数按降幂排列：2x - 6 = {-6, 2}
    auto result = solvePolynomial({-6.0, 2.0});
    CHECK(result.valid);
    CHECK_NEAR(result.roots[0].real(), 3.0, 1e-9);
}

TEST(Algebra, PolynomialQuadratic) {
    // x² - 5x + 6，降幂排列为 {1, -5, 6}
    auto result = solvePolynomial({1.0, -5.0, 6.0});
    CHECK(result.valid);
    CHECK_EQ(result.roots.size(), size_t(2));
    for (const auto& root : result.roots) {
        const double residual = root.real() * root.real() - 5.0 * root.real() + 6.0;
        CHECK_NEAR(residual, 0.0, 1e-7);
    }
    std::vector<double> realRoots;
    for (const auto& root : result.roots) {
        if (std::fabs(root.imag()) < 1e-6) realRoots.push_back(root.real());
    }
    std::sort(realRoots.begin(), realRoots.end());
    CHECK_EQ(realRoots.size(), size_t(2));
    if (realRoots.size() == 2) {
        CHECK_NEAR(realRoots[0], 2.0, 1e-6);
        CHECK_NEAR(realRoots[1], 3.0, 1e-6);
    }
}

TEST(Algebra, PolynomialCubic) {
    // x³ - 6x² + 11x - 6 = (x-1)(x-2)(x-3)，降幂排列为 {1, -6, 11, -6}
    auto result = solvePolynomial({1.0, -6.0, 11.0, -6.0});
    CHECK(result.valid);
    CHECK_EQ(result.roots.size(), size_t(3));

    std::vector<double> realRoots;
    for (const auto& root : result.roots) {
        const double residual = std::pow(root.real(), 3) - 6.0 * root.real() * root.real() +
                                11.0 * root.real() - 6.0;
        CHECK_NEAR(residual, 0.0, 1e-6);
        if (std::fabs(root.imag()) < 1e-6) realRoots.push_back(root.real());
    }
    std::sort(realRoots.begin(), realRoots.end());
    CHECK_EQ(realRoots.size(), size_t(3));
    if (realRoots.size() == 3) {
        CHECK_NEAR(realRoots[0], 1.0, 1e-6);
        CHECK_NEAR(realRoots[1], 2.0, 1e-6);
        CHECK_NEAR(realRoots[2], 3.0, 1e-6);
    }
}

TEST(Algebra, PolynomialQuarticWithComplexRoots) {
    // x⁴ + 1 = 0，降幂排列为 {1, 0, 0, 0, 1}，四个复根，每个根的模都为 1。
    auto result = solvePolynomial({1.0, 0.0, 0.0, 0.0, 1.0});
    CHECK(result.valid);
    CHECK_EQ(result.roots.size(), size_t(4));
    for (const auto& root : result.roots) {
        CHECK_NEAR(std::abs(root), 1.0, 1e-6);
        // 代回 x⁴ + 1 = 0 检验残差。
        const Complex z = root;
        const Complex z2 = z * z;
        const Complex z4 = z2 * z2;
        CHECK_NEAR(std::abs(z4 + Complex(1.0, 0.0)), 0.0, 1e-6);
    }
}

TEST(Algebra, PolynomialDegenerate) {
    auto zero = solvePolynomial({0.0, 0.0});
    CHECK(zero.valid == false);

    auto constant = solvePolynomial({5.0});
    CHECK(constant.valid == false);
}

TEST(Algebra, RootsOfUnity) {
    auto roots = rootsOfUnity(4);
    CHECK_EQ(roots.size(), size_t(4));
    for (const auto& root : roots) {
        CHECK_NEAR(std::abs(root), 1.0, 1e-12);
    }
    auto negative = rootsOfUnity(0);
    CHECK(negative.empty());
}

TEST(Algebra, SolutionDescribe) {
    auto result = solveQuadratic(1.0, -5.0, 6.0);
    const std::string text = result.describe();
    CHECK(text.find("2.000000") != std::string::npos);
    CHECK(text.find("3.000000") != std::string::npos);
}
