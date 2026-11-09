// 统计核心：描述统计、线性回归、相关分析。
#pragma once

#include <optional>
#include <string>
#include <vector>

namespace calc {

// 一个二维观测点。
struct Point {
    double x = 0.0;
    double y = 0.0;
};

// 线性回归结果。valid 为 false 表示输入不足以拟合。
struct LinearRegression {
    bool valid = false;
    std::string reason;       // 无效原因（中文，可直接显示）
    double meanX = 0.0;
    double meanY = 0.0;
    double slope = 0.0;       // k
    double intercept = 0.0;   // b
    double r = 0.0;           // 相关系数
    double rSquared = 0.0;    // 决定系数
    double stdDevX = 0.0;     // 样本标准差
    double stdDevY = 0.0;
    double slopeStdError = 0.0;  // k 的标准误
    double interceptStdError = 0.0;
    size_t sampleSize = 0;

    // 由回归方程预测 y。
    double predict(double x) const { return slope * x + intercept; }
};

// 描述统计量。
struct DescriptiveStats {
    bool valid = false;
    size_t count = 0;
    double mean = 0.0;
    double median = 0.0;
    double variance = 0.0;   // 样本方差（分母 n-1）
    double stdDev = 0.0;
    double min = 0.0;
    double max = 0.0;
    double range = 0.0;
    double sum = 0.0;
    double q1 = 0.0;
    double q3 = 0.0;
    double iqr = 0.0;
};

// 单变量描述统计。空输入返回 valid == false。
DescriptiveStats describe(const std::vector<double>& values);

// 双变量描述统计（x 与 y 分别统计）。
DescriptiveStats describeX(const std::vector<Point>& points);
DescriptiveStats describeY(const std::vector<Point>& points);

// 最小二乘线性回归。点数不足或 x 无变化时返回无效结果。
LinearRegression fitLinearRegression(const std::vector<Point>& points);

// 皮尔逊相关系数。点数不足或任一变量无变化时返回 nullopt。
std::optional<double> pearsonCorrelation(const std::vector<Point>& points);

}  // namespace calc
