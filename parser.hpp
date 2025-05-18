#ifndef PDP11_PARSER_HPP
#define PDP11_PARSER_HPP

#include "lexer.hpp"
#include "ast.hpp"
#include <vector>
#include <memory>
#include <stdexcept>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    
    std::unique_ptr<Program> parseProgram();

private:
    // Вспомогательные методы
    const Token& currentToken() const;
    const Token& peekToken() const;
    void advance();
    bool match(TokenType type);
    void expect(TokenType type, const std::string& errorMsg);

    // Методы парсинга
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<Label> parseLabel();
    std::unique_ptr<Instruction> parseInstruction();
    std::unique_ptr<Directive> parseDirective();
    std::unique_ptr<Operand> parseOperand();
    AddrMode parseAddressingMode();

    std::vector<Token> tokens;
    size_t currentPos = 0;
};

#endif // PDP11_PARSER_HPP