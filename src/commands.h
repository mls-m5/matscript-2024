#pragma once

#include "vm.h"

namespace vm {

struct VariableDeclaration : public Command {
    Token name;

    Value run(Context &context) override {
        return context.closure->define(name);
    }
};

} // namespace vm
