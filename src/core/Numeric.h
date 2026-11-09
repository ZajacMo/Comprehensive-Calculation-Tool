// 数值基础类型：复数、浮点容差比较、统一格式化。
#pragma once

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <string>

namespace calc {

using Complex = std::complex<double>;

// 整型别名集中定义在此，避免各模块重复 typedef 造成冲突。
using int64 = std::int64_t;
using uint64 = std::uint64_t;

// MSVC 不提供 M_PI，这里统一使用自有常量。
constexpr double kPi = 3.14159265358979323846;

// 浮点比较容差。回归与几何的中间量多来自用户输入，相对容���比绝对容差稳健。
inline bool nearlyEqual(double a, double b, double tol = 1e-9) {
    if (a == b) return true;
    double diff = std::fabs(a - b);
    double scale = std::max({std::fabs(a), std::fabs(b), 1.0});
    return diff <= tol * scale;
}

// 定点小数格式化。带符号时保留符号，负零输出为 0。
std::string formatReal(double value, int precision = 6);

// 复数格式化：虚部为零只显示实部，实部为零只显示虚部。
std::string formatComplex(const Complex& z, int precision = 6);

}  // namespace calc
