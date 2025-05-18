#ifndef PDP11_AST_HPP
#define PDP11_AST_HPP

#include <memory>
#include <vector>
#include <string>
#include <cstdint>

// ========================================================
// 1. Режимы адресации PDP-11 (полный набор)
// ========================================================
enum class AddrMode {
    REGISTER,     // R0-R7, SP, PC
    IMMEDIATE,    // #42
    ABSOLUTE,     //@#address
    RELATIVE,     //address
    REG_DEF,      //(Rn)
    AUTOINC,      //(Rn)+
    AUTODEC,      //-(Rn)
    INDEXED       //X(Rn)
};

// ========================================================
// 2. Базовые классы AST + Visitor Pattern
// ========================================================
struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void accept(class ASTVisitor& visitor) const = 0;
};

struct ASTVisitor {
    virtual ~ASTVisitor() = default;
    virtual void visit(const class Instruction&) = 0;
    virtual void visit(const class Operand&) = 0;
    virtual void visit(const class Directive&) = 0;
    virtual void visit(const class Label&) = 0;
    virtual void visit(const class Program&) = 0;
};

// ========================================================
// 3. Конкретные узлы AST
// ========================================================
struct Operand : ASTNode {
    AddrMode mode;
    std::string reg;    // Для регистров: "R1", "PC"
    int value = 0;      // Для чисел (#42, 0o52)
    std::string label;   // Для меток
    
    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }
};

struct Instruction : ASTNode {
    enum class Type {
        MOV, CMP, ADD, SUB, JSR, RTS, 
        HALT, CLR, COM, INC, DEC, NEG, JMP
    } type;
    
    std::unique_ptr<Operand> src;
    std::unique_ptr<Operand> dst;
    
    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }
};

struct Directive : ASTNode {
    enum class Type {
        WORD, BYTE, END, EQU, ASCII, FILL
    } type;
    
    std::vector<std::unique_ptr<Operand>> operands;
    
    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }
};

struct Label : ASTNode {
    std::string name;
    std::unique_ptr<ASTNode> statement;
    
    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }
};

struct Program : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
    
    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }
};

// ========================================================
// 4. Вспомогательные билдеры (опционально)
// ========================================================
namespace ASTBuilder {
    std::unique_ptr<Program> createProgram();
    std::unique_ptr<Operand> createReg(const std::string& reg);
    std::unique_ptr<Operand> createImm(int value);
    std::unique_ptr<Operand> createLabelRef(const std::string& label);
    std::unique_ptr<Instruction> createMov(
        std::unique_ptr<Operand> src, 
        std::unique_ptr<Operand> dst);
std::unique_ptr<Operand> createAbs(const std::string& label);
std::unique_ptr<Operand> createRel(const std::string& label);
std::unique_ptr<Operand> createRegDef(const std::string& reg);
std::unique_ptr<Operand> createAutoInc(const std::string& reg);
std::unique_ptr<Operand> createAutoDec(const std::string& reg);
std::unique_ptr<Operand> createIndexed(int offset, const std::string& reg);

std::unique_ptr<Instruction> createCmp(
    std::unique_ptr<Operand> src,
    std::unique_ptr<Operand> dst);
std::unique_ptr<Instruction> createAdd(
    std::unique_ptr<Operand> src,
    std::unique_ptr<Operand> dst);
std::unique_ptr<Instruction> createSub(
    std::unique_ptr<Operand> src,
    std::unique_ptr<Operand> dst);
std::unique_ptr<Instruction> createJsr(const std::string& target);
std::unique_ptr<Instruction> createRts();
std::unique_ptr<Instruction> createHalt();
std::unique_ptr<Instruction> createClr(std::unique_ptr<Operand> dst);
std::unique_ptr<Instruction> createCom(std::unique_ptr<Operand> dst);
std::unique_ptr<Instruction> createInc(std::unique_ptr<Operand> dst);
std::unique_ptr<Instruction> createDec(std::unique_ptr<Operand> dst);
std::unique_ptr<Instruction> createNeg(std::unique_ptr<Operand> dst);
std::unique_ptr<Instruction> createJmp(const std::string& target);

std::unique_ptr<Directive> createWord(std::vector<std::unique_ptr<Operand>> values);
std::unique_ptr<Directive> createByte(std::vector<std::unique_ptr<Operand>> values);
std::unique_ptr<Directive> createAscii(const std::string& text);
std::unique_ptr<Directive> createEqu(const std::string& label, int value);
std::unique_ptr<Directive> createEnd();
std::unique_ptr<Directive> createFill(int count, int value);

std::unique_ptr<Label> createLabel(const std::string& name, std::unique_ptr<ASTNode> statement);
}

#endif // PDP11_AST_HPP