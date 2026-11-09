#include "test_framework.h"

#include "core/Geometry.h"
#include "core/Numeric.h"

using namespace calc;

TEST(Geometry, VectorBasics) {
    Vec3 a{1, 0, 0};
    Vec3 b{0, 1, 0};
    CHECK_NEAR(a.dot(b), 0.0, 1e-12);
    CHECK_NEAR(a.norm(), 1.0, 1e-12);

    Vec3 cross = a.cross(b);
    CHECK_NEAR(cross.x, 0.0, 1e-12);
    CHECK_NEAR(cross.y, 0.0, 1e-12);
    CHECK_NEAR(cross.z, 1.0, 1e-12);

    Vec3 sum = a + b;
    CHECK_NEAR(sum.x, 1.0, 1e-12);
    CHECK_NEAR(sum.y, 1.0, 1e-12);

    Vec3 scaled = a * 3.0;
    CHECK_NEAR(scaled.x, 3.0, 1e-12);
}

TEST(Geometry, VectorNormalize) {
    Vec3 v{3, 4, 0};
    Vec3 unit = v.normalized();
    CHECK_NEAR(unit.norm(), 1.0, 1e-12);
    CHECK_NEAR(unit.x, 0.6, 1e-12);

    Vec3 zero{0, 0, 0};
    Vec3 zeroUnit = zero.normalized();
    CHECK_NEAR(zeroUnit.norm(), 0.0, 1e-12);
}

TEST(Geometry, TriangleRightAngle) {
    // 直角三角形，面积应为 0.5 * 3 * 4 = 6，周长 12。
    auto result = analyzeTriangle({0, 0, 0}, {3, 0, 0}, {0, 4, 0});
    CHECK(result.valid);
    CHECK(result.degenerate == false);
    CHECK_NEAR(result.area, 6.0, 1e-12);
    CHECK_NEAR(result.perimeter, 12.0, 1e-12);
    CHECK_NEAR(result.sideA, 5.0, 1e-12);
    CHECK_NEAR(result.sideB, 4.0, 1e-12);
    CHECK_NEAR(result.sideC, 3.0, 1e-12);
}

TEST(Geometry, TriangleInSpace) {
    // 与平面三角形等价（第三坐标不影响面积）。
    auto flat = analyzeTriangle({0, 0, 0}, {3, 0, 0}, {0, 4, 0});
    auto inSpace = analyzeTriangle({0, 0, 5}, {3, 0, 5}, {0, 4, 5});
    CHECK_NEAR(flat.area, inSpace.area, 1e-12);
    CHECK_NEAR(inSpace.area, 6.0, 1e-12);
}

TEST(Geometry, TriangleEquilateral) {
    // 边长 2 的等边三角形，面积为 √3。
    auto result = analyzeTriangle({0, 0, 0}, {2, 0, 0}, {1, std::sqrt(3.0), 0});
    CHECK(result.valid);
    CHECK_NEAR(result.area, std::sqrt(3.0), 1e-9);
    CHECK_NEAR(result.perimeter, 6.0, 1e-9);
}

TEST(Geometry, TriangleDegenerate) {
    // 三点共线。
    auto collinear = analyzeTriangle({0, 0, 0}, {1, 0, 0}, {2, 0, 0});
    CHECK(collinear.valid);
    CHECK(collinear.degenerate);
    CHECK_NEAR(collinear.area, 0.0, 1e-12);

    // 三点重合。
    auto identical = analyzeTriangle({1, 1, 1}, {1, 1, 1}, {1, 1, 1});
    CHECK(identical.degenerate);
    CHECK_NEAR(identical.area, 0.0, 1e-12);
}

TEST(Geometry, TriangleAltitude) {
    // 3-4-5 直角三角形，以最长边 5 为底，高为 2*6/5 = 2.4。
    auto result = analyzeTriangle({0, 0, 0}, {3, 0, 0}, {0, 4, 0});
    CHECK_NEAR(result.altitude, 2.4, 1e-9);
}

TEST(Geometry, TetrahedronUnitCubeCorner) {
    // 单位四面体：顶点 (0,0,0) (1,0,0) (0,1,0) (0,0,1)
    // 体积 = 1/6。
    // 三个直角面面积各为 1/2，第四个面是边长 √2 的等边三角形，面积为 √3/2。
    // 表面积 = 3/2 + √3/2
    auto result = analyzeTetrahedron({0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1});
    CHECK(result.valid);
    CHECK(result.degenerate == false);
    CHECK_NEAR(result.volume, 1.0 / 6.0, 1e-12);
    CHECK_NEAR(result.surfaceArea, 1.5 + std::sqrt(3.0) / 2.0, 1e-9);

    CHECK_NEAR(result.centroid.x, 0.25, 1e-12);
    CHECK_NEAR(result.centroid.y, 0.25, 1e-12);
    CHECK_NEAR(result.centroid.z, 0.25, 1e-12);
}

