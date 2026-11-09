#include "Sequences.h"

#include <algorithm>
#include <limits>

namespace calc {
namespace {

// int64 乘法的溢出保护：检查 a*b 是否超出可表示范围。
bool safeMultiply(int64 a, int64 b, int64& out) {
    if (a == 0 || b == 0) {
        out = 0;
        return true;
    }
    const int64 limit = std::numeric_limits<int64>::max();
    if (a > 0 && b > 0) {
        if (a > limit / b) return false;
    } else if (a < 0 && b < 0) {
        if (a < limit / b) return false;  // b 为负，limit/b 为负
    } else if (a > 0 && b < 0) {
        if (b < std::numeric_limits<int64>::min() / a) return false;
    } else {
        if (a < std::numeric_limits<int64>::min() / b) return false;
    }
    out = a * b;
    return true;
}

bool safeAdd(int64 a, int64 b, int64& out) {
    const int64 limit = std::numeric_limits<int64>::max();
    if (b > 0 && a > limit - b) return false;
    if (b < 0 && a < std::numeric_limits<int64>::min() - b) return false;
    out = a + b;
    return true;
}

}  // namespace

std::optional<int64> fibonacci(uint64_t n) {
    // F(92) 已接近 int64 上限，超过则返回空。
    if (n > 92) return std::nullopt;
    int64 previous = 0;
    int64 current = 1;
    for (uint64_t i = 0; i < n; ++i) {
        int64 next = 0;
        if (!safeAdd(previous, current, next)) return std::nullopt;
        previous = current;
        current = next;
    }
    return previous;
}

std::optional<int64> lucas(uint64_t n) {
    if (n > 91) return std::nullopt;
    int64 previous = 2;
    int64 current = 1;
    for (uint64_t i = 0; i < n; ++i) {
        int64 next = 0;
        if (!safeAdd(previous, current, next)) return std::nullopt;
        previous = current;
        current = next;
    }
    return previous;
}

std::optional<int64> binomial(uint64_t n, uint64_t k) {
    if (k > n) return std::nullopt;
    const uint64_t reduced = std::min(k, n - k);
    int64 result = 1;
    for (uint64_t i = 0; i < reduced; ++i) {
        // result = result * (n - i) / (i + 1)
        int64 product = 0;
        if (!safeMultiply(result, static_cast<int64>(n - i), product)) return std::nullopt;
        const int64 divisor = static_cast<int64>(i + 1);
        if (product % divisor != 0) return std::nullopt;
        result = product / divisor;
    }
    return result;
}

std::optional<int64> catalan(uint64_t n) {
    const auto choose = binomial(2 * n, n);
    if (!choose.has_value()) return std::nullopt;
    if ((*choose) % static_cast<int64>(n + 1) != 0) return std::nullopt;
    return (*choose) / static_cast<int64>(n + 1);
}

std::optional<int64> factorial(uint64_t n) {
    if (n > 20) return std::nullopt;  // 21! 已超出 int64
    int64 result = 1;
    for (uint64_t i = 2; i <= n; ++i) {
        int64 product = 0;
        if (!safeMultiply(result, static_cast<int64>(i), product)) return std::nullopt;
        result = product;
    }
    return result;
}

std::optional<int64> permutation(uint64_t n, uint64_t k) {
    if (k > n) return std::nullopt;
    int64 result = 1;
    for (uint64_t i = 0; i < k; ++i) {
        int64 product = 0;
        if (!safeMultiply(result, static_cast<int64>(n - i), product)) return std::nullopt;
        result = product;
    }
    return result;
}

std::optional<int64> integerPartition(uint64_t n) {
    // 动态规划：dp[j] 表示用当前已允许的数凑出 j 的方案数。
    std::vector<int64> dp(n + 1, 0);
    dp[0] = 1;
    for (uint64_t part = 1; part <= n; ++part) {
        for (uint64_t sum = part; sum <= n; ++sum) {
            int64 next = 0;
            if (!safeAdd(dp[sum], dp[sum - part], next)) return std::nullopt;
            dp[sum] = next;
        }
    }
    return dp[n];
}

std::vector<int64> fibonacciSequence(uint64_t count) {
    std::vector<int64> values;
    values.reserve(static_cast<size_t>(count));
    int64 previous = 0;
    int64 current = 1;
    for (uint64_t i = 0; i < count; ++i) {
        values.push_back(previous);
        int64 next = 0;
        if (!safeAdd(previous, current, next)) break;
        previous = current;
        current = next;
    }
    return values;
}

std::vector<int64> pellSequence(uint64_t count) {
    // P(0)=0, P(1)=1, P(n) = 2*P(n-1) + P(n-2)
    std::vector<int64> values;
    values.reserve(static_cast<size_t>(count));
    int64 previous = 0;
    int64 current = 1;
    for (uint64_t i = 0; i < count; ++i) {
        values.push_back(previous);
        int64 next = 0;
        if (!safeMultiply(current, 2, next)) break;
        int64 sum = 0;
        if (!safeAdd(next, previous, sum)) break;
        previous = current;
        current = sum;
    }
    return values;
}

}  // namespace calc
