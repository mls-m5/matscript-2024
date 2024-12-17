#pragma once

#include "vm.h"
#include <memory>
#include <vector>

namespace vm {

struct VariableDeclaration : public Expression {
    Token name;

    Value run(Context &context) override {
        return context.closure->define(name);
    }
};

struct Assignment : public Expression {
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;

    Value run(Context &context) override {
        auto l = left->run(context);
        auto r = right->run(context);
        return l = r;
    }
};

struct VariableAccessor : public Expression {
    Token name;

    Value run(Context &context) override {
        return context.at(name);
    }
};

struct FunctionCall : public Expression {
    std::shared_ptr<Expression> functionValue;
    std::vector<std::shared_ptr<Expression>> arguments;

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

struct StringLiteral : public Expression {
    StringLiteral(Token text)
        : text{text} {}

    Token text;

    Value run(Context &context) override {
        return String{text.text};
    }
};

struct ArrayDeclaration : public Expression {
    Value run(Context &context) override {
        return std::make_shared<Array>();
    }
};

struct ForDeclaration : public Expression {
    Section section;
    std::shared_ptr<Expression> declaration;
    std::shared_ptr<Expression> range;

    Value run(Context &context) override {
        auto closure = Map{};
        auto newContext = Context{
            .closure = &closure,
            .parent = &context,
        };

        auto ret = Value{};

        declaration->run(newContext);

        auto r = range->run(newContext);

        auto next = r.as<Map>()[t("next")].as<Function>();

        for (Value value; !(value = call(next, {r}, newContext)).asBool();) {
            ret = call(section, newContext);
        }

        return ret;
    }
};

// struct MemberAccessor : public Command {
//     std::shared_ptr<Command> object;
//     std::shared_ptr<Command> member;
// };

struct MemberFunctionCall : public Expression {
    std::shared_ptr<Expression> object;
    Token memberName;
    std::vector<std::shared_ptr<Expression>> arguments;

    Value run(Context &context) override {
        auto o = object->run(context);
        auto function = o.as<Map>()[memberName].as<Function>();

        auto args = std::vector<Value>{};
        args.resize(arguments.size() + 1);

        for (auto &a : arguments) {
            args.push_back(a->run(context));
        }

        return call(function, args, context, o);
    }
};

} // namespace vm
