#include "lexer.hpp"
#include "parser.hpp"
#include "symtab.hpp"
#include "codegen.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

void saveBinary(const std::string& filename, const std::vector<uint16_t>& code) {
    std::ofstream out(filename, std::ios::binary);
    for (auto word : code) {
        uint8_t bytes[2] = {static_cast<uint8_t>(word & 0xFF), 
                            static_cast<uint8_t>((word >> 8) & 0xFF)};
        out.write(reinterpret_cast<const char*>(bytes), 2);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input.asm> <output.bin>\n";
        return 1;
    }

    try {
         printf("1. File read\n");
        // 1. Чтение исходного файла
        std::ifstream file(argv[1]);
        std::string source((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        printf("2. Lexer\n");
        // 2. Лексический анализ
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        printf("3. Parser\n");
        // 3. Синтаксический анализ
        Parser parser(tokens);
        auto program = parser.parseProgram();

        printf("4. Symtab\n");
        // 4. Построение таблицы символов
        SymbolTable symtab;
        symtab.build(*program);

        printf("5. Codegen\n");
        // 5. Генерация кода
        CodeGenerator generator(symtab);
        auto machine_code = generator.generate(*program);
        printf("6. Saving bin...\n");
        
        // 6. Сохранение результата
        saveBinary(argv[2], machine_code);

        std::cout << "Successfully generated " << machine_code.size() 
                  << " words of machine code.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}