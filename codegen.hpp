#ifndef PDP11_CODEGEN_HPP
#define PDP11_CODEGEN_HPP

#include "ast.hpp"
#include "symtab.hpp"
#include <vector>
#include <cstdint>
#include <stdexcept>

class CodeGenerator : public ASTVisitor {
public:
    explicit CodeGenerator(SymbolTable& symtab) : symtab(symtab) {}
    
    std::vector<uint16_t> generate(Program& program);
    
    // Visitor методы
    void visit(const Instruction& instr) override;
    void visit(const Operand& op) override;
    void visit(const Directive& dir) override;
    void visit(const Label& label) override;
    void visit(const Program& program) override;

private:
    SymbolTable& symtab;
    std::vector<uint16_t> output;
    uint16_t current_pc = 0;
    
    void emit(uint16_t word);
    void encodeInstruction(const Instruction& instr);
    uint16_t encodeOperand(const Operand& op, bool is_src);
    uint16_t encodeRegister(const std::string& reg);
};

#endif // PDP11_CODEGEN_HPP