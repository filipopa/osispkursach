#ifndef PDP11_LEXER_HPP
#define PDP11_LEXER_HPP

#include <string>
#include <vector>
#include <unordered_map>

// Типы токенов
enum class TokenType {
    // Мнемоники команд
    MOV, CMP, ADD, SUB, JSR, RTS, HALT, CLR, COM, INC, DEC, NEG, JMP,

    // Регистры (R0-R7, SP, PC)
    REGISTER,

    // Числовые литералы (десятичные, восьмеричные, шестнадцатеричные)
    NUMBER,

    // Метки (labels)
    LABEL,

    // Директивы ассемблера
    DIRECTIVE_WORD,   // .WORD
    DIRECTIVE_BYTE,   // .BYTE
    DIRECTIVE_END,    // .END
    DIRECTIVE_EQU,    // .EQU
    DIRECTIVE_ASCII,  // .ASCII

    // Символы
    COMMA,        // ,
    LPAREN,       // (
    RPAREN,       // )
    HASH,         // #
    AT,           // @
    PLUS,         // +
    MINUS,        // -
    COLON,        // :

    // Служебные
    END_OF_FILE,  // Конец файла
    UNKNOWN       // Неизвестный токен
};

// Структура токена
struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
};

// Лексер
class Lexer {
public:
    explicit Lexer(const std::string &source);
    std::vector<Token> tokenize();

private:
    char peek() const;
    char advance();
    void skipWhitespace();
    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;

    Token parseNumber();
    Token parseIdentifierOrKeyword();
    Token parseLabel();
    Token parseDirective();

    std::string source;
    size_t position = 0;
    size_t line = 1;
    size_t column = 1;

    const std::unordered_map<std::string, TokenType> keywords = {
        {"MOV", TokenType::MOV}, {"CMP", TokenType::CMP}, {"ADD", TokenType::ADD},
        {"SUB", TokenType::SUB}, {"JSR", TokenType::JSR}, {"RTS", TokenType::RTS},
        {"HALT", TokenType::HALT}, {"CLR", TokenType::CLR}, {"COM", TokenType::COM},
        {"INC", TokenType::INC}, {"DEC", TokenType::DEC}, {"NEG", TokenType::NEG},
        {"JMP", TokenType::JMP}
    };

    const std::unordered_map<std::string, TokenType> directives = {
        {".WORD", TokenType::DIRECTIVE_WORD},
        {".BYTE", TokenType::DIRECTIVE_BYTE},
        {".END", TokenType::DIRECTIVE_END},
        {".EQU", TokenType::DIRECTIVE_EQU},
        {".ASCII", TokenType::DIRECTIVE_ASCII}
    };
};

#endif // PDP11_LEXER_HPP