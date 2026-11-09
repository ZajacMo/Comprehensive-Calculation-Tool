#include "ApiRoutes.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "core/Algebra.h"
#include "core/ComplexMath.h"
#include "core/Geometry.h"
#include "core/HttpServer.h"
#include "core/Json.h"
#include "core/LinearAlgebra.h"
#include "core/NumberTheory.h"
#include "core/Numeric.h"
#include "core/Sequences.h"
#include "core/Statistics.h"

namespace calc {
namespace http {
namespace {

using json::Array;
using json::Object;
using json::Value;

// 把 double 数组转成 JSON 数组。
Array toJsonArray(const std::vector<double>& values) {
    Array items;
    items.reserve(values.size());
    for (double v : values) {
        items.push_back(Value(v));
    }
    return items;
}

// 构造 {"ok": true, ...} 形式的成功响应。
Response ok(Object payload) {
    payload["ok"] = Value(true);
    return Response::json(Value(payload).dump());
}

// 构造 {"ok": false, "error": "..."} 形式的失败响应。
Response fail(const std::string& message, int status = 400) {
    Object payload;
    payload["ok"] = Value(false);
    payload["error"] = Value(message);
    return Response::json(Value(payload).dump(), status);
}

// 从查询串读取整数参数，缺失或非法时返回 fallback。
bool readInteger(const Request& request, const std::string& key, long long& out, std::string& error) {
    if (!request.queryParams.count(key)) {
        error = "缺少参数 " + key;
        return false;
    }
    try {
        size_t consumed = 0;
        const long long value = std::stoll(request.param(key), &consumed);
        if (consumed != request.param(key).size()) {
            error = "参数 " + key + " 不是合法整数";
            return false;
        }
        out = value;
        return true;
    } catch (...) {
        error = "参数 " + key + " 不是合法整数";
        return false;
    }
}

// 从查询串读取浮点参数。
bool readDouble(const Request& request, const std::string& key, double& out, std::string& error) {
    if (!request.queryParams.count(key)) {
        error = "缺少参数 " + key;
        return false;
    }
    try {
        size_t consumed = 0;
        const double value = std::stod(request.param(key), &consumed);
        if (consumed != request.param(key).size()) {
            error = "参数 " + key + " 不是合法数字";
            return false;
        }
        out = value;
        return true;
    } catch (...) {
        error = "参数 " + key + " 不是合法数字";
        return false;
    }
}

// 读取一组浮点参数（逗号分隔）。
bool readDoubleList(const Request& request, const std::string& key,
                    std::vector<double>& out, std::string& error) {
    if (!request.queryParams.count(key)) {
        error = "缺少参数 " + key;
        return false;
    }
    const std::string& raw = request.param(key);
    std::istringstream stream(raw);
    std::string token;
    while (std::getline(stream, token, ',')) {
        // 去掉首尾空白。
        const size_t start = token.find_first_not_of(" \t");
        const size_t end = token.find_last_not_of(" \t");
        if (start == std::string::npos) continue;
        const std::string cleaned = token.substr(start, end - start + 1);
        try {
            size_t consumed = 0;
            const double value = std::stod(cleaned, &consumed);
            if (consumed != cleaned.size()) {
                error = "参数 " + key + " 中含有非法数字：" + cleaned;
                return false;
            }
            out.push_back(value);
        } catch (...) {
            error = "参数 " + key + " 中含有非法数字：" + cleaned;
            return false;
        }
    }
    if (out.empty()) {
        error = "参数 " + key + " 不能为空";
        return false;
    }
    return true;
}

// ---------------------------------------------------------------- 路由实现

Response handleFactorize(const Request& request) {
    long long n = 0;
    std::string error;
    if (!readInteger(request, "n", n, error)) return fail(error);
    if (n < 2) return fail("请输入不小于 2 的整数。");

    const int64 value = static_cast<int64>(n);
    const Factorization factorization = factorize(value);

    Array factors;
    for (const auto& factor : factorization.factors) {
        Object item;
        item["prime"] = Value(factor.prime);
        item["exponent"] = Value(factor.exponent);
        factors.push_back(Value(item));
    }

    Object payload;
    payload["n"] = Value(value);
    payload["factorization"] = Value(factors);
    payload["factorizationText"] = Value(factorization.describe());
    payload["isPrime"] = Value(isPrime(static_cast<uint64>(value)));
    payload["eulerPhi"] = Value(eulerPhi(value));
    payload["mobius"] = Value(mobius(value));
    payload["divisorCount"] = Value(divisorCount(value));
    payload["divisorSum"] = Value(divisorSum(value));
    payload["base2"] = Value(toBase(value, 2).value_or(""));
    payload["base8"] = Value(toBase(value, 8).value_or(""));
    payload["base16"] = Value(toBase(value, 16).value_or(""));
    payload["reconstructs"] = Value(factorization.reconstruct() == value);
    return ok(payload);
}

Response handleIsPrime(const Request& request) {
    long long n = 0;
    std::string error;
    if (!readInteger(request, "n", n, error)) return fail(error);
    if (n < 2) return fail("请输入不小于 2 的整数。");

    Object payload;
    payload["n"] = Value(static_cast<int64>(n));
    payload["isPrime"] = Value(isPrime(static_cast<uint64>(n)));
    return ok(payload);
}

Response handleStats(const Request& request) {
    std::vector<double> values;
    std::string error;
    if (!readDoubleList(request, "values", values, error)) return fail(error);

    const DescriptiveStats stats = describe(values);
    Object payload;
    payload["count"] = Value(static_cast<int64>(stats.count));
    payload["mean"] = Value(stats.mean);
    payload["median"] = Value(stats.median);
    payload["variance"] = Value(stats.variance);
    payload["stdDev"] = Value(stats.stdDev);
    payload["min"] = Value(stats.min);
    payload["max"] = Value(stats.max);
    payload["range"] = Value(stats.range);
    payload["q1"] = Value(stats.q1);
    payload["q3"] = Value(stats.q3);
    payload["iqr"] = Value(stats.iqr);
    payload["sum"] = Value(stats.sum);
    payload["values"] = Value(toJsonArray(values));
    return ok(payload);
}

Response handleRegress(const Request& request) {
    std::vector<double> coordinates;
    std::string error;
    if (!readDoubleList(request, "points", coordinates, error)) return fail(error);
    if (coordinates.size() % 2 != 0) {
        return fail("数据点需要成对给出：x1,y1,x2,y2,...");
    }

    std::vector<Point> points;
    for (size_t i = 0; i + 1 < coordinates.size(); i += 2) {
        points.push_back({coordinates[i], coordinates[i + 1]});
    }

    const LinearRegression fit = fitLinearRegression(points);
    if (!fit.valid) {
        Object payload;
        payload["ok"] = Value(false);
        payload["error"] = Value(fit.reason);
        payload["count"] = Value(static_cast<int64>(points.size()));
        return Response::json(Value(payload).dump());
    }

    Object payload;
    payload["count"] = Value(static_cast<int64>(fit.sampleSize));
    payload["meanX"] = Value(fit.meanX);
    payload["meanY"] = Value(fit.meanY);
    payload["slope"] = Value(fit.slope);
    payload["intercept"] = Value(fit.intercept);
    payload["r"] = Value(fit.r);
    payload["rSquared"] = Value(fit.rSquared);
    payload["stdDevX"] = Value(fit.stdDevX);
    payload["stdDevY"] = Value(fit.stdDevY);
    payload["slopeStdError"] = Value(fit.slopeStdError);
    payload["interceptStdError"] = Value(fit.interceptStdError);

    // 若提供了 x，则一并给出预测值。
    if (request.queryParams.count("x")) {
        double x = 0.0;
        if (!readDouble(request, "x", x, error)) return fail(error);
        payload["predictX"] = Value(x);
        payload["predictY"] = Value(fit.predict(x));
    }
    return ok(payload);
}

Response handleLinear(const Request& request) {
    double a = 0.0, b = 0.0;
    std::string error;
    if (!readDouble(request, "a", a, error)) return fail(error);
    if (!readDouble(request, "b", b, error)) return fail(error);

    const EquationSolution solution = solveLinear(a, b);
    Object payload;
    payload["a"] = Value(a);
    payload["b"] = Value(b);
    payload["valid"] = Value(solution.valid);
    payload["reason"] = Value(solution.reason);
    payload["rootsText"] = Value(solution.describe());
    if (solution.valid) {
        Array roots;
        for (const auto& root : solution.roots) {
            Object item;
            item["real"] = Value(root.real());
            item["imag"] = Value(root.imag());
            roots.push_back(Value(item));
        }
        payload["roots"] = Value(roots);
    }
    return ok(payload);
}

Response handleQuadratic(const Request& request) {
    double a = 0.0, b = 0.0, c = 0.0;
    std::string error;
    if (!readDouble(request, "a", a, error)) return fail(error);
    if (!readDouble(request, "b", b, error)) return fail(error);
    if (!readDouble(request, "c", c, error)) return fail(error);

    const EquationSolution solution = solveQuadratic(a, b, c);
    Object payload;
    payload["a"] = Value(a);
    payload["b"] = Value(b);
    payload["c"] = Value(c);
    payload["valid"] = Value(solution.valid);
    payload["reason"] = Value(solution.reason);
    payload["discriminant"] = Value(solution.discriminant);
    payload["hasRealRoots"] = Value(solution.hasRealRoots);
    payload["rootsText"] = Value(solution.describe());
    if (solution.valid) {
        Array roots;
        for (const auto& root : solution.roots) {
            Object item;
            item["real"] = Value(root.real());
            item["imag"] = Value(root.imag());
            roots.push_back(Value(item));
        }
        payload["roots"] = Value(roots);
    }
    return ok(payload);
}

Response handleVertex(const Request& request) {
    double a = 0.0, h = 0.0, k = 0.0;
    std::string error;
    if (!readDouble(request, "a", a, error)) return fail(error);
    if (!readDouble(request, "h", h, error)) return fail(error);
    if (!readDouble(request, "k", k, error)) return fail(error);
    if (a == 0.0) return fail("a = 0 时不是二次方程。");

    const EquationSolution solution = solveQuadraticVertex(a, h, k);
    Object payload;
    payload["a"] = Value(a);
    payload["h"] = Value(h);
    payload["k"] = Value(k);
    payload["expandedB"] = Value(-2.0 * a * h);
    payload["expandedC"] = Value(a * h * h + k);
    payload["valid"] = Value(solution.valid);
    payload["discriminant"] = Value(solution.discriminant);
    payload["hasRealRoots"] = Value(solution.hasRealRoots);
    payload["rootsText"] = Value(solution.describe());
    Array roots;
    for (const auto& root : solution.roots) {
        Object item;
        item["real"] = Value(root.real());
        item["imag"] = Value(root.imag());
        roots.push_back(Value(item));
    }
    payload["roots"] = Value(roots);
    return ok(payload);
}

Response handlePolynomial(const Request& request) {
    std::vector<double> coefficients;
    std::string error;
    if (!readDoubleList(request, "coefficients", coefficients, error)) return fail(error);
    if (coefficients.size() < 2) return fail("至少需要两个系数。");

    const PolynomialRoots result = solvePolynomial(coefficients);
    Object payload;
    payload["valid"] = Value(result.valid);
    payload["reason"] = Value(result.reason);
    if (result.valid) {
        Array roots;
        for (const auto& root : result.roots) {
            Object item;
            item["real"] = Value(root.real());
            item["imag"] = Value(root.imag());
            item["modulus"] = Value(std::abs(root));
            roots.push_back(Value(item));
        }
        payload["roots"] = Value(roots);
        payload["count"] = Value(static_cast<int64>(result.roots.size()));
    }
    return ok(payload);
}

Response handleTriangle(const Request& request) {
    std::vector<double> coordinates;
    std::string error;
    if (!readDoubleList(request, "points", coordinates, error)) return fail(error);
    if (coordinates.size() != 9) return fail("需要 9 个坐标值。");

    const Vec3 a{coordinates[0], coordinates[1], coordinates[2]};
    const Vec3 b{coordinates[3], coordinates[4], coordinates[5]};
    const Vec3 c{coordinates[6], coordinates[7], coordinates[8]};
    const TriangleResult result = analyzeTriangle(a, b, c);

    Object payload;
    payload["valid"] = Value(result.valid);
    payload["degenerate"] = Value(result.degenerate);
    payload["reason"] = Value(result.reason);
    payload["area"] = Value(result.area);
    payload["perimeter"] = Value(result.perimeter);
    payload["sideA"] = Value(result.sideA);
    payload["sideB"] = Value(result.sideB);
    payload["sideC"] = Value(result.sideC);
    payload["altitude"] = Value(result.altitude);
    return ok(payload);
}

Response handleTetrahedron(const Request& request) {
    std::vector<double> coordinates;
    std::string error;
    if (!readDoubleList(request, "points", coordinates, error)) return fail(error);
    if (coordinates.size() != 12) return fail("需要 12 个坐标值。");

    const Vec3 a{coordinates[0], coordinates[1], coordinates[2]};
    const Vec3 b{coordinates[3], coordinates[4], coordinates[5]};
    const Vec3 c{coordinates[6], coordinates[7], coordinates[8]};
    const Vec3 d{coordinates[9], coordinates[10], coordinates[11]};
    const TetrahedronResult result = analyzeTetrahedron(a, b, c, d);

    Object payload;
    payload["valid"] = Value(result.valid);
    payload["degenerate"] = Value(result.degenerate);
    payload["reason"] = Value(result.reason);
    payload["volume"] = Value(result.volume);
    payload["surfaceArea"] = Value(result.surfaceArea);
    payload["centroid"] = Value(toJsonArray({result.centroid.x, result.centroid.y, result.centroid.z}));
    payload["circumcenter"] =
        Value(toJsonArray({result.circumcenter.x, result.circumcenter.y, result.circumcenter.z}));
    payload["circumradius"] = Value(result.circumradius);
    payload["inradius"] = Value(result.inradius);
    return ok(payload);
}

Response handleSolid(const Request& request) {
    const std::string kind = request.param("kind");
    double radius = 0.0, height = 0.0;
    long long sides = 0;
    std::string error;

    if (!readDouble(request, "radius", radius, error)) return fail(error);
    if (kind == "sphere") {
        const SolidResult solid = analyzeSphere(radius);
        if (!solid.valid) return fail(solid.reason);
        Object payload;
        payload["kind"] = Value(kind);
        payload["surfaceArea"] = Value(solid.surfaceArea);
        payload["volume"] = Value(solid.volume);
        return ok(payload);
    }

    if (!readDouble(request, "height", height, error)) return fail(error);
    if (kind == "cylinder" || kind == "cone") {
        const SolidResult solid = (kind == "cylinder")
                                      ? analyzeCylinder(radius, height)
                                      : analyzeCone(radius, height);
        if (!solid.valid) return fail(solid.reason);
        Object payload;
        payload["kind"] = Value(kind);
        payload["surfaceArea"] = Value(solid.surfaceArea);
        payload["volume"] = Value(solid.volume);
        return ok(payload);
    }

    if (kind == "prism" || kind == "pyramid") {
        if (!readInteger(request, "sides", sides, error)) return fail(error);
        const SolidResult solid = (kind == "prism")
                                      ? analyzePrism(radius, height, static_cast<int>(sides))
                                      : analyzePyramid(radius, height, static_cast<int>(sides));
        if (!solid.valid) return fail(solid.reason);
        Object payload;
        payload["kind"] = Value(kind);
        payload["surfaceArea"] = Value(solid.surfaceArea);
        payload["volume"] = Value(solid.volume);
        return ok(payload);
    }
    return fail("未知的几何体类型：" + kind);
}

Response handleMatrix(const Request& request) {
    std::vector<double> flat;
    std::string error;
    if (!readDoubleList(request, "values", flat, error)) return fail(error);

    long long rows = 0, cols = 0;
    if (!readInteger(request, "rows", rows, error)) return fail(error);
    if (!readInteger(request, "cols", cols, error)) return fail(error);
    if (rows <= 0 || cols <= 0) return fail("矩阵的行数与列数必须为正数。");
    if (static_cast<size_t>(rows * cols) != flat.size()) {
        return fail("矩阵元素个数与 rows×cols 不一致。");
    }

    Matrix matrix(static_cast<size_t>(rows), static_cast<size_t>(cols));
    for (size_t r = 0; r < static_cast<size_t>(rows); ++r) {
        for (size_t c = 0; c < static_cast<size_t>(cols); ++c) {
            matrix.set(r, c, flat[r * static_cast<size_t>(cols) + c]);
        }
    }

    Object payload;
    payload["rows"] = Value(rows);
    payload["cols"] = Value(cols);
    payload["determinant"] = Value(matrix.determinant());
    payload["rank"] = Value(static_cast<int64>(matrix.rank()));
    payload["isSymmetric"] = Value(matrix.isSymmetric());
    payload["transpose"] = Value(toJsonArray(
        [&matrix] {
            std::vector<double> values;
            const Matrix t = matrix.transposed();
            for (size_t r = 0; r < t.rows(); ++r) {
                for (size_t c = 0; c < t.cols(); ++c) {
                    values.push_back(t.at(r, c));
                }
            }
            return values;
        }()));

    const auto inverse = matrix.inverse();
    if (inverse.has_value()) {
        std::vector<double> values;
        for (size_t r = 0; r < inverse->rows(); ++r) {
            for (size_t c = 0; c < inverse->cols(); ++c) {
                values.push_back(inverse->at(r, c));
            }
        }
        payload["inverse"] = Value(toJsonArray(values));
        payload["invertible"] = Value(true);
    } else {
        payload["invertible"] = Value(false);
    }
    return ok(payload);
}

Response handleSolveSystem(const Request& request) {
    std::vector<double> coefficients;
    std::vector<double> rightSide;
    std::string error;
    if (!readDoubleList(request, "a", coefficients, error)) return fail(error);
    if (!readDoubleList(request, "b", rightSide, error)) return fail(error);

    const size_t n = rightSide.size();
    if (coefficients.size() != n * n) {
        return fail("系数矩阵的元素个数必须等于 n×n。");
    }

    Matrix matrix(n, n);
    for (size_t r = 0; r < n; ++r) {
        for (size_t c = 0; c < n; ++c) {
            matrix.set(r, c, coefficients[r * n + c]);
        }
    }

    const LinearSystemResult result = solveLinearSystemDetailed(matrix, rightSide);
    Object payload;
    payload["valid"] = Value(result.valid);
    payload["reason"] = Value(result.reason);
    payload["hasInfiniteSolutions"] = Value(result.hasInfiniteSolutions);
    if (result.valid) {
        payload["solution"] = Value(toJsonArray(result.solution));
        Array freeVariables;
        for (int index : result.freeVariables) {
            freeVariables.push_back(Value(index));
        }
        payload["freeVariables"] = Value(freeVariables);
    }
    return ok(payload);
}

Response handleSequences(const Request& request) {
    long long n = 0;
    std::string error;
    if (!readInteger(request, "n", n, error)) return fail(error);
    if (n < 0) return fail("n 不能为负数。");

    const uint64 index = static_cast<uint64>(n);
    Object payload;
    payload["n"] = Value(n);
    payload["fibonacci"] = Value(fibonacci(index).has_value()
                                     ? Value(*fibonacci(index))
                                     : Value());
    payload["lucas"] = Value(lucas(index).has_value() ? Value(*lucas(index)) : Value());
    payload["catalan"] = Value(catalan(index).has_value() ? Value(*catalan(index)) : Value());
    payload["factorial"] = Value(factorial(index).has_value() ? Value(*factorial(index)) : Value());
    payload["partition"] = Value(integerPartition(index).has_value()
                                     ? Value(*integerPartition(index))
                                     : Value());
    payload["overflow"] = Value(!fibonacci(index).has_value());
    return ok(payload);
}

Response handleCombinatorics(const Request& request) {
    long long n = 0, k = 0;
    std::string error;
    if (!readInteger(request, "n", n, error)) return fail(error);
    if (!readInteger(request, "k", k, error)) return fail(error);
    if (n < 0 || k < 0) return fail("n 与 k 不能为负数。");

    const uint64 un = static_cast<uint64>(n);
    const uint64 uk = static_cast<uint64>(k);
    const auto choose = binomial(un, uk);
    const auto permute = permutation(un, uk);

    Object payload;
    payload["n"] = Value(n);
    payload["k"] = Value(k);
    payload["binomial"] = Value(choose.has_value() ? Value(*choose) : Value());
    payload["permutation"] = Value(permute.has_value() ? Value(*permute) : Value());
    payload["valid"] = Value(choose.has_value());
    return ok(payload);
}

Response handleBaseConvert(const Request& request) {
    long long base = 0;
    std::string error;
    if (!readInteger(request, "base", base, error)) return fail(error);
    if (base < 2 || base > 36) return fail("进制必须在 2 到 36 之间。");

    const std::string text = request.param("value");
    if (text.empty()) return fail("缺少参数 value");

    const auto converted = fromBase(text, static_cast<int>(base));
    if (!converted.has_value()) {
        return fail("'" + text + "' 不是合法的 " + std::to_string(base) + " 进制整数。");
    }

    Object payload;
    payload["value"] = Value(*converted);
    payload["base"] = Value(base);
    payload["base2"] = Value(toBase(*converted, 2).value_or(""));
    payload["base8"] = Value(toBase(*converted, 8).value_or(""));
    payload["base10"] = Value(toBase(*converted, 10).value_or(""));
    payload["base16"] = Value(toBase(*converted, 16).value_or(""));
    payload["base36"] = Value(toBase(*converted, 36).value_or(""));
    return ok(payload);
}

Response handleModular(const Request& request) {
    long long a = 0, m = 0;
    std::string error;
    if (!readInteger(request, "a", a, error)) return fail(error);
    if (!readInteger(request, "m", m, error)) return fail(error);
    if (m <= 0) return fail("模数必须为正数。");

    Object payload;
    payload["a"] = Value(a);
    payload["m"] = Value(m);
    const auto inverse = modularInverse(a, m);
    payload["inverse"] = Value(inverse.has_value() ? Value(*inverse) : Value());
    payload["hasInverse"] = Value(inverse.has_value());

    const auto egcd = extendedGcd(a, m);
    payload["gcd"] = Value(egcd.gcd);
    payload["bezoutX"] = Value(egcd.x);
    payload["bezoutY"] = Value(egcd.y);
    return ok(payload);
}

Response handleComplex(const Request& request) {
    double real = 0.0, imag = 0.0;
    std::string error;
    if (!readDouble(request, "real", real, error)) return fail(error);
    if (!readDouble(request, "imag", imag, error)) return fail(error);

    const Complex value(real, imag);
    Object payload;
    payload["real"] = Value(value.real());
    payload["imag"] = Value(value.imag());
    payload["modulus"] = Value(std::abs(value));
    payload["argument"] = Value(std::arg(value));

    long long power = 0;
    if (request.queryParams.count("power")) {
        if (!readInteger(request, "power", power, error)) return fail(error);
        const auto result = complexPow(value, power);
        if (result.has_value()) {
            payload["power"] = Value(power);
            payload["powerReal"] = Value(result->real());
            payload["powerImag"] = Value(result->imag());
        }
    }

    if (request.queryParams.count("roots")) {
        if (!readInteger(request, "roots", power, error)) return fail(error);
        if (power < 1 || power > 64) return fail("根的个数必须在 1 到 64 之间。");
        Array roots;
        for (const auto& root : complexRoots(value, static_cast<int>(power))) {
            Object item;
            item["real"] = Value(root.real());
            item["imag"] = Value(root.imag());
            roots.push_back(Value(item));
        }
        payload["roots"] = Value(roots);
    }
    return ok(payload);
}

Response handleHealth(const Request&) {
    Object payload;
    payload["status"] = Value("ok");
    payload["service"] = Value("综合计算工具 API");
    payload["version"] = Value("2.1.0");
    return ok(payload);
}

}  // namespace

void registerApiRoutes(Server& server, const std::string& webRoot) {
    server.route("/api/health", handleHealth);
    server.route("/api/factorize", handleFactorize);
    server.route("/api/isprime", handleIsPrime);
    server.route("/api/stats", handleStats);
    server.route("/api/regress", handleRegress);
    server.route("/api/linear", handleLinear);
    server.route("/api/quadratic", handleQuadratic);
    server.route("/api/vertex", handleVertex);
    server.route("/api/polynomial", handlePolynomial);
    server.route("/api/triangle", handleTriangle);
    server.route("/api/tetrahedron", handleTetrahedron);
    server.route("/api/solid", handleSolid);
    server.route("/api/matrix", handleMatrix);
    server.route("/api/solve-system", handleSolveSystem);
    server.route("/api/sequences", handleSequences);
    server.route("/api/combinatorics", handleCombinatorics);
    server.route("/api/base-convert", handleBaseConvert);
    server.route("/api/modular", handleModular);
    server.route("/api/complex", handleComplex);

    if (!webRoot.empty()) {
        server.serveStatic("/", webRoot);
    }
}

}  // namespace http
}  // namespace calc
