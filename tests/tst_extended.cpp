#include "test_framework.h"

#include "core/ComplexMath.h"
#include "core/LinearAlgebra.h"
#include "core/NumberTheory.h"
#include "core/Sequences.h"

using namespace calc;

TEST(LinearAlgebra, MatrixBasics) {
    Matrix m = makeMatrix({{1, 2}, {3, 4}});
    CHECK_EQ(m.rows(), size_t(2));
    CHECK_EQ(m.cols(), size_t(2));
    CHECK_NEAR(m.at(0, 0), 1.0, 1e-12);
    CHECK_NEAR(m.at(1, 1), 4.0, 1e-12);
    // 越界访问返回 0。
    CHECK_NEAR(m.at(5, 5), 0.0, 1e-12);
}

TEST(LinearAlgebra, MatrixArithmetic) {
    Matrix a = makeMatrix({{1, 2}, {3, 4}});
    Matrix b = makeMatrix({{5, 6}, {7, 8}});

    auto sum = a + b;
    CHECK_NEAR(sum.at(0, 0), 6.0, 1e-12);
    CHECK_NEAR(sum.at(1, 1), 12.0, 1e-12);

    auto difference = b - a;
    CHECK_NEAR(difference.at(0, 0), 4.0, 1e-12);
    CHECK_NEAR(difference.at(1, 1), 4.0, 1e-12);

    auto scaled = a * 2.0;
    CHECK_NEAR(scaled.at(0, 1), 4.0, 1e-12);

    auto product = a * b;
    CHECK_NEAR(product.at(0, 0), 19.0, 1e-12);
    CHECK_NEAR(product.at(0, 1), 22.0, 1e-12);
    CHECK_NEAR(product.at(1, 0), 43.0, 1e-12);
    CHECK_NEAR(product.at(1, 1), 50.0, 1e-12);
}

TEST(LinearAlgebra, MatrixTranspose) {
    Matrix m = makeMatrix({{1, 2, 3}, {4, 5, 6}});
    auto t = m.transposed();
    CHECK_EQ(t.rows(), size_t(3));
    CHECK_EQ(t.cols(), size_t(2));
    CHECK_NEAR(t.at(2, 1), 6.0, 1e-12);
}

TEST(LinearAlgebra, Determinant) {
    CHECK_NEAR(makeMatrix({{1, 2}, {3, 4}}).determinant(), -2.0, 1e-12);
    CHECK_NEAR(makeMatrix({{2}}).determinant(), 2.0, 1e-12);
    CHECK_NEAR(makeMatrix({{1, 0}, {0, 1}}).determinant(), 1.0, 1e-12);
    // 奇异矩阵行列式为 0。
    CHECK_NEAR(makeMatrix({{1, 2}, {2, 4}}).determinant(), 0.0, 1e-12);
    // 3×3 行列式。
    CHECK_NEAR(makeMatrix({{6, 1, 1}, {4, -2, 5}, {2, 8, 7}}).determinant(), -306.0, 1e-9);
}

TEST(LinearAlgebra, Inverse) {
    Matrix m = makeMatrix({{4, 7}, {2, 6}});
    auto inverse = m.inverse();
    CHECK(inverse.has_value());

    // A * A⁻¹ 应为单位矩阵。
    auto product = m * (*inverse);
    CHECK_NEAR(product.at(0, 0), 1.0, 1e-9);
    CHECK_NEAR(product.at(0, 1), 0.0, 1e-9);
    CHECK_NEAR(product.at(1, 0), 0.0, 1e-9);
    CHECK_NEAR(product.at(1, 1), 1.0, 1e-9);
}

TEST(LinearAlgebra, InverseOfSingularMatrix) {
    Matrix singular = makeMatrix({{1, 2}, {2, 4}});
    CHECK(singular.inverse().has_value() == false);
}

