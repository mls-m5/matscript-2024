
#include "tokenizer.h"
#include "token.h"
#include <unordered_map>

namespace {

std::string convertTypeName(std::string_view name) {
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

#define ITEM(x)
#define KEYWORD(x) {convertTypeName(#x), TokenType::x},
#define OP(x, y) {y, TokenType::x},
#define BOP(x, y, z) {y, TokenType::x},

auto tokenizerMap = std::unordered_map<std::string, TokenType>{TYPE_LIST};

#undef ITEM
#undef OP
#undef BOP
#undef KEYWORD

enum class CurrentType {
    Space,
    Alpha,
    Number,
    Quote,
    Operator,
};

auto TokenizerGetType = [](char c) {
    if (std::isalpha(c) || c == '_') {
        return CurrentType::Alpha;
    }
    if (std::isdigit(c)) {
        return CurrentType::Number;
    }
    if (std::isspace(c)) {
        return CurrentType::Space;
    }
    if (c == '"') {
        return CurrentType::Quote;
    }
    return CurrentType::Operator;
};

} // namespace

Token Tokenizer::from(std::string_view str, Token::Location location) {
    auto token = Token{.text = std::string{str}, .location = location};

    if (str.empty()) {
        return token;
    }

    auto firstType = TokenizerGetType(str.front());

    if (firstType == CurrentType::Quote) {
        token.type = TokenType::StringLiteral;
        return token;
    }
    if (auto f = tokenizerMap.find(token.text); f != tokenizerMap.end()) {
        token.type = f->second;
        return token;
    }
    if (firstType == CurrentType::Alpha) {
        token.type = TokenType::Text;
        return token;
    }
    if (firstType == CurrentType::Number) {
        token.type = TokenType::NumericConstant;
        return token;
    }

    if (auto f = tokenizerMap.find(std::string{str}); f != tokenizerMap.end()) {
        token.text = str;
        token.type = f->second;
    }

    return token;
}

void Tokenizer::readOneToken() {

    auto &token = _outBuffer.emplace_back();
    token.location.path = _path;
    token.location.line = _currentLine;
    token.location.column = _index + 1;

    auto currentC = currentChar();

    if (currentC == '\0') {
        token.type = TokenType::Eof;
        _outBuffer.emplace_back().type = TokenType::Eof;
        return;
    }

    auto type = TokenizerGetType(currentC);

    auto finalize = [&] {
        if (type == CurrentType::Quote) {
            token.type = TokenType::StringLiteral;
            return;
        }
        if (auto f = tokenizerMap.find(token.text); f != tokenizerMap.end()) {
            token.type = f->second;
            return;
        }
        if (type == CurrentType::Alpha) {
            token.type = TokenType::Text;
            return;
        }
        if (type == CurrentType::Number) {
            token.type = TokenType::NumericConstant;
            return;
        }

        // Operators

        auto copy = token; // Since `token` is a reference
        _outBuffer.pop_back();

        for (bool repeat = true; repeat;) {
            repeat = false;
            for (size_t i = copy.text.size(); i > 0; --i) {
                auto str = copy.text.substr(0, i);
                if (auto f = tokenizerMap.find(str); f != tokenizerMap.end()) {
                    auto &ntoken = _outBuffer.emplace_back(copy);
                    ntoken.text = str;
                    ntoken.type = f->second;
                    repeat = true;
                    copy.text = copy.text.substr(i);
                    copy.location.column += i;
                    break;
                }
            }
        }
    };

    for (char c; (c = currentChar()) != '\0'; consumeChar()) {
        auto nextType = TokenizerGetType(c);
        switch (type) {
        case CurrentType::Space:
            if (nextType != CurrentType::Space) {
                token.text += c;
                token.location.column = _index;
                type = nextType;
            }
            break;
        case CurrentType::Alpha:
            if (nextType == CurrentType::Alpha ||
                nextType == CurrentType::Number) {
                token.text += c;
            }
            else {
                finalize();
                return;
            }
            break;
        case CurrentType::Number:
            if (nextType == CurrentType::Number) {
                token.text += c;
            }
            else if (nextType == CurrentType::Operator && c == '.') {
                token.text += c;
            }
            else if (c == '\'') {
            }
            else {
                finalize();
                return;
            }
            break;
        case CurrentType::Quote:
            token.text += c;
            if (token.text.size() > 1 && nextType == CurrentType::Quote) {
                consumeChar();
                finalize();
                return;
            }
            break;
        case CurrentType::Operator:
            if (nextType == CurrentType::Operator) {
                token.text += c;

                if (token.text == "//") {
                    // Comments
                    // TODO: If needed handle for example cases like "+//"
                    // TODO: Fix comments at start of line
                    _inBuffer = {};
                    token.text = {};
                    type = CurrentType::Space;
                    continue;
                }
            }
            else {
                finalize();
                return;
            }
            break;
        }
    }

    if (token.text.empty()) {
        token.type = TokenType::Eof;
        return;
    }
    finalize();
}
