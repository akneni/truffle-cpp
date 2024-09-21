#include "json.hpp"
#include "lexer.h"
#include "parser.h"
#include "scope_tr.h"
#include "code_gen.h"

#include <time.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <regex>
#include <optional>
#include <stdexcept>
#include <cctype>
#include <iomanip>
#include <fstream>

std::string f_read_to_string(std::string filepath) {
    // Open the file in input mode
    std::ifstream file(filepath);

    // Check if the file is open
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }

    // Use a stringstream to read the entire file into a string
    std::stringstream buffer;
    buffer << file.rdbuf();

    // Close the file
    file.close();

    // Return the contents of the file as a string
    return buffer.str();
}

void write_to_file(const std::string& path, const std::string& data) {
    // Create an ofstream object to handle file output
    std::ofstream outfile(path);

    // Check if the file was opened successfully
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << path << std::endl;
        return;
    }

    // Write the data to the file
    outfile << data;

    // Close the file
    outfile.close();

    // Optionally, you can notify the user that the operation was successful
    std::cout << "Data written to file: " << path << std::endl;
}


void generate_ast() {
    time_t start = clock();
    
    
    std::string source = f_read_to_string("truffle/main.tr");

    Lexer lexer = Lexer(source);

    
    while (true) {
        auto x = lexer.next();
        if (!x.has_value()) {
            break;
        }
    }

    VarLst var_lst = VarLst();
    FuncLst fn_lst = FuncLst();

    fn_lst.push_back(FunctionTr {
        .name = "print",
        .param_type = {},
        .ret_type = BeDataType::Null,
    });

    fn_lst.push_back(FunctionTr {
        .name = "__some_c_func",
        .param_type = {},
        .ret_type = BeDataType::Null,
    });

    unsigned int idx = 0;
    nlohmann::json ast = parse_module(lexer.tokens, idx, &var_lst, &fn_lst);

    std::string ast_str = ast.dump();
    write_to_file("ast.json", ast_str);

    time_t end = clock();
    printf("Time Elapsed: %f", ((double) end - (double) start) / (double) CLOCKS_PER_SEC);
}



int main() {
    std::cout << "Starting LLVM code gen...\n";

    // TEMP
    generate_ast();
    return 0;

    std::string ast_str = f_read_to_string("ast.json");
    
    nlohmann::json ast = nlohmann::json::parse(ast_str);

    gen_llvm_ir("truffle-main.ll", ast);

    return 0;
}
