#include "Statistics.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "Numeric.h"

namespace calc {
namespace {

// 线性插值分位数（与 numpy 默认的 linear 插值一致）。
double quantile(std::vector<double> sorted, double q) {
    if (sorted.empty()) return 0.0;
    if (sorted.size() == 1) return sorted.front();
    double position = q * static_cast<double>(sorted.size() - 1);
    size_t lower = static_cast<size_t>(std::floor(position));
    size_t upper = static_cast<size_t>(std::ceil(position));
    double fraction = position - static_cast<double>(lower);
    return sorted[lower] * (1.0 - fraction) + sorted[upper] * fraction;
}

}  // namespace

DescriptiveStats describe(const std::vector<double>& values) {
    DescriptiveStats stats;
    if (values.empty()) return stats;

    std::vector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());

    stats.valid = true;
    stats.count = values.size();
    stats.sum = std::accumulate(values.begin(), values.end(), 0.0);
    stats.mean = stats.sum / static_cast<double>(values.size());
    stats.min = sorted.front();
    stats.max = sorted.back();
    stats.range = stats.max - stats.min;
    stats.median = quantile(sorted, 0.5);
    stats.q1 = quantile(sorted, 0.25);
    stats.q3 = quantile(sorted, 0.75);
    stats.iqr = stats.q3 - stats.q1;

    if (values.size() > 1) {
        double squaredSum = 0.0;
        for (double v : values) {
            double delta = v - stats.mean;
            squaredSum += delta * delta;
        }
        stats.variance = squaredSum / static_cast<double>(values.size() - 1);
        stats.stdDev = std::sqrt(stats.variance);
    }
    return stats;
}

DescriptiveStats describeX(const std::vector<Point>& points) {
    std::vector<double> xs;
    xs.reserve(points.size());
    for (const auto& p : points) xs.push_back(p.x);
    return describe(xs);
}

DescriptiveStats describeY(const std::vector<Point>& points) {
    std::vector<double> ys;
    ys.reserve(points.size());
    for (const auto& p : points) ys.push_back(p.y);
    return describe(ys);
}

LinearRegression fitLinearRegression(const std::vector<Point>& points) {
    LinearRegression result;
    result.sampleSize = points.size();

    if (points.size() < 2) {
        result.reason = "至少需要两个数据点才能拟合直线。";
        return result;
    }

    double sumX = 0.0, sumY = 0.0;
    for (const auto& p : points) {
        sumX += p.x;
        sumY += p.y;
    }
    const double n = static_cast<double>(points.size());
    result.meanX = sumX / n;
    result.meanY = sumY / n;

    double sxx = 0.0, syy = 0.0, sxy = 0.0;
    for (const auto& p : points) {
        const double dx = p.x - result.meanX;
        const double dy = p.y - result.meanY;
        sxx += dx * dx;
        syy += dy * dy;
        sxy += dx * dy;
    }

    if (nearlyEqual(sxx, 0.0)) {
        result.reason = "所有数据点的 x 值相同，无法确定斜率。";
        return result;
    }

    result.slope = sxy / sxx;
    result.intercept = result.meanY - result.slope * result.meanX;
    result.r = (nearlyEqual(syy, 0.0)) ? 0.0 : sxy / std::sqrt(sxx * syy);
    result.rSquared = result.r * result.r;

    const double sampleVarianceX = sxx / (n - 1.0);
    result.stdDevX = std::sqrt(sampleVarianceX);
    result.stdDevY = std::sqrt(syy / (n - 1.0));

    // 斜率标准误 se(k) = sqrt( (Σ(y-ŷ)²/(n-2)) / Sxx )
    if (points.size() > 2) {
        double residualSum = 0.0;
        for (const auto& p : points) {
            const double residual = p.y - (result.slope * p.x + result.intercept);
            residualSum += residual * residual;
        }
        const double residualVariance = residualSum / (n - 2.0);
        result.slopeStdError = std::sqrt(residualVariance / sxx);
        result.interceptStdError = result.slopeStdError * std::sqrt(result.meanX * result.meanX + residualVariance / sxx);
    }

    result.valid = true;
    return result;
}

std::optional<double> pearsonCorrelation(const std::vector<Point>& points) {
    if (points.size() < 2) return std::nullopt;

    double sumX = 0.0, sumY = 0.0;
    for (const auto& p : points) {
        sumX += p.x;
        sumY += p.y;
    }
    const double n = static_cast<double>(points.size());
    const double meanX = sumX / n;
    const double meanY = sumY / n;

    double sxx = 0.0, syy = 0.0, sxy = 0.0;
    for (const auto& p : points) {
        const double dx = p.x - meanX;
        const double dy = p.y - meanY;
        sxx += dx * dx;
        syy += dy * dy;
        sxy += dx * dy;
    }
    if (nearlyEqual(sxx, 0.0) || nearlyEqual(syy, 0.0)) return std::nullopt;
    return sxy / std::sqrt(sxx * syy);
}

}  // namespace calc
