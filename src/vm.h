#pragma once

#include "token.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
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

struct Value {
    std::variant<String, Int, Float, OtherValue> value;

    template <typename T>
    Value &operator=(std::shared_ptr<T> v) {
        auto o = OtherValue{};

        o.set(v);
        o.set<T>(v);
        value = std::move(o);

        return *this;
    }

    template <typename T>
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
};

template <>
inline Float &Value::as() {
    if (!std::holds_alternative<Float>(value)) {
        throw std::runtime_error{"Cannot convert value to float"};
    }

    return std::get<Float>(value);
}

template <>
inline Int &Value::as() {
    if (!std::holds_alternative<Int>(value)) {
        throw std::runtime_error{"Cannot convert value to int"};
    }

    return std::get<Int>(value);
}

template <>
inline String &Value::as() {
    if (!std::holds_alternative<String>(value)) {
        throw std::runtime_error{"Cannot convert value to string"};
    }

    return std::get<String>(value);
}

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
