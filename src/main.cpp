#include "settings.h"
#include "token.h"
#include "tokenizer.h"
#include <filesystem>
#include <iostream>

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

    return 0;
}
