#include "test_framework.h"

#include "core/InputParsing.h"

using namespace calc;

TEST(InputParsing, Trim) {
    CHECK_STREQ(trim("  hello  "), "hello");
    CHECK_STREQ(trim(""), "");
    CHECK_STREQ(trim("   "), "");
    CHECK_STREQ(trim("\t\nx\n"), "x");
}

TEST(InputParsing, ParseValidDoubles) {
    auto result = parseDouble("3.14");
    CHECK(result.ok);
    CHECK_NEAR(result.value, 3.14, 1e-12);

    auto negative = parseDouble("-2.5");
    CHECK(negative.ok);
    CHECK_NEAR(negative.value, -2.5, 1e-12);

    auto withPlus = parseDouble("+7");
    CHECK(withPlus.ok);
    CHECK_NEAR(withPlus.value, 7.0, 1e-12);

    auto exponent = parseDouble("1.5e3");
    CHECK(exponent.ok);
    CHECK_NEAR(exponent.value, 1500.0, 1e-9);

    auto negativeExponent = parseDouble("2e-2");
    CHECK(negativeExponent.ok);
    CHECK_NEAR(negativeExponent.value, 0.02, 1e-12);

    auto surrounded = parseDouble("  42  ");
    CHECK(surrounded.ok);
    CHECK_NEAR(surrounded.value, 42.0, 1e-12);
}

TEST(InputParsing, ParseInvalidDoubles) {
    CHECK(parseDouble("").ok == false);
    CHECK(parseDouble("   ").ok == false);
    CHECK(parseDouble("abc").ok == false);
    CHECK(parseDouble("3.14.15").ok == false);
    CHECK(parseDouble("1,5").ok == false);
    CHECK(parseDouble("1e").ok == false);
    CHECK(parseDouble("1e+").ok == false);
    CHECK(parseDouble("--5").ok == false);
    CHECK(parseDouble("5-").ok == false);
    CHECK(parseDouble("1e999").ok == false);  // 溢出
}

TEST(InputParsing, ParseIntegers) {
    auto result = parseInteger("42");
    CHECK(result.ok);
    CHECK_EQ(result.value, 42LL);

    auto negative = parseInteger("-17");
    CHECK(negative.ok);
    CHECK_EQ(negative.value, -17LL);

    auto zero = parseInteger("0");
    CHECK(zero.ok);
    CHECK_EQ(zero.value, 0LL);

    CHECK(parseInteger("").ok == false);
    CHECK(parseInteger("3.5").ok == false);
    CHECK(parseInteger("abc").ok == false);
    CHECK(parseInteger("1e3").ok == false);
    CHECK(parseInteger("99999999999999999999").ok == false);
}

TEST(InputParsing, ParseDoubleList) {
    auto results = parseDoubleList({"1", "2.5", "bad", ""});
    CHECK_EQ(results.size(), size_t(4));
    CHECK(results[0].ok);
    CHECK(results[1].ok);
    CHECK(results[2].ok == false);
    CHECK(results[3].ok == false);
    CHECK(!results[2].error.empty());
}
