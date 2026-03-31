#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>

#include "lexer.hpp"
#include "optimizer.hpp"
#include "codegen.hpp"

/**
 * Welcome to the Brainfuck-to-x86-64 Compiler!
 * This was a fun challenge to build a "real" compiler that produces
 * optimized machine code from a seemingly simple language.
 */

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Hey! It looks like you forgot to provide a source file.\n";
        std::cout << "Usage: " << argv[0] << " <source.bf> [-o executable_name]\n";
        return 1;
    }

    std::string source_path = argv[1];
    std::string output_name = "a.out";
    bool should_link = false;

    // A very simple argument parser
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            output_name = argv[++i];
            should_link = true;
        }
    }

    // 1. Read the source code
    std::ifstream in(source_path);
    if (!in) {
        std::cerr << "Error: I couldn't open '" << source_path << "'. Does it exist?\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string source = buffer.str();

    try {
        std::cout << "--- Starting Compilation ---" << std::endl;

        // 2. Lexical analysis (Source -> ActionPlan)
        auto plan = bf::Lexer::parse(source);
        std::cout << "Parsed " << plan.size() << " basic actions." << std::endl;

        // 3. Optimization (ActionPlan -> Optimized ActionPlan)
        plan = bf::Optimizer::optimize(plan);
        std::cout << "Optimized down to " << plan.size() << " high-speed actions." << std::endl;

        // 4. Code Generation (ActionPlan -> NASM Assembly)
        std::string asm_path = source_path + ".s";
        std::ofstream out(asm_path);
        if (!out) {
            std::cerr << "Error: I couldn't create the assembly file '" << asm_path << "'.\n";
            return 1;
        }

        bf::CodeGen::generate_nasm(plan, out);
        out.close();

        // 5. Assembling and Linking (Optional)
        if (should_link) {
            std::string obj_path = source_path + ".o";
            std::string nasm_cmd = "nasm -f elf64 " + asm_path + " -o " + obj_path;
            std::string ld_cmd = "ld " + obj_path + " -o " + output_name;

            std::cout << "Assembling with NASM..." << std::endl;
            if (std::system(nasm_cmd.c_str()) != 0) return 1;

            std::cout << "Linking with LD..." << std::endl;
            if (std::system(ld_cmd.c_str()) != 0) return 1;

            // Cleanup temp files
            std::remove(asm_path.c_str());
            std::remove(obj_path.c_str());
            std::cout << "Success! Your executable is ready: ./" << output_name << std::endl;
        } else {
            std::cout << "Success! Assembly generated: " << asm_path << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Oops! Something went wrong: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
