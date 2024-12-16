#include <filesystem>
#include <iostream>
#include <istream>

#include "token.h"
#include "tokenizer.h"

int main(int argc, char *argv[]) {
    std::istream &stream = std::cin;

    auto file = Tokenizer{stream, "stdin"};

    for (Token token; (file.current(TokenType::Any).type != TokenType::Eof);) {
        std::cout << token << "\n";
    }

    return 0;
}
