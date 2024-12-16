#include "token.h"
#include "log.h"
#include "tokenizer.h"
#include <algorithm>
#include <cctype>
#include <string_view>
#include <vector>

namespace {

std::string convertLlvmTypeName(std::string_view name) {
    if (name == "Text") {
        return "identifier";
    }

    auto ret = std::string{};

    ret += std::tolower(name.front());
    name.remove_prefix(1);

    for (auto c : name) {
        if (c > 'A' && c <= 'Z') {
            ret += '_';
            ret += std::tolower(c);
        }
        else {
            ret += c;
        }
    }

    return ret;
}

#define ITEM(x) {TokenType::x, convertLlvmTypeName(#x)},
#define KEYWORD(x) ITEM(x)
#define OP(x, y) {TokenType::x, convertLlvmTypeName(#x)},
#define BOP(x, y, z) {TokenType::x, convertLlvmTypeName(#x)},

auto llvmTypeNames = std::vector<std::pair<TokenType, std::string>>{TYPE_LIST};

#undef ITEM
#undef KEYWORD
#undef OP
#undef BOP

} // namespace
std::string_view tokenTypeToName(TokenType type) {
    return llvmTypeNames.at(static_cast<int>(type)).second;
}

TokenType tokenNameToType(std::string_view name, std::string_view text) {
    if (text == "fn") {
        return TokenType::Fn;
    }
    else if (text == "true") {
        return TokenType::True;
    }
    else if (text == "false") {
        return TokenType::False;
    }
    else if (text == "let") {
        return TokenType::Let;
    }

    auto it = std::find_if(llvmTypeNames.begin(),
                           llvmTypeNames.end(),
                           [name](auto n) { return n.second == name; });
    if (it == llvmTypeNames.end()) {
        vlog("unknown token type ", name, " with text ", text);
        return TokenType::Unknown;
    }
    return it->first;
}

Token Token::from(std::string_view text, Location location) {
    return Tokenizer::from(text, location);
}
