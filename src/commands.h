#pragma once

#include "vm.h"
#include <memory>
#include <vector>

namespace vm {

struct VariableDeclaration : public Command {
    Token name;

    Value run(Context &context) override {
        return context.closure->define(name);
    }
};

struct Assignment : public Command {
    std::shared_ptr<Command> left;
    std::shared_ptr<Command> right;

    Value run(Context &context) override {
        auto l = left->run(context);
        auto r = right->run(context);
        return l = r;
    }
};

struct VariableAccessor : public Command {
    Token name;

    Value run(Context &context) override {
        return context.at(name);
    }
};

struct FunctionCall : public Command {
    std::shared_ptr<Command> functionValue;
    std::vector<std::shared_ptr<Command>> arguments;

    Value run(Context &context) override {
        auto function = functionValue->run(context);

        auto args = std::vector<Value>{};
        args.resize(arguments.size());

        for (auto &a : arguments) {
            args.push_back(a->run(context));
        }

        return call(function.as<Function>(), args, context);
    }
};

struct StringLiteral : public Command {
    StringLiteral(Token text)
        : text{text} {}

    Token text;

    Value run(Context &context) override {
        return String{text.text};
    }
};

struct ArrayDeclaration : public Command {
    Value run(Context &context) override {
        return std::make_shared<Array>();
    }
};

struct ForDeclaration : public Command {
    Section section;
    std::shared_ptr<Command> declaration;
    std::shared_ptr<Command> range;

    Value run(Context &context) override {
        auto closure = Map{};
        auto newContext = Context{
            .closure = &closure,
            .parent = &context,
        };

        auto ret = Value{};

        declaration->run(newContext);

        auto r = range->run(newContext);

        // TODO:  Call iterator

        throw "implement this";
        // for (Value value; !(value = r.as<>() r.is<Void>());) {
        //     ret = call(section, newContext);
        // }

        return ret;
    }
};

} // namespace vm
