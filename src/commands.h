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

    Value run(Context &context) {
        auto l = left->run(context);
        auto r = right->run(context);
        return l = r;
    }
};

struct VariableAccessor : public Command {
    Token name;

    Value run(Context &context) {
        return context.at(name);
    }
};

struct FunctionCall : public Command {
    std::shared_ptr<Command> functionValue;
    std::vector<std::shared_ptr<Command>> arguments;

    Value run(Context &context) {
        auto function = functionValue->run(context);

        auto args = std::vector<Value>{};
        args.resize(arguments.size());

        for (auto &a : arguments) {
            args.push_back(a->run(context));
        }

        return call(function.as<Function>(), args, context);
    }
};

} // namespace vm
