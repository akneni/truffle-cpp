#include "json.hpp"
#include "lexer.h"
#include "parser.h"
#include "scope_tr.h"

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


int main() {
    std::string source_code = f_read_to_string(std::string("truffle/main.tr"));
    Lexer lexer(source_code);

    std::vector<Token> tokens = {};

    try {
        while (true) {
            auto token_opt = lexer.next();
            if (!token_opt.has_value()) {
                break;
            }
            const Token& token = token_opt.value();
            tokens.push_back(token);
            std::cout << token.to_string() << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Lexer error: " << ex.what() << "\n";
        return 1;
    }

    auto errors = lexer.validate_syntax();
    if (!errors.empty()) {
        std::cerr << "Syntax errors found:\n";
        for (const auto& err : errors) {
            std::cerr << err << "\n";
        }
    } else {
        std::cout << "No syntax errors found.\n";
    }


    std::cout << "\n\n\n\n\n\n";

    VarLst var_lst = VarLst();
    FuncLst fn_lst = FuncLst();
    
    unsigned int idx = 0;
    nlohmann::json ast = parse_function(tokens, idx, &var_lst, &fn_lst);



    std::string json_res = ast.dump();
    std::cout << json_res << "\n\n";

    write_to_file("ast.json", json_res);



    return 0;
}
