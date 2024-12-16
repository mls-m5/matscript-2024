#pragma once

#include "token.h"
#include <exception>
#include <filesystem>
#include <stdexcept>

struct ParserError : public std::exception {
    std::string stack;
    std::string description;
    std::string note;

    std::string combined;

    static std::string getContext(const Token &token);

    static auto createDescriptionString(const Token &token,
                                        std::string type = "error") {
        auto ss = std::ostringstream{};
        ss << (token.location.path
                   ? std::filesystem::relative(token.path(),
                                               std::filesystem::current_path())
                         .string()
                   : "unknown file")
           << ":" << token.line() << ":" << token.column() << ": " + type
           << ": \"" << token.text << "\"";

        return ss.str();
    }

    void combine() {
        combined = description + "\n" + stack + "\n" + note;
    }

    void addToStack(Token location) {
        stack += createDescriptionString(location) + " " +
                 getContext(location) + "\n";
        combine();
    }

    void addNote(std::string n) {
        note += "\n" + n;
        combine();
    }

    // override what
    const char *what() const noexcept override {
        return (combined).c_str();
    }

    ParserError(const Token &token, std::string description)
        : description{description}
        , stack{createDescriptionString(token) + " " + getContext(token)} {
        combine();
    }
};

inline void expect(const Token &token, std::string_view content) {
    if (token.text != content) {
        throw ParserError{token,
                          "expected " + std::string{content} + " but got " +
                              token.text};
    }
}

inline void expect(const Token &token, TokenType type) {
    if (type != TokenType::Any && token.type != type) {
        throw ParserError{token,
                          "expected type " +
                              std::string{tokenTypeToName(type)} + " but got " +
                              std::string{tokenTypeToName(token.type)} +
                              " (\"" + token.text + "\")"};
    }
}

template <typename... T>
inline void expect(const Token &token, TokenType type, const T &...types) {
    auto array = std::array{type, types...};
    bool wasFound = false;
    for (auto t : array) {
        if (token.type == t) {
            wasFound = true;
        }
    }
    if (!wasFound) {
        throw ParserError{token,
                          "expected type other type"
                          " but got " +
                              std::string{tokenTypeToName(token.type)} +
                              " (\"" + token.text + "\")"};
    }
}
