#include "commands.h"
#include "parsererror.h"
#include "settings.h"
#include "token.h"
#include "tokeniterator.h"
#include "tokenizer.h"
#include "vm.h"
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <utility>

std::shared_ptr<vm::Expression> parseExpression(
    TokenIterator &it, std::function<bool(const Token &)> endCondition = {});

std::shared_ptr<vm::Expression> parseVariableDeclaration(TokenIterator &it) {
    auto name = it.pop(TokenType::Text);

    auto declaration = std::make_shared<vm::VariableDeclaration>();

    declaration->name = name;

    return declaration;
}

std::shared_ptr<vm::Expression> parseFor(TokenIterator &it) {
    it.pop(TokenType::For);
    it.pop(TokenType::LParen);

    auto exp = std::make_shared<vm::ForDeclaration>();

    exp->declaration =
        parseExpression(it, [](const Token &t) { return t.text == "in"; });
    expect(it.pop(TokenType::Text), "in");

    exp->range = parseExpression(it);

    it.pop(TokenType::RParen);

    return exp;
}

std::vector<std::shared_ptr<vm::Expression>> parseFunctionArguments(
    TokenIterator &it) {
    auto args = std::vector<std::shared_ptr<vm::Expression>>{};

    it.pop();
    for (; it.current().type != TokenType::RParen;) {
        args.push_back(parseExpression(it));

        if (it.current() == TokenType::RParen) {
            break;
        }
        if (it.current() != TokenType::Comma) {
            throw ParserError{it.current(), "Unexpected token"};
        }
        it.pop();
    }
    it.pop();

    return args;
}

std::shared_ptr<vm::Expression> parseExpression(
    TokenIterator &it, std::function<bool(const Token &)> endCondition) {
    auto exp = std::shared_ptr<vm::Expression>{};

    bool shouldBreak = false;

    auto assignSingleExpression = [&](decltype(exp) e) {
        if (exp) {
            throw ParserError{it.current(), "Unexpected token"};
        }
        exp = e;
    };

    for (; it.current().type != TokenType::Semi &&
           it.current().type != TokenType::Eof && !shouldBreak;) {

        if (endCondition) {
            if (endCondition(it.current())) {
                break;
            }
        }

        switch (it.current().type) {
        case TokenType::Let:
            it.consume();
            if (exp) {
                throw ParserError{it.current(),
                                  "Let must be at beginning of line"};
            }
            exp = parseVariableDeclaration(it);
            break;
        case TokenType::Equal: {
            it.consume();
            if (!exp) {
                throw ParserError{it.current(), "Line cannot start with '='"};
            }

            auto assignment = std::make_shared<vm::Assignment>();

            assignment->left = std::exchange(exp, assignment);

            assignment->right = parseExpression(it);

            break;
        }
        case TokenType::Text: {
            auto accessor = std::make_shared<vm::VariableAccessor>();

            accessor->name = it.pop(TokenType::Text);

            exp = std::move(accessor);

            break;
        }

        case TokenType::LParen: {
            if (!exp) {
                throw ParserError{it.current(), "Unexpected paren"};
            }

            auto call = std::make_shared<vm::FunctionCall>();

            call->functionValue = std::move(exp);

            call->arguments = parseFunctionArguments(it);

            exp = std::move(call);

            break;
        }

        case TokenType::StringLiteral:
            if (exp) {
                throw ParserError{it.current(), "Unexpected token"};
            }

            exp = std::make_shared<vm::StringLiteral>(it.pop());
            break;

        case TokenType::LSquare:
            it.pop();
            it.pop(TokenType::RSquare);

            if (exp) {
                // TODO: Handle array indexing
                throw ParserError{it.current(), "Unexpected token"};
            }

            exp = std::make_shared<vm::ArrayDeclaration>();

            break;
        case TokenType::For:
            assignSingleExpression(parseFor(it));

            break;
        case TokenType::Period: {
            if (!exp) {
                throw ParserError{it.current(), "stray '.'"};
            }

            auto memberFunction = std::make_shared<vm::MemberFunctionCall>();
            it.consume();

            memberFunction->object = std::exchange(exp, memberFunction);

            memberFunction->memberName = it.pop();

            // TODO: Implement regular member accessors
            it.current(TokenType::LParen);

            memberFunction->arguments = parseFunctionArguments(it);

            break;
        }
        default:
            shouldBreak = true;
            break;
        }
    }

    if (!exp) {
        throw ParserError{it.current(), "Unexpected token"};
    }

    return exp;
}

std::shared_ptr<vm::Map> parseRoot(TokenIterator &it) {
    auto map = std::make_shared<vm::Map>();

    auto mainFunction = std::make_shared<vm::Function>();

    for (; it.current() != TokenType::Eof;) {
        mainFunction->body.commands.push_back(parseExpression(it));

        if (it.current().type == TokenType::Semi) {
            it.pop(TokenType::Semi);
        }
    }

    (*map)[t("main")] = mainFunction;

    return map;
}

int main(int argc, char *argv[]) {
    const auto settings = Settings{argc, argv};

    auto file = [&] {
        if (settings.path.empty()) {
            return Tokenizer(std::cin, "stdin");
        }
        else {
            return Tokenizer{settings.path};
        }
    }();

    auto module = parseRoot(file);

    std::cout << std::endl;

    (*module)[Token::from("std")] = vm::getStd();

    auto context = vm::Context{};

    auto &f = module->at<vm::Map>(Token::from("std"))
                  .at<vm::Function>(Token::from("abs"));

    auto ret = call(f, {vm::Float{-1}}, context);

    auto &mainF = module->at<vm::Function>(Token::from("main"));

    call(mainF, {}, context);

    return 0;
}
