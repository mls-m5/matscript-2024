
#include "parsererror.h"
#include <fstream>
#include <string>

std::string ParserError::getContext(const Token &token) {
    if (!token.location.path) {
        return "";
    }
    auto linenum = 0;
    auto file = std::ifstream{*token.location.path};

    auto ret = std::string{};

    for (std::string line; std::getline(file, line); ++linenum) {
        if (linenum < token.line() && linenum > token.line() - 4) {
            ret += line + "\n";
        }
    }

    auto column = token.column();
    if (column > 1) {
        --column;
    }

    if (!ret.empty()) {
        ret += std::string(column, ' ') + "^~~~ Here\n";
    }

    return ret;
}
