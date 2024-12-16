#include <filesystem>
#include <iostream>
#include <istream>
#include <ranges>
#include <string>

#include "log.h"
#include "token.h"
#include "tokenizer.h"

struct Settings {
    std::filesystem::path path;

    Settings(int argc, char *argv[]) {
        auto args = std::vector<std::string>{argv + 1, argv + argc};

        for (auto i : std::ranges::iota_view{0uz, args.size()}) {
            auto arg = args.at(i);

            path = arg;
        }
    }
};

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
