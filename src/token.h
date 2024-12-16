#pragma once

#include <filesystem>
#include <memory>
#include <ostream>
#include <string>

struct Word {
    std::string text;

    std::string_view operator*() const {
        return text;
    }
    std::string_view operator->() const {
        return **this;
    }
};

/// Any is not a type but a selector that can be used to select any of the
/// others Unknown is when the tokenizer cannot tell what type it is
// clang-format off
#define TYPE_LIST \
KEYWORD(Fn)\
    KEYWORD(Pub)\
    KEYWORD(Impl)\
    KEYWORD(True)\
    KEYWORD(False)\
    KEYWORD(Let)\
    KEYWORD(Mut)\
    \
    ITEM(Text)\
    KEYWORD(Struct)\
    KEYWORD(Operator)\
    KEYWORD(StringLiteral)\
    KEYWORD(If)\
    KEYWORD(Else)\
    KEYWORD(Enum)\
    KEYWORD(Int)\
    KEYWORD(For)\
    KEYWORD(Float)\
    KEYWORD(Double)\
    KEYWORD(I8)\
    KEYWORD(I16)\
    KEYWORD(I32)\
    KEYWORD(I64)\
    KEYWORD(Bool)\
    KEYWORD(This)\
    KEYWORD(Const)\
    KEYWORD(Match)\
    BOP(Period, ".", 2)\
    BOP(Star, "*", 5)\
    BOP(Slash, "/", 5)\
    BOP(Percent, "%", 5)\
    BOP(Plus, "+", 6)\
    BOP(Minus, "-", 6)\
    BOP(Less, "<", 9)\
    BOP(LessEqual, "<=", 9)\
    BOP(Greater, ">", 9)\
    BOP(GreaterEqual, ">=", 9)\
    BOP(EqualEqual, "==", 10)\
    BOP(ExclaimEqual, "!=", 10)\
    BOP(Amp, "&", 11)\
    BOP(Equal, "=", 16)\
    BOP(LeftArrow, "<-", 16)\
    BOP(PlusEqual, "+=", 16)\
    BOP(MinusEqual, "-=", 16)\
    BOP(StarEqual, "*=", 16)\
    BOP(SlashEqual, "/=", 16)\
    BOP(PercentEqual, "%=", 16)\
    OP(Comma, ",")\
    OP(PlusPlus, "++")\
    OP(Colon, ":")\
    OP(Semi, ";")\
    OP(LParen, "(")\
    OP(RParen, ")")\
    OP(LBrace, "{")\
    OP(RBrace, "}")\
    OP(LSquare, "[")\
    OP(RSquare, "]")\
    OP(Arrow, "->")\
    OP(Exclaim, "!")\
    OP(At, "@")\
    ITEM(NumericConstant)\
    KEYWORD(Return)\
    ITEM(Any)\
    ITEM(Unknown)\
    ITEM(Eof) // clang-format on

#define KEYWORD(x) x,
#define ITEM(x) x,
#define OP(x, y) x,
#define BOP(x, y, z) x,

enum class TokenType { TYPE_LIST };

#undef ITEM
#undef KEYWORD
#undef OP
#undef BOP

struct Token {
    struct Location {
        int line = 0;
        int column = 0;
        std::shared_ptr<std::filesystem::path> path;
    };

    std::string text;
    TokenType type = TokenType::Unknown;

    Location location;

    auto path() const {
        return location.path ? *location.path : "";
    }

    auto column() const {
        return location.column;
    }

    auto line() const {
        return location.line;
    }

    bool operator==(TokenType type) const {
        return type == this->type;
    }

    bool operator!=(TokenType type) const {
        return type != this->type;
    }

    bool operator==(std::string_view value) const {
        return text == value;
    }

    operator bool() const {
        return type != TokenType::Eof;
    }

    static Token from(std::string_view text,
                      Location location = {0, 0, nullptr});

    static Token fromPath(std::filesystem::path path,
                          int line = 0,
                          int column = 0) {
        return Token{
            "",
            TokenType::Eof,
            {line, column, std::make_shared<std::filesystem::path>(path)}};
    }
};

inline Token t(std::string_view text,
               Token::Location location = {0, 0, nullptr}) {
    return Token::from(text);
}

inline std::ostream &operator<<(std::ostream &stream, const Token &token) {
    stream << token.text;
    return stream;
}

std::string_view tokenTypeToName(TokenType);
TokenType tokenNameToType(std::string_view, std::string_view text);

inline auto eofToken = Token{"", TokenType::Eof};
