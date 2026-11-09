// 数论核心：质因数分解、素性判定、欧拉函数、莫比乌斯函数、约数统计。
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace calc {

using int64 = std::int64_t;
using uint64 = std::uint64_t;

// 一个质因数及其指数，例如 {3, 2} 表示 3^2。
struct PrimeFactor {
    int64 prime = 0;
    int exponent = 0;
};

// 质因数分解结果。n > 1 时 factors 非空且乘积等于 n。
struct Factorization {
    int64 value = 0;
    std::vector<PrimeFactor> factors;

    // 还原原数，验证分解自洽。
    int64 reconstruct() const;
    // 形如 "2^3 * 3^2" 的可读表示。
    std::string describe() const;
};

// 对 n 做质因数分解。n < 2 返回空结果。
Factorization factorize(int64 n);

// 确定性 Miller-Rabin 素性判定，对 64 位无符号整数范围正确。
bool isPrime(uint64 n);

// 欧拉函数 φ(n)：1..n 中与 n 互质的整数个数。
int64 eulerPhi(int64 n);

// 莫比乌斯函数 μ(n)。
int64 mobius(int64 n);

// 正约数个数。
int64 divisorCount(int64 n);

// 正约数之和。
int64 divisorSum(int64 n);

int64 gcd(int64 a, int64 b);
int64 lcm(int64 a, int64 b);

}  // namespace calc
