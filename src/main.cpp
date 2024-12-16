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

    auto &f = module->at<vm::Map>(Token::from("std"))
                  .at<vm::Function>(Token::from("abs"));

    auto ret = call(f, {vm::Float{-1}}, context);

    // std::cout << "Returned value " << ret.as<vm::Float>().value << "\n";

    auto &f2 = module->at<vm::Map>(Token::from("std"))
                   .at<vm::Function>(Token::from("println"));

    call(f2,
         std::vector<vm::Value>{vm::Value{
             vm::String{"returned value:"},
         }},
         context);

    call(f2,
         std::vector<vm::Value>{vm::Value{
             ret,
         }},
         context);

    auto &f3 = module->at<vm::Map>(Token::from("std"))
                   .at<vm::Function>(Token::from("help"));

    call(f3,
         std::vector<vm::Value>{vm::Value{
             ret,
         }},
         context);

    call(f3,
         std::vector<vm::Value>{vm::Value{
             (*module)[t("std")],
         }},
         context);

    return 0;
}