TEST(Geometry, TetrahedronRegular) {
    // 正四面体边长 1：体积 = √2/12，表面积 = √3
    const double s = 1.0;
    const double h = std::sqrt(2.0 / 3.0) * s;
    auto result = analyzeTetrahedron({0, 0, 0}, {s, 0, 0}, {s / 2, std::sqrt(3.0) / 2 * s, 0},
                                      {s / 2, std::sqrt(3.0) / 6 * s, h});
    CHECK(result.valid);
    CHECK_NEAR(result.volume, std::sqrt(2.0) / 12.0, 1e-9);
    CHECK_NEAR(result.surfaceArea, std::sqrt(3.0), 1e-9);
    CHECK(result.circumradius > 0.0);
    CHECK(result.inradius > 0.0);
}

TEST(Geometry, TetrahedronDegenerate) {
    // 四点共面。
    auto result = analyzeTetrahedron({0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0});
    CHECK(result.valid);
    CHECK(result.degenerate);
    CHECK_NEAR(result.volume, 0.0, 1e-12);
}

TEST(Geometry, PolygonArea) {
    // 单位正方形，面积为 1。
    std::vector<Vec3> square = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}};
    CHECK_NEAR(polygonArea(square), 1.0, 1e-12);

    // 倾斜的平行四边形：边向量 (1,0,0) 与 (0,1,1)，面积 = |叉积| = √2。
    std::vector<Vec3> tilted = {{0, 0, 0}, {1, 0, 0}, {1, 1, 1}, {0, 1, 1}};
    CHECK_NEAR(polygonArea(tilted), std::sqrt(2.0), 1e-9);

    // 非平面四边形：Newell 法给出面积向量的模，面积为 √5。
    std::vector<Vec3> nonPlanar = {{0, 0, 1}, {1, 0, 2}, {1, 1, 3}, {0, 1, 4}};
    CHECK_NEAR(polygonArea(nonPlanar), std::sqrt(5.0), 1e-9);

    // 点数不足。
    CHECK_NEAR(polygonArea({{0, 0, 0}, {1, 0, 0}}), 0.0, 1e-12);
}

TEST(Geometry, Sphere) {
    auto result = analyzeSphere(2.0);
    CHECK(result.valid);
    CHECK_NEAR(result.surfaceArea, 4.0 * kPi * 4.0, 1e-9);
    CHECK_NEAR(result.volume, (4.0 / 3.0) * kPi * 8.0, 1e-9);

    auto invalid = analyzeSphere(-1.0);
    CHECK(invalid.valid == false);
}

TEST(Geometry, Cylinder) {
    auto result = analyzeCylinder(2.0, 5.0);
    CHECK(result.valid);
    CHECK_NEAR(result.volume, kPi * 4.0 * 5.0, 1e-9);
    CHECK_NEAR(result.surfaceArea, 2.0 * kPi * 4.0 + 2.0 * kPi * 2.0 * 5.0, 1e-9);

    CHECK(analyzeCylinder(0.0, 5.0).valid == false);
    CHECK(analyzeCylinder(2.0, -1.0).valid == false);
}

TEST(Geometry, Cone) {
    auto result = analyzeCone(3.0, 4.0);
    CHECK(result.valid);
    CHECK_NEAR(result.volume, (1.0 / 3.0) * kPi * 9.0 * 4.0, 1e-9);
    // 母线长 = √(3²+4²) = 5
    CHECK_NEAR(result.surfaceArea, kPi * 3.0 * (3.0 + 5.0), 1e-9);
}

TEST(Geometry, PrismAndPyramid) {
    // 正六棱柱：底面外接圆半径 1，高 2。
    // 正六边形边长 = 2·r·sin(30°) = 1，内切圆半径 = r·cos(30°) = √3/2。
    // 底面积 = (6/2)·1·(√3/2) = 3√3/2
    const double hexagonArea = 3.0 * std::sqrt(3.0) / 2.0;

    auto prism = analyzePrism(1.0, 2.0, 6);
    CHECK(prism.valid);
    CHECK_NEAR(prism.volume, hexagonArea * 2.0, 1e-9);
    // 表面积 = 2·底面积 + 6·边长·高
    CHECK_NEAR(prism.surfaceArea, 2.0 * hexagonArea + 6.0 * 1.0 * 2.0, 1e-9);

    auto pyramid = analyzePyramid(1.0, 2.0, 6);
    CHECK(pyramid.valid);
    CHECK_NEAR(pyramid.volume, (1.0 / 3.0) * hexagonArea * 2.0, 1e-9);

    // 四棱柱：底面是边长 2 的正方形（外接圆半径 √2），高 1，体积 = 4。
    auto squarePrism = analyzePrism(std::sqrt(2.0), 1.0, 4);
    CHECK(squarePrism.valid);
    CHECK_NEAR(squarePrism.volume, 4.0, 1e-9);

    // 边数不足 3。
    CHECK(analyzePrism(1.0, 2.0, 2).valid == false);
    CHECK(analyzePyramid(1.0, 2.0, 2).valid == false);
}
