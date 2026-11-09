#include "test_framework.h"

#include "core/Json.h"

namespace json = calc::json;

using json::Array;
using json::Object;
using json::Value;

TEST(Json, ParseScalars) {
    Value value;
    std::string error;

    CHECK(json::parse("null", value, error));
    CHECK(value.isNull());

    CHECK(json::parse("true", value, error));
    CHECK(value.isBool());
    CHECK_EQ(value.asBool(), true);

    CHECK(json::parse("false", value, error));
    CHECK_EQ(value.asBool(), false);

    CHECK(json::parse("42", value, error));
    CHECK(value.isNumber());
    CHECK_NEAR(value.asNumber(), 42.0, 1e-12);

    CHECK(json::parse("-3.5", value, error));
    CHECK_NEAR(value.asNumber(), -3.5, 1e-12);

    CHECK(json::parse("1.5e3", value, error));
    CHECK_NEAR(value.asNumber(), 1500.0, 1e-9);
}

TEST(Json, ParseString) {
    Value value;
    std::string error;
    CHECK(json::parse("\"hello\"", value, error));
    CHECK(value.isString());
    CHECK_STREQ(value.asString(), "hello");

    // 转义字符。
    CHECK(json::parse("\"a\\nb\"", value, error));
    CHECK_STREQ(value.asString(), "a\nb");

    CHECK(json::parse("\"\\u4e2d\\u6587\"", value, error));
    CHECK_STREQ(value.asString(), "中文");
}

TEST(Json, ParseArray) {
    Value value;
    std::string error;
    CHECK(json::parse("[1, 2, 3]", value, error));
    CHECK(value.isArray());
    CHECK_EQ(value.asArray().size(), size_t(3));
    CHECK_NEAR(value.at(1)->asNumber(), 2.0, 1e-12);

    // 空数组。
    CHECK(json::parse("[]", value, error));
    CHECK(value.isArray());
    CHECK_EQ(value.asArray().size(), size_t(0));
}

TEST(Json, ParseObject) {
    Value value;
    std::string error;
    CHECK(json::parse("{\"a\": 1, \"b\": \"two\", \"c\": [3]}", value, error));
    CHECK(value.isObject());
    CHECK_NEAR(value.find("a")->asNumber(), 1.0, 1e-12);
    CHECK_STREQ(value.find("b")->asString(), "two");
    CHECK_EQ(value.find("c")->asArray().size(), size_t(1));

    // 不存在的键返回 nullptr。
    CHECK(value.find("missing") == nullptr);
}

TEST(Json, ParseInvalid) {
    Value value;
    std::string error;
    CHECK(json::parse("", value, error) == false);
    CHECK(!error.empty());
    CHECK(json::parse("{", value, error) == false);
    CHECK(json::parse("[1, 2", value, error) == false);
    CHECK(json::parse("{\"a\" 1}", value, error) == false);
    CHECK(json::parse("tru", value, error) == false);
    CHECK(json::parse("1 2", value, error) == false);
}

TEST(Json, DumpRoundTrip) {
    Object object;
    object["name"] = Value("测试");
    object["count"] = Value(3);
    object["ratio"] = Value(0.5);
    object["flag"] = Value(true);
    object["empty"] = Value();

    Array items;
    items.push_back(Value(1));
    items.push_back(Value("two"));
    object["items"] = Value(items);

    const Value value(object);
    const std::string serialized = value.dump();
    CHECK(serialized.find("\"name\"") != std::string::npos);
    CHECK(serialized.find("测试") != std::string::npos);

    // 解析回来应完全一致。
    Value reparsed;
    std::string error;
    CHECK(json::parse(serialized, reparsed, error));
    CHECK_STREQ(reparsed.find("name")->asString(), "测试");
    CHECK_NEAR(reparsed.find("count")->asNumber(), 3.0, 1e-12);
    CHECK_NEAR(reparsed.find("ratio")->asNumber(), 0.5, 1e-12);
    CHECK_EQ(reparsed.find("flag")->asBool(), true);
    CHECK(reparsed.find("empty")->isNull());
    CHECK_EQ(reparsed.find("items")->asArray().size(), size_t(2));
}

TEST(Json, Escape) {
    CHECK_STREQ(json::escape("plain"), "plain");
    CHECK_STREQ(json::escape("a\"b"), "a\\\"b");
    CHECK_STREQ(json::escape("a\\b"), "a\\\\b");
    CHECK_STREQ(json::escape("a\nb"), "a\\nb");
    CHECK_STREQ(json::escape("a\tb"), "a\\tb");
    // 控制字符转成 \uXXXX。
    CHECK_STREQ(json::escape(std::string("a\x01") + "b"), "a\\u0001b");
}

TEST(Json, NumberFormatting) {
    Object object;
    object["int"] = Value(42);
    object["double"] = Value(1.5);
    const std::string serialized = Value(object).dump();
    CHECK(serialized.find("\"int\":42") != std::string::npos);
    CHECK(serialized.find("\"double\":1.5") != std::string::npos);
}

TEST(Json, IndentedDump) {
    Object object;
    object["a"] = Value(1);
    const std::string serialized = Value(object).dump(2);
    CHECK(serialized.find('\n') != std::string::npos);
    // 缩进输出应能再次解析。
    Value reparsed;
    std::string error;
    CHECK(json::parse(serialized, reparsed, error));
    CHECK_NEAR(reparsed.find("a")->asNumber(), 1.0, 1e-12);
}
