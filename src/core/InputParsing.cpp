#include "InputParsing.h"

#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace calc {

std::string trim(const std::string& text) {
    size_t begin = 0;
    size_t end = text.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(text[begin]))) ++begin;
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(begin, end - begin);
}

ParseResult<double> parseDouble(const std::string& text) {
    const std::string cleaned = trim(text);
    if (cleaned.empty()) {
        return ParseResult<double>::failure("输入为空。");
    }

    // std::from_chars 对 double 在部分标准库上支持有限，先做字符级校验。
    size_t index = 0;
    if (cleaned[index] == '+' || cleaned[index] == '-') ++index;

    bool sawDigit = false;
    bool sawDot = false;
    bool sawExponent = false;
    for (; index < cleaned.size(); ++index) {
        const char c = cleaned[index];
        if (std::isdigit(static_cast<unsigned char>(c))) {
            sawDigit = true;
        } else if (c == '.' && !sawDot && !sawExponent) {
            sawDot = true;
        } else if ((c == 'e' || c == 'E') && sawDigit && !sawExponent) {
            sawExponent = true;
            if (index + 1 < cleaned.size() &&
                (cleaned[index + 1] == '+' || cleaned[index + 1] == '-')) {
                ++index;
            }
            // 指数部分必须至少有一个数字。
            if (index + 1 >= cleaned.size() ||
                !std::isdigit(static_cast<unsigned char>(cleaned[index + 1]))) {
                return ParseResult<double>::failure("指数部分缺少数字。");
            }
        } else {
            return ParseResult<double>::failure("'" + cleaned + "' 不是合法的数字。");
        }
    }
    if (!sawDigit) {
        return ParseResult<double>::failure("'" + cleaned + "' 不是合法的数字。");
    }

    errno = 0;
    char* end = nullptr;
    const double value = std::strtod(cleaned.c_str(), &end);
    if (end != cleaned.c_str() + cleaned.size()) {
        return ParseResult<double>::failure("'" + cleaned + "' 不是合法的数字。");
    }
    if (errno == ERANGE || !std::isfinite(value)) {
        return ParseResult<double>::failure("'" + cleaned + "' 超出可表示范围。");
    }
    return ParseResult<double>::success(value);
}

ParseResult<long long> parseInteger(const std::string& text) {
    const std::string cleaned = trim(text);
    if (cleaned.empty()) {
        return ParseResult<long long>::failure("输入为空。");
    }

    size_t index = 0;
    if (cleaned[index] == '+' || cleaned[index] == '-') ++index;
    if (index >= cleaned.size()) {
        return ParseResult<long long>::failure("'" + cleaned + "' 不是合法的整数。");
    }
    for (; index < cleaned.size(); ++index) {
        if (!std::isdigit(static_cast<unsigned char>(cleaned[index]))) {
            return ParseResult<long long>::failure("'" + cleaned + "' 不是合法的整数。");
        }
    }

    errno = 0;
    char* end = nullptr;
    const long long value = std::strtoll(cleaned.c_str(), &end, 10);
    if (errno == ERANGE || end != cleaned.c_str() + cleaned.size()) {
        return ParseResult<long long>::failure("'" + cleaned + "' 超出整数范围。");
    }
    return ParseResult<long long>::success(value);
}

std::vector<ParseResult<double>> parseDoubleList(const std::vector<std::string>& texts) {
    std::vector<ParseResult<double>> results;
    results.reserve(texts.size());
    for (const auto& text : texts) {
        results.push_back(parseDouble(text));
    }
    return results;
}

}  // namespace calc
