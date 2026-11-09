#include "NumberTheory.h"

#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <sstream>

namespace calc {
namespace {

// 用试除法分解 n（n >= 2）。只需要试到 sqrt(n)，相比原始 O(n) 版本大幅提速。
Factorization factorizePositive(int64 n) {
    Factorization result;
    result.value = n;
    for (int64 p = 2; p <= n / p; p += (p == 2 ? 1 : 2)) {
        if (n % p != 0) continue;
        int exponent = 0;
        while (n % p == 0) {
            n /= p;
            ++exponent;
        }
        result.factors.push_back({p, exponent});
    }
    if (n > 1) {
        result.factors.push_back({n, 1});
    }
    return result;
}

// 模加法。a + b 可能溢出 uint64，用减法形式规避。
uint64 addMod(uint64 a, uint64 b, uint64 mod) {
    if (a >= mod - b) return a - (mod - b);
    return a + b;
}

// 模乘法。用倍增法实现，完全避开 128 位整数，MSVC 与 GCC 通用。
uint64 mulMod(uint64 a, uint64 b, uint64 mod) {
    uint64 result = 0;
    a %= mod;
    while (b > 0) {
        if (b & 1) result = addMod(result, a, mod);
        a = addMod(a, a, mod);
        b >>= 1;
    }
    return result;
}

uint64 powMod(uint64 base, uint64 exponent, uint64 mod) {
    uint64 result = 1;
    base %= mod;
    while (exponent > 0) {
        if (exponent & 1) result = mulMod(result, base, mod);
        base = mulMod(base, base, mod);
        exponent >>= 1;
    }
    return result;
}

}  // namespace

int64 Factorization::reconstruct() const {
    int64 product = 1;
    for (const auto& factor : factors) {
        for (int i = 0; i < factor.exponent; ++i) {
            product *= factor.prime;
        }
    }
    return product;
}

std::string Factorization::describe() const {
    if (factors.empty()) return "无质因数";
    std::ostringstream out;
    for (size_t i = 0; i < factors.size(); ++i) {
        if (i > 0) out << " * ";
        out << factors[i].prime;
        if (factors[i].exponent > 1) out << "^" << factors[i].exponent;
    }
    return out.str();
}

Factorization factorize(int64 n) {
    if (n < 2) return Factorization{n, {}};
    return factorizePositive(n);
}

bool isPrime(uint64 n) {
    if (n < 2) return false;
    for (uint64 p : {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL, 19ULL, 23ULL, 29ULL, 31ULL, 37ULL}) {
        if (n % p == 0) return n == p;
    }
    // n = 1 已排除，此处 n 为奇数且 > 37。
    uint64 d = n - 1;
    int r = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++r;
    }
    // 这些底数对 64 位无符号整数范围是确定性的。
    for (uint64 a : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL}) {
        if (a % n == 0) continue;
        uint64 x = powMod(a % n, d, n);
        if (x == 1 || x == n - 1) continue;
        bool composite = true;
        for (int i = 1; i < r; ++i) {
            x = mulMod(x, x, n);
            if (x == n - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

int64 eulerPhi(int64 n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    Factorization f = factorize(n);
    int64 result = n;
    for (const auto& factor : f.factors) {
        result = result / factor.prime * (factor.prime - 1);
    }
    return result;
}

int64 mobius(int64 n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    Factorization f = factorize(n);
    for (const auto& factor : f.factors) {
        if (factor.exponent > 1) return 0;
    }
    return f.factors.size() % 2 == 0 ? 1 : -1;
}

int64 divisorCount(int64 n) {
    if (n <= 0) return 0;
    Factorization f = factorize(n);
    int64 count = 1;
    for (const auto& factor : f.factors) {
        count *= (factor.exponent + 1);
    }
    return count;
}

int64 divisorSum(int64 n) {
    if (n <= 0) return 0;
    Factorization f = factorize(n);
    int64 sum = 1;
    for (const auto& factor : f.factors) {
        int64 term = 1;
        int64 power = 1;
        for (int i = 0; i < factor.exponent; ++i) {
            power *= factor.prime;
            term += power;
        }
        sum *= term;
    }
    return sum;
}

int64 gcd(int64 a, int64 b) {
    a = std::abs(a);
    b = std::abs(b);
    while (b != 0) {
        int64 t = a % b;
        a = b;
        b = t;
    }
    return a;
}

int64 lcm(int64 a, int64 b) {
    if (a == 0 || b == 0) return 0;
    return std::abs(a / gcd(a, b) * b);
}

}  // namespace calc
