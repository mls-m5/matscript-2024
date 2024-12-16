#include "settings.h"
#include "token.h"
#include "tokenizer.h"
#include "vm.h"
#include <filesystem>
#include <iostream>
#include <memory>

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

    for (Token token; (file.current(TokenType::Any).type != TokenType::Eof);) {
        // vlog(file.pop(TokenType::Any).text);
        std::cout << file.pop(TokenType::Any).text << std::endl;
    }

    std::cout << std::endl;

    auto module = std::make_shared<vm::Map>();

    (*module)[Token::from("std")] = vm::getStd();

    auto context = vm::Context{};

    auto f = (*module)[Token::from("std")].as<vm::Function>();

    call(*f, {1}, context);

    return 0;
}
