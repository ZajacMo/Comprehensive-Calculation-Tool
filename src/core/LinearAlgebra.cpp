#include "LinearAlgebra.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "Numeric.h"

namespace calc {
namespace {

// 用部分选主元的高斯消元求行列式。
double determinantGaussian(std::vector<double> a, size_t n) {
    double det = 1.0;
    for (size_t col = 0; col < n; ++col) {
        // 选绝对值最大的行作为主元，减小数值误差。
        size_t pivot = col;
        for (size_t row = col + 1; row < n; ++row) {
            if (std::fabs(a[row * n + col]) > std::fabs(a[pivot * n + col])) {
                pivot = row;
            }
        }
        if (std::fabs(a[pivot * n + col]) < 1e-12) return 0.0;

        if (pivot != col) {
            for (size_t k = 0; k < n; ++k) {
                std::swap(a[pivot * n + k], a[col * n + k]);
            }
            det = -det;
        }

        det *= a[col * n + col];
        for (size_t row = col + 1; row < n; ++row) {
            const double factor = a[row * n + col] / a[col * n + col];
            for (size_t k = col; k < n; ++k) {
                a[row * n + k] -= factor * a[col * n + k];
            }
        }
    }
    return det;
}

// 高斯-约当消元，把系数矩阵化为行最简形。
// 返回化简后的矩阵，pivots 记录每行主元所在的列。
std::vector<double> rref(std::vector<double> a, size_t rows, size_t cols,
                         std::vector<int>& pivots) {
    pivots.clear();
    size_t pivotRow = 0;
    std::vector<size_t> pivotColumns;

    for (size_t col = 0; col < cols && pivotRow < rows; ++col) {
        size_t best = pivotRow;
        for (size_t row = pivotRow; row < rows; ++row) {
            if (std::fabs(a[row * cols + col]) > std::fabs(a[best * cols + col])) {
                best = row;
            }
        }
        if (std::fabs(a[best * cols + col]) < 1e-12) continue;

        for (size_t k = 0; k < cols; ++k) {
            std::swap(a[best * cols + k], a[pivotRow * cols + k]);
        }

        const double pivotValue = a[pivotRow * cols + col];
        for (size_t k = 0; k < cols; ++k) {
            a[pivotRow * cols + k] /= pivotValue;
        }

        for (size_t row = 0; row < rows; ++row) {
            if (row == pivotRow) continue;
            const double factor = a[row * cols + col];
            if (std::fabs(factor) < 1e-15) continue;
            for (size_t k = 0; k < cols; ++k) {
                a[row * cols + k] -= factor * a[pivotRow * cols + k];
            }
        }

        pivotColumns.push_back(static_cast<int>(col));
        pivots.push_back(static_cast<int>(col));
        ++pivotRow;
    }
    return a;
}

}  // namespace

Matrix::Matrix(size_t rows, size_t cols, double fill)
    : m_rows(rows), m_cols(cols), m_data(rows * cols, fill) {}

double Matrix::at(size_t row, size_t col) const {
    if (row >= m_rows || col >= m_cols) return 0.0;
    return m_data[row * m_cols + col];
}

void Matrix::set(size_t row, size_t col, double value) {
    if (row >= m_rows || col >= m_cols) return;
    m_data[row * m_cols + col] = value;
}

std::vector<double> Matrix::row(size_t index) const {
    if (index >= m_rows) return {};
    return std::vector<double>(m_data.begin() + static_cast<long>(index * m_cols),
                               m_data.begin() + static_cast<long>((index + 1) * m_cols));
}

std::vector<double> Matrix::column(size_t index) const {
    if (index >= m_cols) return {};
    std::vector<double> values;
    values.reserve(m_rows);
    for (size_t r = 0; r < m_rows; ++r) {
        values.push_back(at(r, index));
    }
    return values;
}

Matrix Matrix::operator+(const Matrix& other) const {
    if (m_rows != other.m_rows || m_cols != other.m_cols) return Matrix();
    Matrix result(m_rows, m_cols);
    for (size_t i = 0; i < m_data.size(); ++i) {
        result.m_data[i] = m_data[i] + other.m_data[i];
    }
    return result;
}

Matrix Matrix::operator-(const Matrix& other) const {
    if (m_rows != other.m_rows || m_cols != other.m_cols) return Matrix();
    Matrix result(m_rows, m_cols);
    for (size_t i = 0; i < m_data.size(); ++i) {
        result.m_data[i] = m_data[i] - other.m_data[i];
    }
    return result;
}

Matrix Matrix::operator*(double scalar) const {
    Matrix result(m_rows, m_cols);
    for (size_t i = 0; i < m_data.size(); ++i) {
        result.m_data[i] = m_data[i] * scalar;
    }
    return result;
}

Matrix Matrix::operator*(const Matrix& other) const {
    if (m_cols != other.m_rows) return Matrix();
    Matrix result(m_rows, other.m_cols);
    for (size_t r = 0; r < m_rows; ++r) {
        for (size_t c = 0; c < other.m_cols; ++c) {
            double sum = 0.0;
            for (size_t k = 0; k < m_cols; ++k) {
                sum += at(r, k) * other.at(k, c);
            }
            result.set(r, c, sum);
        }
    }
    return result;
}

Matrix Matrix::transposed() const {
    Matrix result(m_cols, m_rows);
    for (size_t r = 0; r < m_rows; ++r) {
        for (size_t c = 0; c < m_cols; ++c) {
            result.set(c, r, at(r, c));
        }
    }
    return result;
}

double Matrix::determinant() const {
    if (m_rows != m_cols || m_rows == 0) return 0.0;
    return determinantGaussian(m_data, m_rows);
}

std::optional<Matrix> Matrix::inverse() const {
    if (m_rows != m_cols || m_rows == 0) return std::nullopt;
    const size_t n = m_rows;

    // 构造增广矩阵 [A | I]，再对左半部分做高斯-约当消元。
    std::vector<double> augmented(n * 2 * n, 0.0);
    for (size_t r = 0; r < n; ++r) {
        for (size_t c = 0; c < n; ++c) {
            augmented[r * 2 * n + c] = at(r, c);
            augmented[r * 2 * n + n + c] = (r == c) ? 1.0 : 0.0;
        }
    }

    for (size_t col = 0; col < n; ++col) {
        size_t pivot = col;
        for (size_t row = col + 1; row < n; ++row) {
            if (std::fabs(augmented[row * 2 * n + col]) > std::fabs(augmented[pivot * 2 * n + col])) {
                pivot = row;
            }
        }
        if (std::fabs(augmented[pivot * 2 * n + col]) < 1e-12) {
            return std::nullopt;  // 奇异矩阵
        }

        for (size_t k = 0; k < 2 * n; ++k) {
            std::swap(augmented[pivot * 2 * n + k], augmented[col * 2 * n + k]);
        }

        const double pivotValue = augmented[col * 2 * n + col];
        for (size_t k = 0; k < 2 * n; ++k) {
            augmented[col * 2 * n + k] /= pivotValue;
        }

        for (size_t row = 0; row < n; ++row) {
            if (row == col) continue;
            const double factor = augmented[row * 2 * n + col];
            if (std::fabs(factor) < 1e-15) continue;
            for (size_t k = 0; k < 2 * n; ++k) {
                augmented[row * 2 * n + k] -= factor * augmented[col * 2 * n + k];
            }
        }
    }

    Matrix result(n, n);
    for (size_t r = 0; r < n; ++r) {
        for (size_t c = 0; c < n; ++c) {
            result.set(r, c, augmented[r * 2 * n + n + c]);
        }
    }
    return result;
}

size_t Matrix::rank() const {
    if (empty()) return 0;
    std::vector<int> pivots;
    const auto reduced = rref(m_data, m_rows, m_cols, pivots);
    return static_cast<size_t>(pivots.size());
}

bool Matrix::isSymmetric() const {
    if (m_rows != m_cols) return false;
    for (size_t r = 0; r < m_rows; ++r) {
        for (size_t c = r + 1; c < m_cols; ++c) {
            if (!nearlyEqual(at(r, c), at(c, r), 1e-9)) return false;
        }
    }
    return true;
}

std::string Matrix::describe() const {
    std::ostringstream out;
    out << "[";
    for (size_t r = 0; r < m_rows; ++r) {
        if (r > 0) out << ", ";
        out << "[";
        for (size_t c = 0; c < m_cols; ++c) {
            if (c > 0) out << ", ";
            out << formatReal(at(r, c), 4);
        }
        out << "]";
    }
    out << "]";
    return out.str();
}

Matrix makeMatrix(const std::vector<std::vector<double>>& rows) {
    if (rows.empty()) return Matrix();
    const size_t cols = rows.front().size();
    // 各行长度不一致时按最短的一行截断，避免读越界。
    size_t minCols = cols;
    for (const auto& r : rows) {
        minCols = std::min(minCols, r.size());
    }
    Matrix result(rows.size(), minCols);
    for (size_t r = 0; r < rows.size(); ++r) {
        for (size_t c = 0; c < minCols; ++c) {
            result.set(r, c, rows[r][c]);
        }
    }
    return result;
}

LinearSystemResult solveLinearSystemDetailed(const Matrix& a, const std::vector<double>& b) {
    LinearSystemResult result;
    const size_t n = a.cols();
    if (a.rows() != n || b.size() != n) {
        result.reason = "系数矩阵必须是 n×n 方阵，且右端项长度必须等于 n。";
        return result;
    }

    // 增广矩阵 [A | b]
    std::vector<double> augmented(n * (n + 1), 0.0);
    for (size_t r = 0; r < n; ++r) {
        for (size_t c = 0; c < n; ++c) {
            augmented[r * (n + 1) + c] = a.at(r, c);
        }
        augmented[r * (n + 1) + n] = b[r];
    }

    std::vector<int> pivotColumns;
    const auto reduced = rref(augmented, n, n + 1, pivotColumns);

    // 检查是否出现 0 = c（c ≠ 0）的矛盾行。
    for (size_t r = 0; r < n; ++r) {
        bool allZero = true;
        for (size_t c = 0; c < n; ++c) {
            if (std::fabs(reduced[r * (n + 1) + c]) > 1e-12) {
                allZero = false;
                break;
            }
        }
        if (allZero && std::fabs(reduced[r * (n + 1) + n]) > 1e-12) {
            result.reason = "方程组无解（增广矩阵出现矛盾行）。";
            return result;
        }
    }

    result.valid = true;
    result.solution.assign(n, 0.0);

    std::vector<bool> isPivot(n, false);
    for (int col : pivotColumns) {
        isPivot[static_cast<size_t>(col)] = true;
    }

    for (size_t row = 0; row < pivotColumns.size(); ++row) {
        const size_t col = static_cast<size_t>(pivotColumns[row]);
        result.solution[col] = reduced[row * (n + 1) + n];
    }

    for (size_t col = 0; col < n; ++col) {
        if (!isPivot[col]) {
            result.freeVariables.push_back(static_cast<int>(col));
        }
    }
    result.hasInfiniteSolutions = !result.freeVariables.empty();
    if (result.hasInfiniteSolutions) {
        result.reason = "方程组有无穷多解，自由变量个数为 " +
                        std::to_string(result.freeVariables.size()) + "。";
    }
    return result;
}

std::optional<std::vector<double>> solveLinearSystem(const Matrix& a, const std::vector<double>& b) {
    const auto detailed = solveLinearSystemDetailed(a, b);
    if (!detailed.valid || detailed.hasInfiniteSolutions) return std::nullopt;
    return detailed.solution;
}

}  // namespace calc
