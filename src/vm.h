#pragma once

#include "token.h"
#include <memory>
#include <ranges>
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

    template <typename T>
    void set(std::shared_ptr<T> ptr) {
        type = typeid(T).hash_code();
    }
    std::shared_ptr<OtherValueContent> content;
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

    Value &operator=(OtherPtr v) {
        // value = std::move(v);
        value = OtherValue{
            .content = v,
        };

        return *this;
    }
};

struct Context {
    struct Map *closure = nullptr;

    Context *parent = nullptr;
};

struct Command {
    virtual ~Command() = default;
    virtual void run(struct Context &context) = 0;
};

struct Section {
    std::vector<std::unique_ptr<Command>> commands;
};

struct Function : public OtherValueContent {

    std::vector<Token> argumentNames;

    Section body;

    using FunctionType = void (*)(Context &);
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
};

void call(const Section &section, Context &context);

void call(const Function &f, std::vector<Value> values, Context &context);

// struct Module {
//     Map values;
// };

const std::shared_ptr<Map> &getStd();

} // namespace vm
