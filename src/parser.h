#ifndef PARSER_H
#define PARSER_H


#include "json.hpp"
#include "lexer.h"
#include <string>
#include <vector>

nlohmann::json parse_code_block(std::vector<Token> tokens, unsigned int &idx);
nlohmann::json parse_if_block(std::vector<Token> tokens, unsigned int &idx);
nlohmann::json parse_loop(std::vector<Token> tokens, unsigned int &idx);
nlohmann::json parse_function(std::vector<Token> tokens, unsigned int &idx);
nlohmann::json parse_expression(std::vector<Token> tokens, unsigned int &idx);
nlohmann::json parse_expression_h(std::vector<Token> tokens, unsigned int start, unsigned int end);
nlohmann::json parse_literal(std::vector<Token> tokens, unsigned int &idx);
nlohmann::json parse_variable(std::vector<Token> tokens, unsigned int &idx);
nlohmann::json parse_declaration(std::vector<Token> tokens, unsigned int &idx);
nlohmann::json parse_assignment(std::vector<Token> tokens, unsigned int &idx);


enum DataType {
    I64,
    U64,
    U8,
    F64,
    Bool,
    Char,
    String,
    Null,
};

DataType dtype_from_str(std::string s);
std::string dtype_to_str(DataType dtype);
bool dtypes_check_valid(DataType actual, DataType inferenced);


enum OperationType {
    Add,
    Subtract,
    Mult,
    Div,
    Mod,
    GreaterThan,
    LessThan,
    GreaterThanEq,
    LessThanEq,
    Eq,
    NotEq,
};

TokenType get_op_type(OperationType op);
unsigned int get_op_priority(OperationType op);

#endif // PARSER_H
