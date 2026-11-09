// 输入解析：GUI 与 CLI 共用的校验逻辑，确保两边行为一致。
#pragma once

#include <optional>
#include <string>
#include <vector>

namespace calc {

// 解析结果：成功时携带值，失败时携带中文错误原因。
template <typename T>
struct ParseResult {
    bool ok = false;
    T value{};
    std::string error;

    static ParseResult success(T v) { return {true, v, {}}; }
    static ParseResult failure(std::string message) { return {false, T{}, std::move(message)}; }

    explicit operator bool() const { return ok; }
};

// 解析一个双精度数。拒绝空串、非数字、以及 1e400 这类溢出输入。
ParseResult<double> parseDouble(const std::string& text);

// 解析一个 64 位整数。拒绝小数与非数字。
ParseResult<long long> parseInteger(const std::string& text);

// 解析一组双精度数，逐项校验。
std::vector<ParseResult<double>> parseDoubleList(const std::vector<std::string>& texts);

// 去掉首尾空白。
std::string trim(const std::string& text);

}  // namespace calc
