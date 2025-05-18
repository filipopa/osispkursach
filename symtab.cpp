#include "symtab.hpp"
#include <sstream>

void SymbolTable::build(const Program& program) {
    current_addr = 0; // Начинаем с адреса 0
    
    for (const auto& stmt : program.statements) {
        processNode(*stmt);
    }
    
    validate(); // Проверяем все ли символы разрешены
}

void SymbolTable::processNode(const ASTNode& node) {
    if (auto instr = dynamic_cast<const Instruction*>(&node)) {
        processInstruction(*instr);
        // Инструкции PDP-11 занимают 1 или 2 слова
        current_addr += (instr->dst ? 2 : 1);
    }
    else if (auto dir = dynamic_cast<const Directive*>(&node)) {
        processDirective(*dir);
        // Обновляем адрес в зависимости от директивы
        switch (dir->type) {
            case Directive::Type::WORD:
                current_addr += dir->operands.size();
                break;
            case Directive::Type::BYTE:
                current_addr += (dir->operands.size() + 1) / 2;
                break;
            case Directive::Type::FILL:
                current_addr += dir->operands[0]->value;
                break;
            // ... другие директивы
            default:
                break;
        }
    }
    else if (auto label = dynamic_cast<const Label*>(&node)) {
        processLabel(*label);
        if (label->statement) {
            processNode(*label->statement);
        }
    }
}

void SymbolTable::processInstruction(const Instruction& instr) {
    // Для инструкций нужно обновить current_addr
    // (уже обрабатывается в processNode)
}

void SymbolTable::processDirective(const Directive& dir) {
    if (dir.type == Directive::Type::EQU) {
        // Обработка констант вида LABEL .EQU value
        if (dir.operands.size() != 2) {
            throw std::runtime_error("Invalid .EQU directive");
        }
        
        const auto& label = dynamic_cast<const Operand&>(*dir.operands[0]);
        const auto& value = dynamic_cast<const Operand&>(*dir.operands[1]);
        
        symbols[label.label] = {
            static_cast<uint16_t>(value.value),
            true,
            true,
            current_addr
        };
    }
}

void SymbolTable::processLabel(const Label& label) {
    if (symbols.count(label.name) && symbols[label.name].is_defined) {
        throw std::runtime_error("Duplicate label: " + label.name);
    }
    
    symbols[label.name] = {
        current_addr,
        true,
        false,
        current_addr
    };
}

uint16_t SymbolTable::resolve(const std::string& name) const {
    if (!symbols.count(name)) {
        throw std::runtime_error("Undefined symbol: " + name);
    }
    return symbols.at(name).value;
}

void SymbolTable::validate() const {
    for (const auto& [name, sym] : symbols) {
        if (!sym.is_defined) {
            throw std::runtime_error("Symbol not defined: " + name);
        }
    }
}