TEST(LinearAlgebra, Rank) {
    CHECK_EQ(makeMatrix({{1, 2}, {3, 4}}).rank(), size_t(2));
    CHECK_EQ(makeMatrix({{1, 2}, {2, 4}}).rank(), size_t(1));
    CHECK_EQ(makeMatrix({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}).rank(), size_t(2));
    CHECK_EQ(makeMatrix({{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}).rank(), size_t(3));
}

TEST(LinearAlgebra, Symmetry) {
    CHECK(makeMatrix({{1, 2}, {2, 1}}).isSymmetric());
    CHECK(makeMatrix({{1, 2}, {3, 1}}).isSymmetric() == false);
}

TEST(LinearAlgebra, SolveLinearSystemUnique) {
    // x + y = 3, x - y = 1  =>  x = 2, y = 1
    Matrix a = makeMatrix({{1, 1}, {1, -1}});
    auto result = solveLinearSystemDetailed(a, {3.0, 1.0});
    CHECK(result.valid);
    CHECK(result.hasInfiniteSolutions == false);
    CHECK_EQ(result.solution.size(), size_t(2));
    CHECK_NEAR(result.solution[0], 2.0, 1e-9);
    CHECK_NEAR(result.solution[1], 1.0, 1e-9);
}

TEST(LinearAlgebra, SolveLinearSystemThreeByThree) {
    Matrix a = makeMatrix({{2, 1, -1}, {-3, -1, 2}, {-2, 1, 2}});
    auto result = solveLinearSystemDetailed(a, {8.0, -11.0, -3.0});
    CHECK(result.valid);
    CHECK(result.hasInfiniteSolutions == false);
    CHECK_NEAR(result.solution[0], 2.0, 1e-9);
    CHECK_NEAR(result.solution[1], 3.0, 1e-9);
    CHECK_NEAR(result.solution[2], -1.0, 1e-9);
}

TEST(LinearAlgebra, SolveLinearSystemNoSolution) {
    // x + y = 1, 2x + 2y = 2 看似一致；改成 2x + 2y = 3 则矛盾。
    Matrix a = makeMatrix({{1, 1}, {2, 2}});
    auto result = solveLinearSystemDetailed(a, {1.0, 3.0});
    CHECK(result.valid == false);
    CHECK(!result.reason.empty());
}

TEST(LinearAlgebra, SolveLinearSystemInfinite) {
    // x + y = 1（两式相同），有无穷多解。
    Matrix a = makeMatrix({{1, 1}, {1, 1}});
    auto result = solveLinearSystemDetailed(a, {1.0, 1.0});
    CHECK(result.valid);
    CHECK(result.hasInfiniteSolutions);
    CHECK_EQ(result.freeVariables.size(), size_t(1));
}

TEST(Sequences, Fibonacci) {
    CHECK_EQ(fibonacci(0).value(), int64(0));
    CHECK_EQ(fibonacci(1).value(), int64(1));
    CHECK_EQ(fibonacci(10).value(), int64(55));
    CHECK_EQ(fibonacci(50).value(), int64(12586269025LL));
    // 超出 int64 表示范围时返回空。
    CHECK(fibonacci(100).has_value() == false);
}

TEST(Sequences, Lucas) {
    CHECK_EQ(lucas(0).value(), int64(2));
    CHECK_EQ(lucas(1).value(), int64(1));
    CHECK_EQ(lucas(10).value(), int64(123));
}

TEST(Sequences, Binomial) {
    CHECK_EQ(binomial(5, 2).value(), int64(10));
    CHECK_EQ(binomial(52, 5).value(), int64(2598960));
    CHECK_EQ(binomial(10, 0).value(), int64(1));
    CHECK_EQ(binomial(10, 10).value(), int64(1));
    // k > n 无意义。
    CHECK(binomial(3, 5).has_value() == false);
}

TEST(Sequences, Catalan) {
    CHECK_EQ(catalan(0).value(), int64(1));
    CHECK_EQ(catalan(1).value(), int64(1));
    CHECK_EQ(catalan(5).value(), int64(42));
    CHECK_EQ(catalan(10).value(), int64(16796));
}

TEST(Sequences, FactorialAndPermutation) {
    CHECK_EQ(factorial(0).value(), int64(1));
    CHECK_EQ(factorial(5).value(), int64(120));
    CHECK_EQ(factorial(20).value(), int64(2432902008176640000LL));
    CHECK(factorial(21).has_value() == false);

    CHECK_EQ(permutation(5, 2).value(), int64(20));
    CHECK_EQ(permutation(5, 5).value(), int64(120));
}

TEST(Sequences, IntegerPartition) {
    CHECK_EQ(integerPartition(0).value(), int64(1));
    CHECK_EQ(integerPartition(1).value(), int64(1));
    CHECK_EQ(integerPartition(5).value(), int64(7));
    CHECK_EQ(integerPartition(10).value(), int64(42));
}

TEST(Sequences, SequenceGenerators) {
    auto fib = fibonacciSequence(8);
    CHECK_EQ(fib.size(), size_t(8));
    CHECK_EQ(fib[0], int64(0));
    CHECK_EQ(fib[7], int64(13));

    auto pell = pellSequence(6);
    CHECK_EQ(pell.size(), size_t(6));
    CHECK_EQ(pell[0], int64(0));
    CHECK_EQ(pell[1], int64(1));
    CHECK_EQ(pell[2], int64(2));
    CHECK_EQ(pell[3], int64(5));
}

TEST(NumberTheory, ExtendedGcd) {
    auto egcd = extendedGcd(240, 46);
    CHECK_EQ(egcd.gcd, int64(2));
    // 验证 a*x + b*y == gcd
    CHECK_EQ(240 * egcd.x + 46 * egcd.y, int64(2));

    auto coprime = extendedGcd(9, 28);
    CHECK_EQ(coprime.gcd, int64(1));
    CHECK_EQ(9 * coprime.x + 28 * coprime.y, int64(1));
}

TEST(NumberTheory, ModularInverse) {
    auto inverse = modularInverse(3, 11);
    CHECK(inverse.has_value());
    CHECK_EQ(*inverse, int64(4));  // 3*4 = 12 ≡ 1 (mod 11)

    // gcd != 1 时不存在逆元。
    CHECK(modularInverse(6, 9).has_value() == false);
}

TEST(NumberTheory, ModularPower) {
    CHECK_EQ(modularPower(2, 10, 1000).value(), int64(24));
    CHECK_EQ(modularPower(3, 0, 7).value(), int64(1));
    CHECK_EQ(modularPower(5, 3, 13).value(), int64(8));  // 125 mod 13 = 8
}

TEST(NumberTheory, BaseConversion) {
    CHECK_STREQ(toBase(255, 16).value(), "FF");
    CHECK_STREQ(toBase(255, 2).value(), "11111111");
    CHECK_STREQ(toBase(100, 10).value(), "100");
    CHECK_STREQ(toBase(0, 2).value(), "0");
    CHECK_STREQ(toBase(-255, 16).value(), "-FF");

    CHECK_EQ(fromBase("FF", 16).value(), int64(255));
    CHECK_EQ(fromBase("11111111", 2).value(), int64(255));
    CHECK_EQ(fromBase("777", 8).value(), int64(511));
    CHECK_EQ(fromBase("-42", 10).value(), int64(-42));

    // 非法输入。
    CHECK(fromBase("2", 2).has_value() == false);
    CHECK(fromBase("999", 8).has_value() == false);
    CHECK(fromBase("G", 16).has_value() == false);   // G = 16，超过 16 进制上限
    CHECK(fromBase("", 10).has_value() == false);
    CHECK(toBase(10, 1).has_value() == false);
    CHECK(toBase(10, 37).has_value() == false);

    // base 36 的最大数字是 Z（值 35），因此 "ZZ" 是合法的。
    CHECK_EQ(fromBase("ZZ", 36).value(), int64(35 * 36 + 35));
}

TEST(ComplexMath, Power) {
    auto squared = complexPow(Complex(1, 1), 2);
    CHECK(squared.has_value());
    CHECK_NEAR(squared->real(), 0.0, 1e-12);
    CHECK_NEAR(squared->imag(), 2.0, 1e-12);

    auto cubed = complexPow(Complex(0, 1), 3);
    CHECK(cubed.has_value());
    CHECK_NEAR(cubed->real(), 0.0, 1e-12);
    CHECK_NEAR(cubed->imag(), -1.0, 1e-12);

    // 负指数需要非零底数。
    CHECK(complexPow(Complex(0, 0), -1).has_value() == false);
}

TEST(ComplexMath, Roots) {
    auto roots = complexRoots(Complex(1, 0), 3);
    CHECK_EQ(roots.size(), size_t(3));
    for (const auto& root : roots) {
        // 每个根的三次方应为 1。
        const Complex cubed = root * root * root;
        CHECK_NEAR(cubed.real(), 1.0, 1e-9);
        CHECK_NEAR(cubed.imag(), 0.0, 1e-9);
    }
}

TEST(ComplexMath, PolarForms) {
    auto polar = complexPolar(Complex(3, 4));
    CHECK_NEAR(polar.first, 5.0, 1e-12);
    CHECK_NEAR(polar.second, std::atan2(4.0, 3.0), 1e-12);

    auto back = complexFromPolar(5.0, std::atan2(4.0, 3.0));
    CHECK_NEAR(back.real(), 3.0, 1e-12);
    CHECK_NEAR(back.imag(), 4.0, 1e-12);
}
