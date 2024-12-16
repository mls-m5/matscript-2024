#pragma once

#include "token.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <variant>
#include <vector>

namespace vm {

struct Vector {};

struct Variable {
    Token name;
};

struct OtherValueContent {
    virtual ~OtherValueContent() = default;
};

struct OtherValue {
    decltype(typeid(void).hash_code()) type = typeid(void).hash_code();
    std::string_view tName = typeid(void).name();

    template <typename T>
    void set(std::shared_ptr<T> ptr) {
        type = typeid(T).hash_code();
        tName = typeid(T).name();
        _content = ptr;
    }

    const std::shared_ptr<OtherValueContent> content() {
        return _content;
    }

private:
    std::shared_ptr<OtherValueContent> _content;
};

using OtherPtr = std::shared_ptr<OtherValueContent>;

struct String {
    std::string value;
};

struct Int {
    int64_t value = 0;
};

struct Float {
    double value = 0.f;
};

struct Bool {
    bool value = false;
};

template <typename T>
concept BuiltinTypes = std::same_as<T, String> || std::same_as<T, Int> ||
                       std::same_as<T, Float> || std::same_as<T, Bool>;

template <typename T>
concept InheritsOther = std::is_base_of_v<OtherValueContent, T>;

struct Value {
    std::variant<String, Int, Float, Bool, OtherValue> value;

    Value() = default;

    template <BuiltinTypes T>
    Value(const T &value)
        : value{value} {}

    template <InheritsOther T>
    Value(std::shared_ptr<T> v) {
        auto o = OtherValue{};
        o.set(v);
        value = std::move(o);
    }

    Value(const Value &) = default;

    template <InheritsOther T>
    Value &operator=(std::shared_ptr<T> v) {
        auto o = OtherValue{};
        o.set(v);
        value = std::move(o);

        return *this;
    }

    template <InheritsOther T>
    T &as() {
        if (!std::holds_alternative<OtherValue>(value)) {
            throw std::runtime_error{"Cannot convert value to function"};
        }

        auto &o = std::get<OtherValue>(value);
        auto hash = typeid(T).hash_code();
        if (o.type != hash) {
            throw std::runtime_error{"Cannot convert value to function " +
                                     std::string{o.tName} + " to " +
                                     std::string{typeid(T).name()}};
        }

        return static_cast<T &>(*o.content());
    }

    template <BuiltinTypes T>
    T &as() {
        if (!std::holds_alternative<T>(value)) {
            throw std::runtime_error{"Cannot convert value to float"};
        }

        return std::get<T>(value);
    }

    template <InheritsOther T>
    bool is() {
        if (std::holds_alternative<OtherValue>(value)) {
            if (std::get<OtherValue>(value).type == typeid(T).hash_code()) {
                return true;
            }
        }

        return false;
    }

    template <BuiltinTypes T>
    bool is() {
        if (std::holds_alternative<T>(value)) {
            return true;
        }

        return false;
    }
};

// template <>
// inline Float &Value::as() {
//     if (!std::holds_alternative<Float>(value)) {
//         throw std::runtime_error{"Cannot convert value to float"};
//     }

//     return std::get<Float>(value);
// }

// template <>
// inline Int &Value::as() {
//     if (!std::holds_alternative<Int>(value)) {
//         throw std::runtime_error{"Cannot convert value to int"};
//     }

//     return std::get<Int>(value);
// }

// template <>
// inline String &Value::as() {
//     if (!std::holds_alternative<String>(value)) {
//         throw std::runtime_error{"Cannot convert value to string"};
//     }

//     return std::get<String>(value);
// }

struct Context {
    struct Map *closure = nullptr;

    Context *parent = nullptr;
};

struct Command {
    virtual ~Command() = default;
    virtual Value run(struct Context &context) = 0;
};

struct Section {
    std::vector<std::unique_ptr<Command>> commands;
};

struct Function : public OtherValueContent {
    using FunctionType = Value (*)(Context &);

    Function() = default;
    Function(std::vector<Token> args, FunctionType f)
        : argumentNames{std::move(args)}
        , native{f} {}

    std::vector<Token> argumentNames;

    Section body;

    FunctionType native = nullptr;
};

struct Map : public OtherValueContent {
    struct Declaration {
        Token name;
        Value value;
    };

    std::vector<Declaration> values;

    Value protoype;

    Value &operator[](const Token &name) {
        for (auto &it : values) {
            if (it.name == name.text) {
                return it.value;
            }
        }

        values.push_back({
            .name = name,
        });
        return values.back().value;
    }

    template <typename T>
    T &at(const Token &name) {
        for (auto &it : values) {
            if (it.name == name.text) {
                return it.value.as<T>();
            }
        }

        throw std::runtime_error{"could not find member " + name.text +
                                 " in map"};
    }

    Value &at(const Token &name) {
        for (auto &it : values) {
            if (it.name == name.text) {
                return it.value;
            }
        }

        throw std::runtime_error{"could not find member " + name.text +
                                 " in map"};
    }
};

Value call(const Section &section, Context &context);

Value call(const Function &f, std::vector<Value> values, Context &context);

const std::shared_ptr<Map> &getStd();

} // namespace vm
