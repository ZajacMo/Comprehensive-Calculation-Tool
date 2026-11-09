// 空间几何核心：三维向量、三角形、四面体、常见几何体。
#pragma once

#include <optional>
#include <string>
#include <vector>

namespace calc {

// 三维向量。
struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(double s) const { return {x * s, y * s, z * s}; }
    double dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }
    double norm() const;
    Vec3 normalized() const;
};

// 三角形计算结果。
struct TriangleResult {
    bool valid = false;
    std::string reason;
    double area = 0.0;
    double perimeter = 0.0;
    double sideA = 0.0;   // |BC|
    double sideB = 0.0;   // |CA|
    double sideC = 0.0;   // |AB|
    double altitude = 0.0;  // 到底边的最大高（以最长边为底）
    bool degenerate = false; // 三点共线或重合
};

// 计算三维三角形的面积、周长与边长。
TriangleResult analyzeTriangle(const Vec3& a, const Vec3& b, const Vec3& c);

// 四面体计算结果。
struct TetrahedronResult {
    bool valid = false;
    std::string reason;
    double volume = 0.0;
    double surfaceArea = 0.0;
    Vec3 centroid;             // 重心
    Vec3 circumcenter;         // 外接球球心
    double circumradius = 0.0; // 外接球半径
    double inradius = 0.0;     // 内切球半径
    bool degenerate = false;
};

// 计算四面体的体积、表面积、重心与内外接球。
TetrahedronResult analyzeTetrahedron(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d);

// 平面多边形（凸）面积，用于计算棱柱与棱锥。
double polygonArea(const std::vector<Vec3>& vertices);

// 常见几何体的表面积与体积。
struct SolidResult {
    bool valid = false;
    std::string reason;
    double surfaceArea = 0.0;
    double volume = 0.0;
};

SolidResult analyzeSphere(double radius);
SolidResult analyzeCylinder(double radius, double height);
SolidResult analyzeCone(double radius, double height);
SolidResult analyzePrism(double radius, double height, int sides);   // 正棱柱
SolidResult analyzePyramid(double radius, double height, int sides);  // 正棱锥

}  // namespace calc
