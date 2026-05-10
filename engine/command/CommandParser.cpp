#include "CommandParser.h"

#include <cctype>

namespace AIForge {

int ParsedCommand::FindKeyword(const std::string& keyword) const {
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == keyword) return static_cast<int>(i);
    }
    return -1;
}

std::string CommandParser::ToLower(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

ParsedCommand CommandParser::Parse(const std::string& input) {
    ParsedCommand out;
    out.raw = input;

    std::string current;
    bool inQuote = false;
    auto flush = [&] {
        if (!current.empty()) {
            out.tokens.push_back(current);
            current.clear();
        }
    };

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        if (inQuote) {
            if (c == '"') {
                inQuote = false;
                flush();
            } else {
                current.push_back(c);
            }
        } else {
            if (c == '"') {
                inQuote = true;
            } else if (std::isspace(static_cast<unsigned char>(c))) {
                flush();
            } else {
                current.push_back(c);
            }
        }
    }
    flush();

    if (!out.tokens.empty()) {
        out.verb = ToLower(out.tokens.front());
        out.tokens.erase(out.tokens.begin());
    }
    return out;
}

}  // namespace AIForge
