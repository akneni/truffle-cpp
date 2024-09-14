#include "parser.h"
#include "json.hpp"

nlohmann::json AstNode::to_json() const {
    nlohmann::json j = data;
    
    AstNodeType node_type = type;
    std::string type_str;

    switch (node_type) {
        case AstNodeType::Literal: type_str = "Literal"; break;
        case AstNodeType::Variable: type_str = "Variable"; break;
        case AstNodeType::Expression: type_str = "Expression"; break;
        case AstNodeType::DeclarationStatement: type_str = "DeclarationStatement"; break;
        case AstNodeType::AssignmentStatement: type_str = "AssignmentStatement"; break;
        case AstNodeType::Function: type_str = "Function"; break;
        case AstNodeType::FunctionCall: type_str = "FunctionCall"; break;
        case AstNodeType::Loop: type_str = "Loop"; break;
        case AstNodeType::IfBlock: type_str = "IfBlock"; break;
        case AstNodeType::CodeBlock: type_str = "CodeBlock"; break;
        default: type_str = "UNKNOWN";
    }

    j["type"] = type_str;

    return j;
}


AstNode parse_code_block(std::vector<Token> tokens, unsigned int idx) {
    if (tokens[0].token_type != TokenType::OpenCurlyBrace) {
        throw std::runtime_error("[fn parse_code_block] called while tokens doesn't start with a curly brace.");
    }


}

AstNode parse_if_block(std::vector<Token> tokens, unsigned int idx) {

}

AstNode parse_loop(std::vector<Token> tokens, unsigned int idx) {

}

AstNode parse_function(std::vector<Token> tokens, unsigned int idx) {

}

AstNode parse_expression(std::vector<Token> tokens, unsigned int idx) {

}
