#include "Numeric.h"

#include <iomanip>
#include <sstream>

namespace calc {

std::string formatReal(double value, int precision) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    std::string text = out.str();
    // 消除 "-0.000000" 这类负零显示。
    if (!text.empty() && text.front() == '-' &&
        text.find_first_not_of("-0.", 0) == std::string::npos) {
        text.erase(text.begin());
    }
    return text;
}

std::string formatComplex(const Complex& z, int precision) {
    double re = z.real();
    double im = z.imag();
    if (nearlyEqual(im, 0.0, 1e-12)) {
        return formatReal(re, precision);
    }
    if (nearlyEqual(re, 0.0, 1e-12)) {
        return formatReal(im, precision) + "i";
    }
    std::string sign = (im < 0) ? " - " : " + ";
    return formatReal(re, precision) + sign + formatReal(std::fabs(im), precision) + "i";
}

}  // namespace calc
