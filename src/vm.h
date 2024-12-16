#pragma once

#include "token.h"
#include <ranges>
#include <variant>
#include <vector>

namespace vm {

struct Vector {};

struct Variable {
    Token name;
};

struct OtherValue {};

struct String {
    std::string value;
};

struct Int {
    int64_t value = 0;
};

struct Value {
    std::variant<String, Int, OtherValue> value;
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

struct Function : public OtherValue {
    std::vector<Token> argumentNames;

    Section body;

    using FunctionType = void (*)(Context &);
    FunctionType native = nullptr;
};

struct Map : public OtherValue {
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

void call(const Section &section, Context &context) {
    for (auto &command : section.commands) {
        command->run(context);
    }
}

void call(const Function &f, std::vector<Value> values, Context &context) {
    auto closure = Map{};

    auto newContext = Context{
        .closure = &closure,
        .parent = &context,
    };

    if (f.native) {
        f.native(newContext);
        return;
    }

    for (auto i : std::ranges::iota_view{
             0uz, std::min(values.size(), f.argumentNames.size())}) {
        closure[f.argumentNames.at(i)] = std::move(values.at(i));
    }

    call(f.body, newContext);
}

struct Module {
    Map values;

    void defineVariable(Token name) {}
};

} // namespace vm
