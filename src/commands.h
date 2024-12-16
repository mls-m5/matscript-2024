#pragma once

#include "vm.h"
#include <memory>

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

} // namespace vm
