#include "lexer.hpp"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string &source) : source(source) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (position < source.size()) {
        char current = peek();

        if (isspace(current)) {
            skipWhitespace();
            continue;
        }

        if (current == ';') {
            // Пропускаем комментарии (до конца строки)
            while (peek() != '\n' && position < source.size()) {
                advance();
            }
            continue;
        }

        if (isDigit(current)) {
            tokens.push_back(parseNumber());
            continue;
        }

        if (isAlpha(current) || current == '.') {
            tokens.push_back(parseIdentifierOrKeyword());
            continue;
        }

        if (current == ':') {
            tokens.push_back({TokenType::COLON, ":", line, column});
            advance();
            continue;
        }

        if (current == ',') {
            tokens.push_back({TokenType::COMMA, ",", line, column});
            advance();
            continue;
        }

        if (current == '(') {
            tokens.push_back({TokenType::LPAREN, "(", line, column});
            advance();
            continue;
        }

        if (current == ')') {
            tokens.push_back({TokenType::RPAREN, ")", line, column});
            advance();
            continue;
        }

        if (current == '#') {
            tokens.push_back({TokenType::HASH, "#", line, column});
            advance();
            continue;
        }

        if (current == '@') {
            tokens.push_back({TokenType::AT, "@", line, column});
            advance();
            continue;
        }

        if (current == '+') {
            tokens.push_back({TokenType::PLUS, "+", line, column});
            advance();
            continue;
        }

        if (current == '-') {
            tokens.push_back({TokenType::MINUS, "-", line, column});
            advance();
            continue;
        }

        // Неизвестный символ
        tokens.push_back({TokenType::UNKNOWN, std::string(1, current), line, column});
        advance();
    }

    tokens.push_back({TokenType::END_OF_FILE, "", line, column});
    return tokens;
}

char Lexer::peek() const {
    return position < source.size() ? source[position] : '\0';
}

char Lexer::advance() {
    if (position >= source.size()) return '\0';

    char current = source[position++];
    if (current == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return current;
}

void Lexer::skipWhitespace() {
    while (isspace(peek())) {
        advance();
    }
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

Token Lexer::parseNumber() {
    size_t start = position;
    size_t startLine = line;
    size_t startColumn = column;

    // Обработка шестнадцатеричных (0x...), восьмеричных (0o...) и десятичных чисел
    if (peek() == '0') {
        advance();
        if (peek() == 'x' || peek() == 'X') {
            advance(); // Пропускаем 'x' или 'X'
            while (isxdigit(peek())) advance();
        } else if (peek() == 'o' || peek() == 'O') {
            advance(); // Пропускаем 'o' или 'O'
            while (peek() >= '0' && peek() <= '7') advance();
        } else {
            while (isDigit(peek())) advance();
        }
    } else {
        while (isDigit(peek())) advance();
    }

    std::string value = source.substr(start, position - start);
    return {TokenType::NUMBER, value, startLine, startColumn};
}

Token Lexer::parseIdentifierOrKeyword() {
    size_t start = position;
    size_t startLine = line;
    size_t startColumn = column;

    while (isAlphaNumeric(peek()) || peek() == '.') {
        advance();
    }

    std::string value = source.substr(start, position - start);

    // Проверяем, является ли ключевым словом (командой)
    if (keywords.find(value) != keywords.end()) {
        return {keywords.at(value), value, startLine, startColumn};
    }

    // Проверяем, является ли директивой
    if (directives.find(value) != directives.end()) {
        return {directives.at(value), value, startLine, startColumn};
    }

    // Проверяем, является ли регистром (R0-R7, SP, PC)
    if ((value.size() == 2 || value.size() == 3) && (value[0] == 'R' || value == "SP" || value == "PC")) {
        if (value[0] == 'R') {
            if (value.size() == 2 && value[1] >= '0' && value[1] <= '7') {
                return {TokenType::REGISTER, value, startLine, startColumn};
            }
        } else if (value == "SP" || value == "PC") {
            return {TokenType::REGISTER, value, startLine, startColumn};
        }
    }

    // В противном случае — это метка или неизвестный идентификатор
    return {TokenType::LABEL, value, startLine, startColumn};
}