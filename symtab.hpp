#ifndef PDP11_SYMTAB_HPP
#define PDP11_SYMTAB_HPP

#include "ast.hpp"
#include <unordered_map>
#include <vector>
#include <stdexcept>

class SymbolTable {
public:
    // Запись в таблице символов
    struct Symbol {
        uint16_t value;         // Числовое значение (адрес или константа)
        bool is_defined = false; // Определен ли символ
        bool is_constant = false; // Это константа (.EQU)?
        size_t line;            // Строка определения
    };

    // Построение таблицы символов
    void build(const Program& program);
    
    // Разрешение символа
    uint16_t resolve(const std::string& name) const;
    
    // Проверка на наличие неразрешенных символов
    void validate() const;
    
    // Получение текущего адреса (PC)
    uint16_t currentAddress() const { return current_addr; }
    std::unordered_map<std::string, Symbol> symbols;

private:

    uint16_t current_addr = 0; // Текущий адрес в памяти
    
    void processNode(const ASTNode& node);
    void processInstruction(const Instruction& instr);
    void processDirective(const Directive& dir);
    void processLabel(const Label& label);
};

#endif // PDP11_SYMTAB_HPP