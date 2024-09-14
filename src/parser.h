#include "json.hpp"
#include "lexer.h"
#include <string>
#include <vector>

enum AstNodeType {
    Literal,
    Variable,
    Expression,
    DeclarationStatement,
    AssignmentStatement,
    Function,
    FunctionCall,
    Loop,
    IfBlock,
    CodeBlock,
};

struct AstNode {
    AstNodeType type;
    nlohmann::json data;

    nlohmann::json to_json() const;
};


AstNode parse_code_block(std::vector<Token> tokens, unsigned int idx);
AstNode parse_if_block(std::vector<Token> tokens, unsigned int idx);
AstNode parse_loop(std::vector<Token> tokens, unsigned int idx);
AstNode parse_function(std::vector<Token> tokens, unsigned int idx);
AstNode parse_expression(std::vector<Token> tokens, unsigned int idx);


enum DataType {
    I64,
    U64,
    U8,
    Bool,
    Char,
    String,
};

struct VariableTr {
    std::string name;
    DataType dtype;
};

struct FunctionTr {
    std::string name;
    std::vector<DataType> param_type;
    DataType ret_type;
};

struct VarLst {
    std::vector<std::vector<VariableTr>> vars;

    
    void push_stack();

    void pop_stack();

    void push_back(VariableTr v);
};

