// 复数运算核心：四则运算、幂、极坐标形式。
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Numeric.h"

namespace calc {

// 复数的幂。n 为负整数时要求 z != 0。
std::optional<Complex> complexPow(const Complex& base, int64 exponent);

// 复数的 n 次根，共 n 个。
std::vector<Complex> complexRoots(const Complex& value, int n);

// 复数的自然对数（主值）。
std::optional<Complex> complexLog(const Complex& value);

// 极坐标形式：返回 {模, 辐角(弧度)}。
std::pair<double, double> complexPolar(const Complex& value);

// 极坐标形式转直角坐标形式。
Complex complexFromPolar(double magnitude, double angle);

// 复数的模与辐角分别格式化，便于界面展示。
std::string describeComplex(const Complex& value, int precision = 6);

}  // namespace calc
