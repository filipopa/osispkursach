#include "ast.hpp"
#include <stdexcept>

namespace ASTBuilder {
    
    std::unique_ptr<Program> createProgram() {
        return std::make_unique<Program>();
    }
// ========== Operands ==========
std::unique_ptr<Operand> createReg(const std::string& reg) {
    if (reg != "PC" && reg != "SP" && 
       !(reg.size() == 2 && reg[0] == 'R' && reg[1] >= '0' && reg[1] <= '7')) {
        throw std::invalid_argument("Invalid register: " + reg);
    }
    
    auto op = std::make_unique<Operand>();
    op->mode = AddrMode::REGISTER;
    op->reg = reg;
    return op;
}

std::unique_ptr<Operand> createImm(int value) {
    auto op = std::make_unique<Operand>();
    op->mode = AddrMode::IMMEDIATE;
    op->value = value;
    return op;
}

std::unique_ptr<Operand> createAbs(const std::string& label) {
    auto op = std::make_unique<Operand>();
    op->mode = AddrMode::ABSOLUTE;
    op->label = label;
    return op;
}

std::unique_ptr<Operand> createRel(const std::string& label) {
    auto op = std::make_unique<Operand>();
    op->mode = AddrMode::RELATIVE;
    op->label = label;
    return op;
}

std::unique_ptr<Operand> createRegDef(const std::string& reg) {
    auto op = std::make_unique<Operand>();
    op->mode = AddrMode::REG_DEF;
    op->reg = reg;
    return op;
}

std::unique_ptr<Operand> createAutoInc(const std::string& reg) {
    auto op = std::make_unique<Operand>();
    op->mode = AddrMode::AUTOINC;
    op->reg = reg;
    return op;
}

std::unique_ptr<Operand> createAutoDec(const std::string& reg) {
    auto op = std::make_unique<Operand>();
    op->mode = AddrMode::AUTODEC;
    op->reg = reg;
    return op;
}

std::unique_ptr<Operand> createIndexed(int offset, const std::string& reg) {
    auto op = std::make_unique<Operand>();
    op->mode = AddrMode::INDEXED;
    op->value = offset;
    op->reg = reg;
    return op;
}

std::unique_ptr<Operand> createLabelRef(const std::string& label) {
    auto op = std::make_unique<Operand>();
    op->mode = AddrMode::RELATIVE; // Для меток обычно относительная адресация
    op->label = label;
    return op;
}

// ========== Instructions ==========
std::unique_ptr<Instruction> createMov(
    std::unique_ptr<Operand> src,
    std::unique_ptr<Operand> dst) 
{
    auto instr = std::make_unique<Instruction>();
    instr->type = Instruction::Type::MOV;
    instr->src = std::move(src);
    instr->dst = std::move(dst);
    return instr;
}

std::unique_ptr<Instruction> createCmp(
    std::unique_ptr<Operand> src,
    std::unique_ptr<Operand> dst) 
{
    auto instr = std::make_unique<Instruction>();
    instr->type = Instruction::Type::CMP;
    instr->src = std::move(src);
    instr->dst = std::move(dst);
    return instr;
}

std::unique_ptr<Instruction> createAdd(
    std::unique_ptr<Operand> src,
    std::unique_ptr<Operand> dst) 
{
    auto instr = std::make_unique<Instruction>();
    instr->type = Instruction::Type::ADD;
    instr->src = std::move(src);
    instr->dst = std::move(dst);
    return instr;
}

std::unique_ptr<Instruction> createJsr(const std::string& target) {
    auto instr = std::make_unique<Instruction>();
    instr->type = Instruction::Type::JSR;
    instr->dst = createRel(target);
    return instr;
}

std::unique_ptr<Instruction> createRts() {
    auto instr = std::make_unique<Instruction>();
    instr->type = Instruction::Type::RTS;
    return instr;
}

std::unique_ptr<Instruction> createClr(std::unique_ptr<Operand> dst) {
    auto instr = std::make_unique<Instruction>();
    instr->type = Instruction::Type::CLR;
    instr->dst = std::move(dst);
    return instr;
}

std::unique_ptr<Instruction> createSub(
    std::unique_ptr<Operand> src,
    std::unique_ptr<Operand> dst) 
{
    auto instr = std::make_unique<Instruction>();
    instr->type = Instruction::Type::SUB;
    instr->src = std::move(src);
    instr->dst = std::move(dst);
    return instr;
}

std::unique_ptr<Instruction> createJmp(const std::string& target) {
    auto instr = std::make_unique<Instruction>();
    instr->type = Instruction::Type::JMP;
    instr->dst = createRel(target);
    return instr;
}

std::unique_ptr<Instruction> createHalt() {
    auto instr = std::make_unique<Instruction>();
    instr->type = Instruction::Type::HALT;
    return instr;
}

std::unique_ptr<Instruction> createCom(std::unique_ptr<Operand> dst) {
    auto instr = std::make_unique<Instruction>();
    instr->type = Instruction::Type::COM;
    instr->dst = std::move(dst);
    return instr;
}

std::unique_ptr<Instruction> createInc(std::unique_ptr<Operand> dst) {
    auto instr = std::make_unique<Instruction>();
    instr->type = Instruction::Type::INC;
    instr->dst = std::move(dst);
    return instr;
}

std::unique_ptr<Instruction> createDec(std::unique_ptr<Operand> dst) {
    auto instr = std::make_unique<Instruction>();
    instr->type = Instruction::Type::DEC;
    instr->dst = std::move(dst);
    return instr;
}

std::unique_ptr<Instruction> createNeg(std::unique_ptr<Operand> dst) {
    auto instr = std::make_unique<Instruction>();
    instr->type = Instruction::Type::NEG;
    instr->dst = std::move(dst);
    return instr;
}

// ========== Directives ==========
std::unique_ptr<Directive> createWord(std::vector<std::unique_ptr<Operand>> values) {
    auto dir = std::make_unique<Directive>();
    dir->type = Directive::Type::WORD;
    dir->operands = std::move(values);
    return dir;
}

std::unique_ptr<Directive> createAscii(const std::string& text) {
    auto dir = std::make_unique<Directive>();
    dir->type = Directive::Type::ASCII;
    
    // Каждый символ - отдельный операнд
    for (char c : text) {
        dir->operands.push_back(createImm(static_cast<int>(c)));
    }
    
    return dir;
}

std::unique_ptr<Directive> createByte(std::vector<std::unique_ptr<Operand>> values) {
    auto dir = std::make_unique<Directive>();
    dir->type = Directive::Type::BYTE;
    dir->operands = std::move(values);
    return dir;
}

std::unique_ptr<Directive> createEqu(const std::string& label, int value) {
    auto dir = std::make_unique<Directive>();
    dir->type = Directive::Type::EQU;
    dir->operands.push_back(createLabelRef(label));
    dir->operands.push_back(createImm(value));
    return dir;
}

std::unique_ptr<Directive> createEnd() {
    auto dir = std::make_unique<Directive>();
    dir->type = Directive::Type::END;
    return dir;
}

std::unique_ptr<Directive> createFill(int count, int value) {
    auto dir = std::make_unique<Directive>();
    dir->type = Directive::Type::FILL;
    dir->operands.push_back(createImm(count));
    dir->operands.push_back(createImm(value));
    return dir;
}

// ========== Labels ==========
std::unique_ptr<Label> createLabel(const std::string& name, std::unique_ptr<ASTNode> statement) {
    auto label = std::make_unique<Label>();
    label->name = name;
    label->statement = std::move(statement);
    return label;
}

} // namespace ASTBuilder