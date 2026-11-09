#include "test_framework.h"

#include "core/Statistics.h"

using namespace calc;

TEST(Statistics, DescribeBasic) {
    auto stats = describe({1.0, 2.0, 3.0, 4.0, 5.0});
    CHECK(stats.valid);
    CHECK_EQ(stats.count, size_t(5));
    CHECK_NEAR(stats.mean, 3.0, 1e-12);
    CHECK_NEAR(stats.median, 3.0, 1e-12);
    CHECK_NEAR(stats.min, 1.0, 1e-12);
    CHECK_NEAR(stats.max, 5.0, 1e-12);
    CHECK_NEAR(stats.range, 4.0, 1e-12);
    CHECK_NEAR(stats.sum, 15.0, 1e-12);
    // 样本方差：分母为 n-1
    CHECK_NEAR(stats.variance, 2.5, 1e-12);
    CHECK_NEAR(stats.stdDev, std::sqrt(2.5), 1e-12);
}

TEST(Statistics, DescribeEmpty) {
    auto stats = describe({});
    CHECK(stats.valid == false);
    CHECK_EQ(stats.count, size_t(0));
}

TEST(Statistics, DescribeSingleValue) {
    auto stats = describe({42.0});
    CHECK(stats.valid);
    CHECK_NEAR(stats.mean, 42.0, 1e-12);
    CHECK_NEAR(stats.median, 42.0, 1e-12);
    CHECK_NEAR(stats.variance, 0.0, 1e-12);
}

TEST(Statistics, MedianOfEvenCount) {
    auto stats = describe({1.0, 2.0, 3.0, 4.0});
    CHECK_NEAR(stats.median, 2.5, 1e-12);
    CHECK_NEAR(stats.q1, 1.75, 1e-12);
    CHECK_NEAR(stats.q3, 3.25, 1e-12);
    CHECK_NEAR(stats.iqr, 1.5, 1e-12);
}

TEST(Statistics, RegressionPerfectLine) {
    // y = 2x + 1，完美线性。
    std::vector<Point> points = {{0, 1}, {1, 3}, {2, 5}, {3, 7}};
    auto fit = fitLinearRegression(points);
    CHECK(fit.valid);
    CHECK_NEAR(fit.slope, 2.0, 1e-9);
    CHECK_NEAR(fit.intercept, 1.0, 1e-9);
    CHECK_NEAR(fit.r, 1.0, 1e-9);
    CHECK_NEAR(fit.rSquared, 1.0, 1e-9);
    CHECK_NEAR(fit.predict(4.0), 9.0, 1e-9);
    CHECK_EQ(fit.sampleSize, size_t(4));
}

TEST(Statistics, RegressionNoisyLine) {
    std::vector<Point> points = {{1, 2.1}, {2, 3.9}, {3, 6.2}, {4, 7.8}, {5, 10.1}};
    auto fit = fitLinearRegression(points);
    CHECK(fit.valid);
    CHECK_NEAR(fit.slope, 2.0, 0.05);
    CHECK_NEAR(fit.intercept, 0.0, 0.15);
    CHECK(fit.r > 0.99);
    CHECK_NEAR(fit.rSquared, fit.r * fit.r, 1e-12);
}

TEST(Statistics, RegressionMatchesClosedForm) {
    // 与手算的最小二乘闭式解比对。
    std::vector<Point> points = {{2, 3}, {4, 7}, {6, 8}, {8, 12}};
    auto fit = fitLinearRegression(points);
    CHECK(fit.valid);

    const double n = 4.0;
    double sumX = 0, sumY = 0, sumXY = 0, sumXX = 0;
    for (const auto& p : points) {
        sumX += p.x;
        sumY += p.y;
        sumXY += p.x * p.y;
        sumXX += p.x * p.x;
    }
    const double expectedSlope = (n * sumXY - sumX * sumY) / (n * sumXX - sumX * sumX);
    const double expectedIntercept = (sumY - expectedSlope * sumX) / n;
    CHECK_NEAR(fit.slope, expectedSlope, 1e-9);
    CHECK_NEAR(fit.intercept, expectedIntercept, 1e-9);
}

TEST(Statistics, RegressionInsufficientPoints) {
    auto fit = fitLinearRegression({{1.0, 2.0}});
    CHECK(fit.valid == false);
    CHECK(!fit.reason.empty());

    auto empty = fitLinearRegression({});
    CHECK(empty.valid == false);
}

TEST(Statistics, RegressionConstantX) {
    // 所有 x 相同，斜率无定义。
    auto fit = fitLinearRegression({{1, 2}, {1, 3}, {1, 5}});
    CHECK(fit.valid == false);
    CHECK(!fit.reason.empty());
}

TEST(Statistics, RegressionAcrossPages) {
    // 回归原始实现分两页收集点时会在 elementnumber 为 5 的倍数时漏点，
    // 这里用 10 个点（两页各 5 个）验证结果正确。
    std::vector<Point> points;
    for (int i = 0; i < 10; ++i) {
        points.push_back({static_cast<double>(i), 3.0 * i + 2.0});
    }
    auto fit = fitLinearRegression(points);
    CHECK(fit.valid);
    CHECK_NEAR(fit.slope, 3.0, 1e-9);
    CHECK_NEAR(fit.intercept, 2.0, 1e-9);
    CHECK_EQ(fit.sampleSize, size_t(10));
}

TEST(Statistics, PearsonCorrelation) {
    auto r = pearsonCorrelation({{1, 2}, {2, 4}, {3, 6}});
    CHECK(r.has_value());
    CHECK_NEAR(*r, 1.0, 1e-9);

    auto negative = pearsonCorrelation({{1, 6}, {2, 4}, {3, 2}});
    CHECK(negative.has_value());
    CHECK_NEAR(*negative, -1.0, 1e-9);

    // 单点无法计算相关系数。
    CHECK(pearsonCorrelation({{1, 2}}).has_value() == false);
    // x 无变化同样无定义。
    CHECK(pearsonCorrelation({{1, 2}, {1, 5}, {1, 9}}).has_value() == false);
}

TEST(Statistics, DescribeXAndY) {
    std::vector<Point> points = {{1, 10}, {2, 20}, {3, 30}};
    auto xs = describeX(points);
    auto ys = describeY(points);
    CHECK_NEAR(xs.mean, 2.0, 1e-12);
    CHECK_NEAR(ys.mean, 20.0, 1e-12);
}
