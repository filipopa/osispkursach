#include "parser.hpp"
#include <unordered_map>
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = ASTBuilder::createProgram();
    printf("Size: %d \n",tokens.size());
    while (currentPos < tokens.size() && !match(TokenType::END_OF_FILE)) {
        try {
            printf("\nCurrent pos: %d %c %c",currentPos,currentToken().value,peekToken().value);
            if (auto stmt = parseStatement()) {
                program->statements.push_back(std::move(stmt));
            }
        } catch (const std::runtime_error& e) {
            // Пропускаем ошибочные statement и пытаемся продолжить
            while (currentPos < tokens.size() && 
                  !match(TokenType::END_OF_FILE) &&
                  currentToken().type != TokenType::LABEL) {
                advance();
            }
        }
    }
    
    return program;
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    
    // Обработка меток

    if (match(TokenType::LABEL) && peekToken().type == TokenType::COLON) {
        return parseLabel();
    }
    // Обработка инструкций
    static const std::unordered_map<TokenType, Instruction::Type> instrMap = {
        {TokenType::MOV, Instruction::Type::MOV},
        {TokenType::CMP, Instruction::Type::CMP},
        {TokenType::ADD, Instruction::Type::ADD},
        {TokenType::SUB, Instruction::Type::SUB},
        {TokenType::JSR, Instruction::Type::JSR},
        {TokenType::RTS, Instruction::Type::RTS},
        {TokenType::HALT, Instruction::Type::HALT},
        {TokenType::CLR, Instruction::Type::CLR},
        {TokenType::COM, Instruction::Type::COM},
        {TokenType::INC, Instruction::Type::INC},
        {TokenType::DEC, Instruction::Type::DEC},
        {TokenType::NEG, Instruction::Type::NEG},
        {TokenType::JMP, Instruction::Type::JMP}
    };
    
    if (instrMap.count(currentToken().type)) {
        return parseInstruction();
    }
    
    // Обработка директив
    static const std::unordered_map<TokenType, Directive::Type> dirMap = {
        {TokenType::DIRECTIVE_WORD, Directive::Type::WORD},
        {TokenType::DIRECTIVE_BYTE, Directive::Type::BYTE},
        {TokenType::DIRECTIVE_ASCII, Directive::Type::ASCII},
        {TokenType::DIRECTIVE_END, Directive::Type::END},
        {TokenType::DIRECTIVE_EQU, Directive::Type::EQU},
        {TokenType::DIRECTIVE_FILL, Directive::Type::FILL},
    };
    
    if (dirMap.count(currentToken().type)) {
        return parseDirective();
    }
    
    throw std::runtime_error("Unexpected token: " + currentToken().value);
}

std::unique_ptr<Label> Parser::parseLabel() {
    std::string labelName = currentToken().value;
    advance(); // Пропускаем имя метки
    expect(TokenType::COLON, "Expected ':' after label");
    
    // Метка может быть пустой или содержать statement
    std::unique_ptr<ASTNode> stmt;
    if (!match(TokenType::END_OF_FILE)) {
        stmt = parseStatement();
    }
    return ASTBuilder::createLabel(labelName, std::move(stmt));
}

std::unique_ptr<Instruction> Parser::parseInstruction() {
    auto type = static_cast<Instruction::Type>(currentToken().type);
    advance(); // Пропускаем мнемонику
    
    std::unique_ptr<Operand> src, dst;
    
    // Обработка операндов в зависимости от типа инструкции
    if (type != Instruction::Type::RTS && 
        type != Instruction::Type::HALT) {
        src = parseOperand();
        
        if (match(TokenType::COMMA)) {
            advance();
            dst = parseOperand();
        }
    }

    switch (type) {
        case Instruction::Type::MOV: return ASTBuilder::createMov(std::move(src), std::move(dst));
        case Instruction::Type::CMP: return ASTBuilder::createCmp(std::move(src), std::move(dst));
        case Instruction::Type::ADD: return ASTBuilder::createAdd(std::move(src), std::move(dst));
        case Instruction::Type::SUB: return ASTBuilder::createSub(std::move(src), std::move(dst));
        case Instruction::Type::JSR: return ASTBuilder::createJsr(src->label);
        case Instruction::Type::RTS: return ASTBuilder::createRts();
        case Instruction::Type::HALT: return ASTBuilder::createHalt();
        case Instruction::Type::CLR: return ASTBuilder::createClr(std::move(dst));
        case Instruction::Type::COM: return ASTBuilder::createCom(std::move(dst));
        case Instruction::Type::INC: return ASTBuilder::createInc(std::move(dst));
        case Instruction::Type::DEC: return ASTBuilder::createDec(std::move(dst));
        case Instruction::Type::NEG: return ASTBuilder::createNeg(std::move(dst));
        case Instruction::Type::JMP: return ASTBuilder::createJmp(src->label);
        default:
            throw std::runtime_error("Unsupported instruction");
    }
}

