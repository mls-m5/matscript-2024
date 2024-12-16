#pragma once

#include "matperf/profiler.h"
#include "parsererror.h"
#include "tokeniterator.h"

#include <filesystem>
#include <fstream>
#include <istream>
#include <string_view>
#include <vector>

struct Tokenizer : public TokenIterator {
    Tokenizer(std::istream &in, std::filesystem::path path)
        : _in{std::shared_ptr<std::istream>(&in, [](auto) {})}
        , _path{std::make_shared<std::filesystem::path>(path)} {}

    Tokenizer(std::filesystem::path path)
        : _in{std::make_unique<std::ifstream>(path)}
        , _path{std::make_shared<std::filesystem::path>(path)} {
        if (!_in->good()) {
            throw std::runtime_error{"could not open module " + path.string()};
        }
    }

    Token pop(TokenType expectedType) override {
        PROFILE_FUNCTION();
        while (_outBuffer.empty()) {
            readOneToken();
        }
        auto token = std::move(_outBuffer.front());
        consume();
        expect(token, expectedType);
        return token;
    }

    const Token &current(TokenType expectedType) override {
        PROFILE_FUNCTION();
        while (_outBuffer.empty()) {
            readOneToken();
        }
        expect(_outBuffer.front(), expectedType);
        return _outBuffer.front();
    }

    const Token &next(TokenType expectedType) override {
        PROFILE_FUNCTION();
        while (_outBuffer.size() < 2) {
            readOneToken();
        }
        auto &token = _outBuffer.at(1);
        expect(token, expectedType);
        return token;
    }

    void consume() override {
        PROFILE_FUNCTION();
        if (_outBuffer.empty()) {
            throw std::runtime_error{"cannot erase without token"};
        }
        _outBuffer.erase(_outBuffer.begin());
    }

    static Token from(std::string_view, Token::Location location);

private:
    std::shared_ptr<std::istream> _in;
    std::shared_ptr<std::filesystem::path> _path;

    std::string _inBuffer;
    size_t _index = 0;
    size_t _currentLine = 0;

    std::vector<Token> _outBuffer;

    void readOneToken();

    void readLine() {
        _index = 0;
        ++_currentLine;
        if (!std::getline(*_in, _inBuffer)) {
            _inBuffer.assign(2, '\0');
        }
    }

    char currentChar() {
        while (_index >= _inBuffer.size()) {
            readLine();
        }
        return _inBuffer.at(_index);
    }

    char nextChar() {
        while (2 + _index >= _inBuffer.size()) {
            readLine();
        }
        return _inBuffer.at(1);
    }

    void consumeChar() {
        ++_index;
    }
};
