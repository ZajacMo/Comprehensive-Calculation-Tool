// 数列与组合核心：斐波那契、卢卡斯、卡特兰数、组合数、阶乘、排列。
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Numeric.h"

namespace calc {

// 斐波那契数列。n 从 0 开始，F(0)=0, F(1)=1。
// n 过大时返回 nullopt（避免溢出）。
std::optional<int64> fibonacci(uint64_t n);

// 卢卡斯数列，L(0)=2, L(1)=1。
std::optional<int64> lucas(uint64_t n);

// 卡特兰数 C(n) = (1/(n+1)) * C(2n, n)。
std::optional<int64> catalan(uint64_t n);

// 二项式系数 C(n, k)。
std::optional<int64> binomial(uint64_t n, uint64_t k);

// 阶乘 n!。
std::optional<int64> factorial(uint64_t n);

// 排列数 A(n, k) = n! / (n-k)!。
std::optional<int64> permutation(uint64_t n, uint64_t k);

// 整数分区数 p(n)。
std::optional<int64> integerPartition(uint64_t n);

// 生成前 count 项的数列，供界面展示趋势。
std::vector<int64> fibonacciSequence(uint64_t count);
std::vector<int64> pellSequence(uint64_t count);

}  // namespace calc
