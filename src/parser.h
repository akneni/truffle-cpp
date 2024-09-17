#ifndef PARSER_H
#define PARSER_H


#include "json.hpp"
#include "lexer.h"
#include "scope_tr.h"

#include <string>
#include <vector>

nlohmann::json parse_module(std::vector<Token> tokens, unsigned int &idx, VarLst* var_lst, FuncLst* fn_list);
nlohmann::json parse_code_block(std::vector<Token> tokens, unsigned int &idx, VarLst* var_lst, FuncLst* fn_list);
nlohmann::json parse_if_block(std::vector<Token> tokens, unsigned int &idx, VarLst* var_lst, FuncLst* fn_list);
nlohmann::json parse_loop(std::vector<Token> tokens, unsigned int &idx, VarLst* var_lst, FuncLst* fn_list);
nlohmann::json parse_function(std::vector<Token> tokens, unsigned int &idx, VarLst* var_lst, FuncLst* fn_list);
nlohmann::json parse_expression(std::vector<Token> tokens, unsigned int &idx, VarLst const* var_lst, FuncLst const* fn_list);
nlohmann::json parse_expression_h(std::vector<Token> tokens, unsigned int start, unsigned int end, VarLst const* var_lst, FuncLst const* fn_list);
nlohmann::json parse_function_call(std::vector<Token> tokens, unsigned int &idx, VarLst const* var_lst, FuncLst const* fn_list);
nlohmann::json parse_literal(std::vector<Token> tokens, unsigned int &idx);
nlohmann::json parse_variable(std::vector<Token> tokens, unsigned int &idx, VarLst const* var_lst, FuncLst const* fn_list);
nlohmann::json parse_declaration(std::vector<Token> tokens, unsigned int &idx, VarLst const* var_lst, FuncLst const* fn_list);
nlohmann::json parse_assignment(std::vector<Token> tokens, unsigned int &idx, VarLst const* var_lst, FuncLst const* fn_list);
nlohmann::json parse_return(std::vector<Token> tokens, unsigned int &idx, VarLst const* var_lst, FuncLst const* fn_list);

void consume_whitespace(std::vector<Token> tokens, unsigned int &idx);

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
