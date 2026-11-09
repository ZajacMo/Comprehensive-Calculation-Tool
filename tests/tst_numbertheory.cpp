#include "test_framework.h"

#include "core/NumberTheory.h"

using namespace calc;

TEST(NumberTheory, FactorizeSmallNumbers) {
    auto f = factorize(12);
    CHECK_EQ(f.factors.size(), size_t(2));
    CHECK_EQ(f.factors[0].prime, int64(2));
    CHECK_EQ(f.factors[0].exponent, 2);
    CHECK_EQ(f.factors[1].prime, int64(3));
    CHECK_EQ(f.factors[1].exponent, 1);
    CHECK_EQ(f.reconstruct(), int64(12));
    CHECK_STREQ(f.describe(), "2^2 * 3");
}

TEST(NumberTheory, FactorizeLargeSemiprime) {
    // 原始实现用 O(n) 试除，此规模会长时间无响应。
    const int64 n = 999999937LL;  // 素数
    auto f = factorize(n);
    CHECK_EQ(f.factors.size(), size_t(1));
    CHECK_EQ(f.factors[0].prime, n);
    CHECK_EQ(f.reconstruct(), n);
}

TEST(NumberTheory, FactorizeOneBillion) {
    const int64 n = 1000000000LL;
    auto f = factorize(n);
    CHECK_EQ(f.reconstruct(), n);
    CHECK_EQ(f.factors.size(), size_t(2));  // 2^9 * 5^9
    CHECK_EQ(f.factors[0].exponent, 9);
    CHECK_EQ(f.factors[1].exponent, 9);
}

TEST(NumberTheory, FactorizeInvalidInput) {
    CHECK(factorize(1).factors.empty());
    CHECK(factorize(0).factors.empty());
    CHECK(factorize(-5).factors.empty());
}

TEST(NumberTheory, IsPrimeBasics) {
    CHECK(isPrime(2));
    CHECK(isPrime(3));
    CHECK(isPrime(4) == false);
    CHECK(isPrime(9) == false);
    CHECK(isPrime(97));
    CHECK(isPrime(1) == false);
    CHECK(isPrime(0) == false);
}

TEST(NumberTheory, IsPrimeCarmichaelNumbers) {
    // Carmichael 数会被朴素的费马测试误判为素数。
    CHECK(isPrime(561) == false);
    CHECK(isPrime(1105) == false);
    CHECK(isPrime(1729) == false);
    CHECK(isPrime(2465) == false);
    CHECK(isPrime(2821) == false);
    CHECK(isPrime(6601) == false);
}

TEST(NumberTheory, IsPrimeLarge) {
    CHECK(isPrime(2147483647ULL));          // 2^31 - 1
    CHECK(isPrime(4294967291ULL));          // 最大的 32 位素数
    CHECK(isPrime(1000000007ULL));
    CHECK(isPrime(1000000009ULL));
    CHECK(isPrime(1000000000ULL) == false);
}

TEST(NumberTheory, EulerPhi) {
    CHECK_EQ(eulerPhi(1), int64(1));
    CHECK_EQ(eulerPhi(2), int64(1));
    CHECK_EQ(eulerPhi(10), int64(4));
    CHECK_EQ(eulerPhi(12), int64(4));
    CHECK_EQ(eulerPhi(97), int64(96));
    CHECK_EQ(eulerPhi(1000000000LL), int64(400000000));
}

TEST(NumberTheory, Mobius) {
    CHECK_EQ(mobius(1), int64(1));
    CHECK_EQ(mobius(2), int64(-1));
    CHECK_EQ(mobius(3), int64(-1));
    CHECK_EQ(mobius(4), int64(0));   // 含平方因子
    CHECK_EQ(mobius(6), int64(1));   // 两个不同质因子
    CHECK_EQ(mobius(30), int64(-1)); // 三个不同质因子
}

TEST(NumberTheory, DivisorCountAndSum) {
    CHECK_EQ(divisorCount(1), int64(1));
    CHECK_EQ(divisorCount(12), int64(6));
    CHECK_EQ(divisorCount(36), int64(9));
    CHECK_EQ(divisorCount(1000000000LL), int64(100));
    CHECK_EQ(divisorSum(12), int64(28));
    CHECK_EQ(divisorSum(28), int64(56));
}

TEST(NumberTheory, GcdAndLcm) {
    CHECK_EQ(gcd(12, 18), int64(6));
    CHECK_EQ(gcd(-12, 18), int64(6));
    CHECK_EQ(gcd(0, 5), int64(5));
    CHECK_EQ(lcm(4, 6), int64(12));
    CHECK_EQ(lcm(0, 6), int64(0));
}