std::unique_ptr<Directive> Parser::parseDirective() {
    auto type = static_cast<Directive::Type>(currentToken().type);
    advance(); // Пропускаем директиву
    
    std::vector<std::unique_ptr<Operand>> operands;
    
    // Парсим операнды директивы
    while (!match(TokenType::END_OF_FILE)) {
        operands.push_back(parseOperand());
        if (!match(TokenType::COMMA)) break;
        advance();
    }
    
    switch (type) {
        case Directive::Type::WORD: return ASTBuilder::createWord(std::move(operands));
        case Directive::Type::BYTE: return ASTBuilder::createByte(std::move(operands));
        case Directive::Type::ASCII: {
            if (operands.empty()) throw std::runtime_error("Expected string for .ASCII");
            return ASTBuilder::createAscii(operands[0]->label);
        }
        case Directive::Type::EQU: {
            if (operands.size() != 2) throw std::runtime_error("Expected label and value for .EQU");
            return ASTBuilder::createEqu(operands[0]->label, operands[1]->value);
        }
        case Directive::Type::END: return ASTBuilder::createEnd();
        case Directive::Type::FILL: {
            if (operands.size() != 2) throw std::runtime_error("Expected count and value for .FILL");
            return ASTBuilder::createFill(operands[0]->value, operands[1]->value);
        }
        default:
            throw std::runtime_error("Unsupported directive");
    }
}


