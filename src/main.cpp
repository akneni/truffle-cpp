#include "json.hpp"
#include "lexer.h"
#include "parser.h"

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


int main() {
    std::string source_code = f_read_to_string(std::string("truffle/main.tr"));
    Lexer lexer(source_code);

    try {
        while (true) {
            auto token_opt = lexer.next();
            if (!token_opt.has_value()) {
                break;
            }
            const Token& token = token_opt.value();
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

    return 0;
}
