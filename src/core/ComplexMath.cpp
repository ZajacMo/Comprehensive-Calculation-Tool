#include "ComplexMath.h"

#include <cmath>
#include <sstream>

namespace calc {

std::optional<Complex> complexPow(const Complex& base, int64 exponent) {
    if (exponent < 0) {
        if (std::abs(base) < 1e-300) return std::nullopt;
        const Complex inverse = Complex(1.0, 0.0) / base;
        return complexPow(inverse, -exponent);
    }
    // 指数为 0 时结果为 1（0^0 也按 1 处理）。
    Complex result(1.0, 0.0);
    Complex factor = base;
    int64 remaining = exponent;
    while (remaining > 0) {
        if (remaining & 1) result *= factor;
        factor *= factor;
        remaining >>= 1;
    }
    return result;
}

std::vector<Complex> complexRoots(const Complex& value, int n) {
    std::vector<Complex> roots;
    if (n <= 0) return roots;

    const double magnitude = std::abs(value);
    const double angle = std::arg(value);
    // 根的模为 |z|^(1/n)。
    const double rootMagnitude = std::pow(magnitude, 1.0 / n);

    roots.reserve(static_cast<size_t>(n));
    for (int k = 0; k < n; ++k) {
        const double rootAngle = (angle + 2.0 * kPi * k) / n;
        roots.push_back(Complex(rootMagnitude * std::cos(rootAngle),
                                rootMagnitude * std::sin(rootAngle)));
    }
    return roots;
}

std::optional<Complex> complexLog(const Complex& value) {
    if (std::abs(value) < 1e-300) return std::nullopt;
    return Complex(std::log(std::abs(value)), std::arg(value));
}

std::pair<double, double> complexPolar(const Complex& value) {
    return {std::abs(value), std::arg(value)};
}

Complex complexFromPolar(double magnitude, double angle) {
    return Complex(magnitude * std::cos(angle), magnitude * std::sin(angle));
}

std::string describeComplex(const Complex& value, int precision) {
    std::ostringstream out;
    out << formatComplex(value, precision);
    out << "  (模 " << formatReal(std::abs(value), precision)
        << ", 辐角 " << formatReal(std::arg(value), precision) << " rad)";
    return out.str();
}

}  // namespace calc