std::unique_ptr<Operand> Parser::parseOperand() {
    AddrMode mode = parseAddressingMode();
    auto op = std::make_unique<Operand>();
    op->mode = mode;
    switch (mode) {
        // Регистровый: Rn
        case AddrMode::REGISTER: {
            op->reg = currentToken().value;
            advance();
            break;
        }

        // Непосредственный: #value
        case AddrMode::IMMEDIATE: {
            expect(TokenType::NUMBER, "Expected number after '#'");
            op->value = std::stoi(currentToken().value);
            advance();
            break;
        }

        // Относительный: label
        case AddrMode::RELATIVE: {
            expect(TokenType::LABEL, "Expected label");
            op->label = currentToken().value;
            advance();
            break;
        }

        // Абсолютный: @#address
        case AddrMode::ABSOLUTE: {
            expect(TokenType::NUMBER, "Expected address after '@#'");
            op->value = std::stoi(currentToken().value);
            advance();
            break;
        }

        // Косвенно-регистровый: (Rn)
        case AddrMode::REG_DEF: {
            expect(TokenType::REGISTER, "Expected register after '('");
            op->reg = currentToken().value;
            advance();
            expect(TokenType::RPAREN, "Expected ')' after register");
            advance();
            break;
        }

        // Автоинкрементный: (Rn)+
        case AddrMode::AUTOINC: {
            expect(TokenType::REGISTER, "Expected register after '('");
            op->reg = currentToken().value;
            advance();
            expect(TokenType::PLUS, "Expected '+' after register");
            advance();
            expect(TokenType::RPAREN, "Expected ')' after '+)'");
            advance();
            break;
        }

        // Автодекрементный: -(Rn)
        case AddrMode::AUTODEC: {
            expect(TokenType::MINUS, "Expected '-'");
            advance();
            expect(TokenType::LPAREN, "Expected '(' after '-'");
            advance();
            expect(TokenType::REGISTER, "Expected register after '-('");
            op->reg = currentToken().value;
            advance();
            expect(TokenType::RPAREN, "Expected ')' after register");
            advance();
            break;
        }

        // Индексный: X(Rn)
        case AddrMode::INDEXED: {
            // Смещение (может быть числом или меткой)
            if (match(TokenType::NUMBER)) {
                op->value = std::stoi(currentToken().value);
            } else if (match(TokenType::LABEL)) {
                op->label = currentToken().value;
            } else {
                throw std::runtime_error("Expected number or label for offset");
            }
            advance();
            
            expect(TokenType::LPAREN, "Expected '(' after offset");
            advance();
            expect(TokenType::REGISTER, "Expected register in indexed mode");
            op->reg = currentToken().value;
            advance();
            expect(TokenType::RPAREN, "Expected ')' after register");
            advance();
            break;
        }
        default:
            throw std::runtime_error("Unsupported addressing mode");
    }
    return op;
}
AddrMode Parser::parseAddressingMode() {
    // Immediate: #value
    if (match(TokenType::HASH)) {
        advance();
        return AddrMode::IMMEDIATE;
    }
    
    // Absolute: @#address
    if (match(TokenType::AT)) {
        advance();
        if (match(TokenType::HASH)) {
            advance();
            return AddrMode::ABSOLUTE;
        }
        // Relative: @address
        return AddrMode::RELATIVE;
    }
    
    // Register deferred: (Rn)
    if (match(TokenType::LPAREN)) {
        advance();
        expect(TokenType::REGISTER, "Expected register after '('");
        std::string reg = currentToken().value;
        advance();
        
        // Auto-increment: (Rn)+
        if (match(TokenType::PLUS)) {
            advance();
            expect(TokenType::RPAREN, "Expected ')' after '(Rn)+'");
            advance();
            return AddrMode::AUTOINC;
        }
        
        expect(TokenType::RPAREN, "Expected ')' after '(Rn)'");
        advance();
        return AddrMode::REG_DEF;
    }
    
    // Auto-decrement: -(Rn)
    if (match(TokenType::MINUS)) {
        advance();
        expect(TokenType::LPAREN, "Expected '(' after '-'");
        advance();
        expect(TokenType::REGISTER, "Expected register after '-(Rn)'");
        std::string reg = currentToken().value;
        advance();
        expect(TokenType::RPAREN, "Expected ')' after '-(Rn)'");
        advance();
        return AddrMode::AUTODEC;
    }
    
    // Indexed: X(Rn)
    if (match(TokenType::NUMBER)) {
        int offset = std::stoi(currentToken().value);
        advance();
        expect(TokenType::LPAREN, "Expected '(' after number in indexed mode");
        advance();
        expect(TokenType::REGISTER, "Expected register in indexed mode");
        std::string reg = currentToken().value;
        advance();
        expect(TokenType::RPAREN, "Expected ')' in indexed mode");
        advance();
        return AddrMode::INDEXED;
    }
    
    // Register: Rn
    if (match(TokenType::REGISTER)) {
        return AddrMode::REGISTER;
    }
    
    // Relative: label
    if (match(TokenType::LABEL)) {
        return AddrMode::RELATIVE;
    }
    
    throw std::runtime_error("Unknown addressing mode");
}

// Вспомогательные методы
const Token& Parser::currentToken() const {
    if (currentPos >= tokens.size()) {
        static Token eof{TokenType::END_OF_FILE, "", 0, 0};
        return eof;
    }
    return tokens[currentPos];
}

const Token& Parser::peekToken() const {
    if (currentPos + 1 >= tokens.size()) {
        static Token eof{TokenType::END_OF_FILE, "", 0, 0};
        return eof;
    }
    return tokens[currentPos + 1];
}

void Parser::advance() {
    if (currentPos < tokens.size()) {
        currentPos++;
    }
}

bool Parser::match(TokenType type) {
    return currentToken().type == type;
}

void Parser::expect(TokenType type, const std::string& errorMsg) {
    if (!match(type)) {
        throw std::runtime_error(errorMsg + " at line " + 
                               std::to_string(currentToken().line));
    }
}