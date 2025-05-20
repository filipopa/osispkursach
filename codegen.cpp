#include "codegen.hpp"
#include <algorithm>
#include <iostream>

std::vector<uint16_t> CodeGenerator::generate(Program& program) {
    output.clear();
    current_pc = 0;
    program.accept(*this);
    return output;
}

void CodeGenerator::emit(uint16_t word) {
    printf("Writing in bin: %04o (hex: %04X)\n", word, word);
    output.push_back(word);
    current_pc++;
}

void CodeGenerator::visit(const Program& program) {
    for (const auto& stmt : program.statements) {
        stmt->accept(*this);
    }
}

void CodeGenerator::visit(const Label& label) {
    if (label.statement) {
        label.statement->accept(*this);
    }
}

void CodeGenerator::visit(const Instruction& instr) {
    encodeInstruction(instr);
}
void CodeGenerator::encodeInstruction(const Instruction& instr) {
    uint16_t opcode = 0;
    
    // Определяем опкод
    switch (instr.type) {
        
        // Двухоперандные инструкции
        case Instruction::Type::MOV: opcode = 0010000; break;  // MOV src,dst
        case Instruction::Type::CMP: opcode = 0020000; break;  // CMP src,dst
        case Instruction::Type::ADD: opcode = 060000; break;  // ADD src,dst
        case Instruction::Type::SUB: opcode = 0160000; break;  // SUB src,dst
        
        // Однооперандные инструкции
        case Instruction::Type::JSR: 
            opcode = 0004000; 
            // JSR имеет особый формат: JSR R,dst
            emit(opcode | (encodeRegister(instr.src->reg) << 6) | encodeOperand(*instr.dst, false));
            if (instr.dst->mode == AddrMode::RELATIVE || 
                instr.dst->mode == AddrMode::ABSOLUTE) {
                emit(instr.dst->value);
            }
            return;
            
        case Instruction::Type::CLR: opcode = 0005000; break;  // CLR dst
        case Instruction::Type::COM: opcode = 0005100; break;  // COM dst
        case Instruction::Type::INC: opcode = 0005200; break;  // INC dst
        case Instruction::Type::DEC: opcode = 0005300; break;  // DEC dst
        case Instruction::Type::NEG: opcode = 0005400; break;  // NEG dst

        // Инструкции без операндов
        case Instruction::Type::RTS:
            emit(0000200 | encodeRegister(instr.dst->reg));
            return;
        case Instruction::Type::HALT: emit(0000000); return;  // HALT
        case Instruction::Type::JMP: opcode = 0000100; break;  // JMP dst
        
        default:
            throw std::runtime_error("Unsupported instruction");
    }

    printf("=== DEBUG ===\n");
    printf("Opcode: %06o (oct) = %04X (hex)\n", opcode, opcode);
    
    // Кодируем операнды для обычных инструкций
    uint16_t src_mode = 0;
        if (instr.src) {
        src_mode = encodeOperand(*instr.src, true);
        printf("Src mode: %03o (oct) = %02X (hex)\n", src_mode, src_mode);
    }
    uint16_t dst_mode = 0;
        if (instr.dst) {
        dst_mode = encodeOperand(*instr.dst, false);
        printf("Dst mode: %03o (oct) = %02X (hex)\n", dst_mode, dst_mode);
    }

    if (instr.src && instr.type != Instruction::Type::JMP) {
        src_mode = encodeOperand(*instr.src, true);
    }
    
    if (instr.dst) {
        dst_mode = encodeOperand(*instr.dst, false);
    }
    
    // Особый случай для JMP
    if (instr.type == Instruction::Type::JMP) {
        if (!instr.dst) throw std::runtime_error("JMP requires destination");
        emit(opcode | encodeOperand(*instr.dst, false));
        
        if (instr.dst->mode == AddrMode::RELATIVE || 
            instr.dst->mode == AddrMode::ABSOLUTE) {
            emit(instr.dst->value);
        }
        return;
    }
    
    uint16_t word = opcode | (src_mode * 0100) | dst_mode; 
    printf("Final word: %06o (oct) = %04X (hex)\n", word, word);
    emit(word);

    // Дополнительные слова для некоторых режимов адресации
    if (instr.src && (instr.src->mode == AddrMode::IMMEDIATE || 
                      instr.src->mode == AddrMode::ABSOLUTE ||
                      instr.src->mode == AddrMode::RELATIVE ||
                      instr.src->mode == AddrMode::INDEXED)) {
        emit(instr.src->value);
    }
    
    if (instr.dst && (instr.dst->mode == AddrMode::IMMEDIATE || 
                      instr.dst->mode == AddrMode::ABSOLUTE ||
                      instr.dst->mode == AddrMode::RELATIVE ||
                      instr.dst->mode == AddrMode::INDEXED)) {
        emit(instr.dst->value);
    }

}

uint16_t CodeGenerator::encodeOperand(const Operand& op, bool is_src) {
    uint16_t encoded = 0;
    
    switch (op.mode) {
        case AddrMode::REGISTER:
            encoded = encodeRegister(op.reg);
            break;
        case AddrMode::IMMEDIATE:
            encoded = 027; // #n
            if (!is_src) throw std::runtime_error("Immediate mode not allowed for destination");
            break;
        case AddrMode::RELATIVE:
            encoded = 067; // address
            break;
        case AddrMode::ABSOLUTE:
            encoded = 037; // @#address
            break;
        case AddrMode::REG_DEF:
            encoded = 010 | encodeRegister(op.reg);
            break;
        case AddrMode::AUTOINC:
            encoded = 020 | encodeRegister(op.reg);
            break;
        case AddrMode::AUTODEC:
            encoded = 030 | encodeRegister(op.reg);
            break;
        case AddrMode::INDEXED:
            encoded = 060 | encodeRegister(op.reg);
            emit(op.value); // Дополнительное слово
            break;
        default:
            throw std::runtime_error("Unknown addressing mode");
    }
    
    return encoded;
}

uint16_t CodeGenerator::encodeRegister(const std::string& reg) {
    if (reg == "PC") return 07;
    if (reg == "SP") return 06;
    if (reg[0] == 'R' && reg.size() == 2 && isdigit(reg[1])) {
        return reg[1] - '0';
    }
    throw std::runtime_error("Invalid register: " + reg);
}

void CodeGenerator::visit(const Directive& dir) {
    switch (dir.type) {
        case Directive::Type::WORD:
            for (const auto& op : dir.operands) {
                emit(op->value);
            }
            break;
        case Directive::Type::BYTE:
            for (const auto& op : dir.operands) {
                emit(op->value & 0xFF);
                if (dir.operands.size() % 2 != 0) {
                    emit(0); // Выравнивание
                }
            }
            break;
        case Directive::Type::ASCII:
            for (const auto& op : dir.operands) {
                emit(static_cast<uint16_t>(op->value) & 0xFF);
            }
            break;
        default:
            break;
    }
}

void CodeGenerator::visit(const Operand& op) {
    // Операнды обрабатываются в encodeInstruction
}