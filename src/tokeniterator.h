#include "token.h"

#pragma once

class TokenIterator {
public:
    TokenIterator(const TokenIterator &) = delete;
    TokenIterator(TokenIterator &&) = delete;
    TokenIterator &operator=(const TokenIterator &) = delete;
    TokenIterator &operator=(TokenIterator &&) = delete;
    TokenIterator() = default;
    virtual ~TokenIterator() = default;

    virtual Token pop(TokenType expectedType = TokenType::Any) = 0;
    virtual const Token &current(TokenType expectedType = TokenType::Any) = 0;
    virtual const Token &next(TokenType expectedType = TokenType::Any) = 0;
    virtual void consume() = 0;
};
