#include "Geometry.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "Numeric.h"

namespace calc {
namespace {

// 解线性方程组（高斯消元，3x3）。奇异矩阵返回 false。
bool solve3x3(double m[3][3], double rhs[3], double out[3]) {
    for (int col = 0; col < 3; ++col) {
        int pivot = col;
        for (int row = col + 1; row < 3; ++row) {
            if (std::fabs(m[row][col]) > std::fabs(m[pivot][col])) pivot = row;
        }
        if (std::fabs(m[pivot][col]) < 1e-12) return false;
        if (pivot != col) {
            for (int k = 0; k < 3; ++k) std::swap(m[pivot][k], m[col][k]);
            std::swap(rhs[pivot], rhs[col]);
        }
        for (int row = col + 1; row < 3; ++row) {
            const double factor = m[row][col] / m[col][col];
            for (int k = col; k < 3; ++k) m[row][k] -= factor * m[col][k];
            rhs[row] -= factor * rhs[col];
        }
    }
    for (int row = 2; row >= 0; --row) {
        double sum = rhs[row];
        for (int k = row + 1; k < 3; ++k) sum -= m[row][k] * out[k];
        out[row] = sum / m[row][row];
    }
    return true;
}

// 三点构成平面的法向量。
Vec3 normalOf(const Vec3& a, const Vec3& b, const Vec3& c) {
    return (b - a).cross(c - a);
}

// 计算四面体四个面的面积。
double tetrahedronSurfaceArea(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d) {
    double total = 0.0;
    total += analyzeTriangle(a, b, c).area;
    total += analyzeTriangle(a, b, d).area;
    total += analyzeTriangle(a, c, d).area;
    total += analyzeTriangle(b, c, d).area;
    return total;
}

}  // namespace

double Vec3::norm() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vec3 Vec3::normalized() const {
    const double length = norm();
    if (length < 1e-15) return {0.0, 0.0, 0.0};
    return {x / length, y / length, z / length};
}

TriangleResult analyzeTriangle(const Vec3& a, const Vec3& b, const Vec3& c) {
    TriangleResult result;

    const Vec3 ab = b - a;
    const Vec3 ac = c - a;
    const Vec3 bc = c - b;

    result.sideA = bc.norm();
    result.sideB = ac.norm();
    result.sideC = ab.norm();
    result.perimeter = result.sideA + result.sideB + result.sideC;

    const Vec3 cross = ab.cross(ac);
    const double crossLength = cross.norm();
    result.area = crossLength * 0.5;

    // 叉积模为 0 表示三点共线或重合，属于退化情形。
    result.degenerate = crossLength < 1e-12;
    if (result.degenerate) {
        result.reason = "三点共线或重合，三角形退化，面积为 0。";
        result.altitude = 0.0;
        result.valid = true;
        return result;
    }

    // 以最长边为底，高 = 2 * 面积 / 底边长。
    const double longest = std::max({result.sideA, result.sideB, result.sideC});
    result.altitude = (longest > 1e-12) ? (2.0 * result.area / longest) : 0.0;
    result.valid = true;
    return result;
}

TetrahedronResult analyzeTetrahedron(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d) {
    TetrahedronResult result;

    const Vec3 ab = b - a;
    const Vec3 ac = c - a;
    const Vec3 ad = d - a;
    const double scalarTriple = ab.dot(ac.cross(ad));

    if (std::fabs(scalarTriple) < 1e-12) {
        result.degenerate = true;
        result.reason = "四点共面或重合，四面体退化，体积为 0。";
        result.valid = true;
        return result;
    }

    // 体积 = |标量三重积| / 6
    result.volume = std::fabs(scalarTriple) / 6.0;
    result.surfaceArea = tetrahedronSurfaceArea(a, b, c, d);
    result.centroid = (a + b + c + d) * 0.25;

    // 外接球球心：解 |O-a|² = |O-b|² = |O-c|² = |O-d|² 的线性方程组。
    double matrix[3][3] = {
        {2.0 * (b.x - a.x), 2.0 * (b.y - a.y), 2.0 * (b.z - a.z)},
        {2.0 * (c.x - a.x), 2.0 * (c.y - a.y), 2.0 * (c.z - a.z)},
        {2.0 * (d.x - a.x), 2.0 * (d.y - a.y), 2.0 * (d.z - a.z)},
    };
    double rhs[3] = {
        b.dot(b) - a.dot(a),
        c.dot(c) - a.dot(a),
        d.dot(d) - a.dot(a),
    };
    double center[3] = {0.0, 0.0, 0.0};
    if (solve3x3(matrix, rhs, center)) {
        result.circumcenter = {center[0], center[1], center[2]};
        result.circumradius = (result.circumcenter - a).norm();
    }

    // 内切球半径 r = 3V / S
    if (result.surfaceArea > 1e-12) {
        result.inradius = 3.0 * result.volume / result.surfaceArea;
    }

    result.valid = true;
    return result;
}

double polygonArea(const std::vector<Vec3>& vertices) {
    if (vertices.size() < 3) return 0.0;

    // Newell 法：对平面多边形给出精确面积，对非平面多边形给出面积向量的模，
    // 比投影法更稳定，也不依赖顶点共面的假设。
    Vec3 areaVector{0.0, 0.0, 0.0};
    const size_t n = vertices.size();
    for (size_t i = 0; i < n; ++i) {
        const Vec3& current = vertices[i];
        const Vec3& next = vertices[(i + 1) % n];
        areaVector.x += (current.y - next.y) * (current.z + next.z);
        areaVector.y += (current.z - next.z) * (current.x + next.x);
        areaVector.z += (current.x - next.x) * (current.y + next.y);
    }
    return areaVector.norm() * 0.5;
}

SolidResult analyzeSphere(double radius) {
    SolidResult result;
    if (radius <= 0.0) {
        result.reason = "半径必须为正数。";
        return result;
    }
    result.surfaceArea = 4.0 * kPi * radius * radius;
    result.volume = (4.0 / 3.0) * kPi * radius * radius * radius;
    result.valid = true;
    return result;
}

SolidResult analyzeCylinder(double radius, double height) {
    SolidResult result;
    if (radius <= 0.0 || height <= 0.0) {
        result.reason = "半径与高必须为正数。";
        return result;
    }
    result.surfaceArea = 2.0 * kPi * radius * radius + 2.0 * kPi * radius * height;
    result.volume = kPi * radius * radius * height;
    result.valid = true;
    return result;
}

SolidResult analyzeCone(double radius, double height) {
    SolidResult result;
    if (radius <= 0.0 || height <= 0.0) {
        result.reason = "半径与高必须为正数。";
        return result;
    }
    const double slant = std::sqrt(radius * radius + height * height);
    result.surfaceArea = kPi * radius * (radius + slant);
    result.volume = (1.0 / 3.0) * kPi * radius * radius * height;
    result.valid = true;
    return result;
}

SolidResult analyzePrism(double radius, double height, int sides) {
    SolidResult result;
    if (radius <= 0.0 || height <= 0.0) {
        result.reason = "半径与高必须为正数。";
        return result;
    }
    if (sides < 3) {
        result.reason = "正棱柱的边数至少为 3。";
        return result;
    }
    // radius 是底面外接圆半径。正 n 边形的边长与内切圆半径（apothem）分别为：
    const double side = 2.0 * radius * std::sin(kPi / sides);
    const double apothem = radius * std::cos(kPi / sides);
    // 底面积 = n/2 × 边长 × 内切圆半径
    const double baseArea = 0.5 * sides * side * apothem;
    result.surfaceArea = 2.0 * baseArea + sides * side * height;
    result.volume = baseArea * height;
    result.valid = true;
    return result;
}

SolidResult analyzePyramid(double radius, double height, int sides) {
    SolidResult result;
    if (radius <= 0.0 || height <= 0.0) {
        result.reason = "半径与高必须为正数。";
        return result;
    }
    if (sides < 3) {
        result.reason = "正棱锥的边数至少为 3。";
        return result;
    }
    const double side = 2.0 * radius * std::sin(kPi / sides);
    const double apothem = radius * std::cos(kPi / sides);
    const double baseArea = 0.5 * sides * side * apothem;
    // 侧面斜高：顶点到底边中点的距离，用底面内切圆半径而非外接圆半径。
    const double slantHeight = std::sqrt(height * height + apothem * apothem);
    // 每个侧面是底边为 side、高为 slantHeight 的等腰三角形
    const double lateralArea = 0.5 * sides * side * slantHeight;
    result.surfaceArea = baseArea + lateralArea;
    result.volume = (1.0 / 3.0) * baseArea * height;
    result.valid = true;
    return result;
}

}  // namespace calc
