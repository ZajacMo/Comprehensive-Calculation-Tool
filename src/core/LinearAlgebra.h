// 线性代数核心：矩阵运算、行列式、逆矩阵、线性方程组。
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace calc {

// 稠密矩阵，按行存储。
class Matrix {
public:
    Matrix() = default;
    Matrix(size_t rows, size_t cols, double fill = 0.0);

    size_t rows() const { return m_rows; }
    size_t cols() const { return m_cols; }
    bool empty() const { return m_rows == 0 || m_cols == 0; }

    // 越界访问返回 0，便于调用方简化边界判断。
    double at(size_t row, size_t col) const;
    void set(size_t row, size_t col, double value);

    // 按行/列读取，越界时返回空向量。
    std::vector<double> row(size_t index) const;
    std::vector<double> column(size_t index) const;

    Matrix operator+(const Matrix& other) const;
    Matrix operator-(const Matrix& other) const;
    Matrix operator*(double scalar) const;

    // 矩阵乘法。维度不匹配时返回空矩阵。
    Matrix operator*(const Matrix& other) const;

    Matrix transposed() const;

    // 求逆。奇异矩阵返回 nullopt。
    std::optional<Matrix> inverse() const;

    // 行列式。非方阵返回 0。
    double determinant() const;

    // 秩（行阶梯化后的非零行数）。
    size_t rank() const;

    // 是否为对称方阵。
    bool isSymmetric() const;

    // 形如 "[[1, 2], [3, 4]]" 的可读表示。
    std::string describe() const;

private:
    size_t m_rows = 0;
    size_t m_cols = 0;
    std::vector<double> m_data;
};

// 用嵌套 vector 构造矩阵。
Matrix makeMatrix(const std::vector<std::vector<double>>& rows);

// 求解线性方程组 Ax = b。无可解或多解时返回 nullopt。
std::optional<std::vector<double>> solveLinearSystem(const Matrix& a,
                                                      const std::vector<double>& b);

// 高斯-约当消元求解，附带解的自由变量信息。
struct LinearSystemResult {
    bool valid = false;
    std::string reason;
    std::vector<double> solution;
    bool hasInfiniteSolutions = false;
    std::vector<int> freeVariables;  // 下标从 0 开始的自由变量列
};

LinearSystemResult solveLinearSystemDetailed(const Matrix& a, const std::vector<double>& b);

}  // namespace calc
