#include "lexer.h"
#include "parser.h"
#include "json.hpp"
#include "scope_tr.h"
#include "dtype_utils.h"

// Utils
OperationType get_op(std::string op) {
    if (op == "+") return OperationType::Add;
    if (op == "-") return OperationType::Subtract;
    if (op == "*") return OperationType::Mult;
    if (op == "/") return OperationType::Div;
    if (op == "%") return OperationType::Mod;

    if (op == ">") return OperationType::GreaterThan;
    if (op == "<") return OperationType::LessThan;
    if (op == ">=") return OperationType::GreaterThanEq;
    if (op == "<=") return OperationType::LessThanEq;
    if (op == "==") return OperationType::Eq;
    if (op == "!=") return OperationType::NotEq;

    throw std::runtime_error("Unknown operation [fn get_op]: " + op);
}

TokenType get_op_type(OperationType op) {
    if (op == OperationType::Add) return TokenType::ArithmeticOperator;
    if (op == OperationType::Subtract) return TokenType::ArithmeticOperator;
    if (op == OperationType::Mult) return TokenType::ArithmeticOperator;
    if (op == OperationType::Div) return TokenType::ArithmeticOperator;
    if (op == OperationType::Mod) return TokenType::ArithmeticOperator;

    if (op == OperationType::GreaterThan) return TokenType::ComparisonOperator;
    if (op == OperationType::LessThan) return TokenType::ComparisonOperator;
    if (op == OperationType::GreaterThanEq) return TokenType::ComparisonOperator;
    if (op == OperationType::LessThanEq) return TokenType::ComparisonOperator;
    if (op == OperationType::Eq) return TokenType::ComparisonOperator;
    if (op == OperationType::NotEq) return TokenType::ComparisonOperator;

    throw std::runtime_error("Unknown operation [fn get_op_type]: " + op);
}

/// 1 is the lowest priority and 255 is the highest
unsigned int get_op_priority(OperationType op) {
    if (op == OperationType::Add) return 10;
    if (op == OperationType::Subtract) return 10;
    if (op == OperationType::Mult) return 11;
    if (op == OperationType::Div) return 11;
    if (op == OperationType::Mod) return 11;

    if (op == OperationType::GreaterThan) return 9;
    if (op == OperationType::LessThan) return 9;
    if (op == OperationType::GreaterThanEq) return 9;
    if (op == OperationType::LessThanEq) return 9;
    if (op == OperationType::Eq) return 9;
    if (op == OperationType::NotEq) return 9;

    throw std::runtime_error("Unknown operation [fn get_op_priority]: " + op);
}

void consume_whitespace(std::vector<Token> tokens, unsigned int &idx) {
    while (tokens[idx].token_type == TokenType::NewLine) {
        idx++;
    }
}
// -------------------


/// ## Module
/// ```json
/// {
///     "type": "Module",
///     "statements": [] // These can be any node specified below
/// }
/// ```
nlohmann::json parse_module(
    std::vector<Token> tokens, 
    unsigned int &idx,
    VarLst* var_lst,
    FuncLst* fn_list
) {
    var_lst->push_stack();
    fn_list->push_stack();

    nlohmann::json module_node;
    module_node["type"] = "Module";

    std::vector<nlohmann::json> statements = {};

    while (idx < tokens.size()) {
        consume_whitespace(tokens, idx);

        std::cout << tokens[idx].to_string() << "\n";

        if (tokens[idx].token_type == TokenType::Keyword) {
            if (tokens[idx].value == "fn") {
                statements.push_back(parse_function(tokens, idx, var_lst, fn_list));
            }
            else {
                throw std::runtime_error("invalid keyword " + tokens[idx].value);
            }
        }
        else if (tokens[idx].token_type == TokenType::DataType) {
            statements.push_back(parse_declaration(tokens, idx, var_lst, fn_list));
        }
        else if (tokens[idx].token_type == TokenType::CloseCurlyBrace) {
            idx++;
        }
        else {
            throw std::runtime_error("[fn parse_module] Invalid start token -> " + tokens[idx].value);
        }
    }

    module_node["statements"] = statements;

    var_lst->pop_stack();
    fn_list->pop_stack();
    return module_node;
}


/// ## CodeBlock
/// ```json
/// {
///     "type": "CodeBlock",
///     "statements": [] // These can be any node specified below
/// }
/// ```
nlohmann::json parse_code_block(
    std::vector<Token> tokens, 
    unsigned int &idx,
    VarLst* var_lst,
    FuncLst* fn_list
) {
    var_lst->push_stack();
    fn_list->push_stack();

    if (tokens[idx].token_type != TokenType::OpenCurlyBrace) {
        throw std::runtime_error("[fn parse_code_block] called while tokens doesn't start with a curly brace.\n" + tokens[idx].to_string());
    }
    std::vector<nlohmann::json> code_block = {};

    nlohmann::json node;
    node["type"] = "CodeBlock";

    idx++;

    // while (true) {
    for (int i = 0; i < 12; i++) {
        if (tokens[idx].token_type == TokenType::NewLine) {
            idx++;
            continue;
        }
        else if (tokens[idx].token_type == TokenType::CloseCurlyBrace) {
            break;
        }

        if (tokens[idx].equals(TokenType::Keyword)) {
            if (tokens[idx].value == "fn") {
                fn_list->push_back(FunctionTr {
                    .name = tokens[idx+1].value,
                    .param_type = {},
                    .ret_type = BeDataType::Null,
                });

                nlohmann::json fn_block = parse_function(tokens, idx, var_lst, fn_list);
                code_block.push_back(fn_block);
            }
            else if (tokens[idx].equals("if"))  {
                nlohmann::json if_block = parse_if_block(tokens, idx, var_lst, fn_list);
                code_block.push_back(if_block);
            }
            else if (tokens[idx].equals("while"))  {
                nlohmann::json loop_block = parse_loop(tokens, idx, var_lst, fn_list);
                code_block.push_back(loop_block);
            }
            else if (tokens[idx].equals("return")) {
                nlohmann::json ret_statement = parse_return(tokens, idx, var_lst, fn_list);
                code_block.push_back(ret_statement);
            }
            else {
                throw std::runtime_error("[fn parse_code_block] Keyword " + tokens[idx].value + " not supported");
            }
        }
        else if (tokens[idx].token_type == TokenType::DataType) {
            if (tokens[idx+1].token_type != TokenType::Object) {
                throw std::runtime_error("type without variable declaration");
            }
            var_lst->push_back(VariableTr{
                .name = tokens[idx+1].value,
                .dtype = dtype_from_str(tokens[idx].value)
            });
            code_block.push_back(parse_declaration(tokens, idx, var_lst, fn_list));
        }
        else if (tokens[idx].token_type == TokenType::Object) {
            if (fn_list->contains(tokens[idx].value)) {
                code_block.push_back(parse_function_call(tokens, idx, var_lst, fn_list));
            }
            else if (var_lst->contains(tokens[idx].value)) {
                code_block.push_back(parse_assignment(tokens, idx, var_lst, fn_list));
            }
            else {
                throw std::runtime_error("Object `" + tokens[idx].value + "` is undefined");
            }
        }
    }

    node["statements"] = code_block;

    var_lst->pop_stack();
    fn_list->pop_stack();
    return node;
}

/// ## Function
/// ```json
/// {
///     "type": "Function",
///     "parameters": ["DataType", "DataType", ...],
///     "ret-type": "DataType",
///     "code-block": {"type": "CodeBlock", ...}
/// }
/// ```
nlohmann::json parse_function(
    std::vector<Token> tokens, 
    unsigned int &idx,
    VarLst* var_lst,
    FuncLst* fn_list
) {
    var_lst->push_stack();
    fn_list->push_stack();

    // Check for 'fn' keyword
    if (tokens[idx].token_type != TokenType::Keyword || tokens[idx].value != "fn") {
        throw std::runtime_error("[fn parse_function] Expected 'fn' keyword at the beginning of function definition.");
    }
    idx++;  // Move to function name

    nlohmann::json func;
    func["type"] = "Function";

    // Expect function name
    if (tokens[idx].token_type != TokenType::Object) {
        throw std::runtime_error("[fn parse_function] Expected function name after 'fn' keyword.");
    }
    std::string function_name = tokens[idx].value;
    func["name"] = function_name;
    idx++;  // Move to '('

    fn_list->funcs[fn_list->funcs.size()-2].push_back(FunctionTr {
        .name = function_name,
        .param_type = {},
        .ret_type = BeDataType::Null,
    });

    // Expect '('
    if (tokens[idx].token_type != TokenType::OpenParen) {
        throw std::runtime_error("[fn parse_function] Expected '(' after function name.");
    }
    idx++;  // Move to parameters or ')'

    // Parse parameters
    std::vector<nlohmann::json> params = {};

    while (tokens[idx].token_type != TokenType::CloseParen) {
        // Skip commas
        if (tokens[idx].token_type == TokenType::Comma) {
            idx++;
            continue;
        }

        // Expect DataType
        if (tokens[idx].token_type != TokenType::DataType) {
            throw std::runtime_error("[fn parse_function] Expected data type in parameter list.");
        }
        std::string param_dtype_str = tokens[idx].value;
        BeDataType param_dtype = dtype_from_str(param_dtype_str);
        std::string param_dtype_json = dtype_to_str(param_dtype);
        idx++;  // Move to parameter name

        // Expect parameter name
        if (tokens[idx].token_type != TokenType::Object) {
            throw std::runtime_error("[fn parse_function] Expected parameter name after data type.");
        }
        std::string param_name = tokens[idx].value;
        idx++;  // Move to ',' or ')'

        // Store parameter as an object with name and dtype
        nlohmann::json param_obj;
        param_obj["name"] = param_name;
        param_obj["dtype"] = param_dtype_json;
        params.push_back(param_obj);
    }

    func["parameters"] = params;

    // Now tokens[idx] should be ')'
    if (tokens[idx].token_type != TokenType::CloseParen) {
        throw std::runtime_error("[fn parse_function] Expected ')' after parameters list.");
    }
    idx++;  // Move past ')'

    // Check for return type
    if (tokens[idx].token_type == TokenType::DataType) {
        std::string ret_type_str = tokens[idx].value;
        BeDataType ret_type = dtype_from_str(ret_type_str);
        func["ret-type"] = dtype_to_str(ret_type);
        idx++;  // Move to '{'
    } else {
        // Default return type is 'Null' if not specified
        func["ret-type"] = "Null";
    }

    // Parse the function body using parse_code_block
    nlohmann::json code_block = parse_code_block(tokens, idx, var_lst, fn_list);
    func["code-block"] = code_block;

    var_lst->pop_stack();
    fn_list->pop_stack();
    return func;
}


/// ## IfBlock
/// ```json
/// {
///     "type": "IfBlock",
///     "statements": [
///         {
///             "condition": "...",
///             "code-block": {"type": "CodeBlock", ...},
///         }
///     ],
///     "default": {"type": "CodeBlock", ...} 
/// }
/// ```
nlohmann::json parse_if_block(
    std::vector<Token> tokens, 
    unsigned int &idx,
    VarLst* var_lst,
    FuncLst* fn_list
) {
    var_lst->push_stack();
    fn_list->push_stack();

    nlohmann::json if_block;
    if_block["type"] = "IfBlock";

    std::vector<nlohmann::json> statements = {};

    while (true) {
        if (
            tokens[idx].token_type != TokenType::Keyword ||
            tokens[idx].value != "if"
        ) {
            if (tokens[idx].value == "else") {
                break;
            }
            throw std::runtime_error("[fn parse_if_block] invalid start token: " + tokens[idx].value);
        }
        idx++;

        nlohmann::json statement;
        statement["condition"] = parse_expression(tokens, idx, var_lst, fn_list);

        if (tokens[idx].token_type != TokenType::OpenCurlyBrace) {
            throw std::runtime_error("[fn parse_if_block] invalid token after conditional expression: " + tokens[idx].value);
        }

        statement["code-block"] = parse_code_block(tokens, idx, var_lst, fn_list);

        statements.push_back(statement);
    }

    if_block["statements"] = statements;

    consume_whitespace(tokens, idx);
    if (tokens[idx].token_type == TokenType::Keyword && tokens[idx].value == "else") {
        idx++;
        if_block["default"] = parse_code_block(tokens, idx, var_lst, fn_list);
    }

    var_lst->pop_stack();
    fn_list->pop_stack();
    return if_block;
}

/// ## Loop
/// ```json
/// {
///     "type": "Loop",
///     "condition": "...",
///     "code-block": {"type": "CodeBlock", ...} 
/// }
/// ```
nlohmann::json parse_loop(
    std::vector<Token> tokens, 
    unsigned int &idx,
    VarLst* var_lst,
    FuncLst* fn_list
) {
    var_lst->push_stack();
    fn_list->push_stack();

    if (tokens[idx].token_type != TokenType::Keyword || tokens[idx].value != "while") {
        throw std::runtime_error("[fn parse_loop] error parsing, tokens does not start with `while`.");
    }
    idx++;

    nlohmann::json loop_block;
    loop_block["type"] = "Loop";
    loop_block["condition"] = parse_expression(tokens, idx, var_lst, fn_list);

    if (tokens[idx].token_type != TokenType::OpenCurlyBrace) {
        throw std::runtime_error("[fn parse_loop] error parsing, conditional expression not followed by `{`");
    }
    idx++;

    loop_block["code-block"] = parse_code_block(tokens, idx, var_lst, fn_list);

    var_lst->pop_stack();
    fn_list->pop_stack();
    return loop_block;
}

/// ## FunctionCall
/// ```json
/// {
///     "type": "FunctionCall",
///     "function-name": "...",
///     "parameters": ["..."],
///     "dtype": "DataType"
/// }
/// ```
nlohmann::json parse_function_call(
    std::vector<Token> tokens, 
    unsigned int &idx,
    VarLst const* var_lst,
    FuncLst const* fn_list
) {
    if (tokens[idx].token_type != TokenType::Object) {
        throw std::runtime_error("[fn parse_function_call] error parsing, tokens do not start TokenType::Object");
    }

    nlohmann::json func;
    func["type"] = "FunctionCall";
    func["function-name"] = tokens[idx].value;
    
    idx++;
    if (tokens[idx].token_type != TokenType::OpenParen) {
        throw std::runtime_error("[fn parse_function_call] Expected `(` after function name.");
    }
    idx++;

    std::vector<nlohmann::json> arguments = {};
    while (tokens[idx].token_type != TokenType::CloseParen) {
        if (tokens[idx].token_type == TokenType::NewLine) {
            idx++;
            continue;
        }

        arguments.push_back(parse_expression(tokens, idx, var_lst, fn_list));
    }

    idx++;

    func["parameters"] = arguments;
    func["dtype"] = "Null (TODO)";

    return func;
}

/// ## Expression
/// ```json
/// {
///     "type": "Expression",
///     "left-operand": "...",
///     "operator": "...",
///     "right-operand": "...",
///     "dtype": "DataType"
/// }
/// ```
nlohmann::json parse_expression(
    std::vector<Token> tokens, 
    unsigned int &idx,
    VarLst const* var_lst,
    FuncLst const* fn_list
) {
    unsigned int num_paren = 0;
    bool num_paren_on = false;

    unsigned int expr_end_idx = idx;

    for (unsigned int i = idx; i < tokens.size(); i++) {
        if (
            tokens[i].token_type == TokenType::CloseCurlyBrace ||
            tokens[i].token_type == TokenType::CloseSquareBracket ||
            tokens[i].token_type == TokenType::SemiColon ||
            tokens[i].token_type == TokenType::Comma || 
            tokens[i].token_type == TokenType::NewLine
        ) {
            expr_end_idx = i;
            break;
        }
        else if (tokens[i].token_type == TokenType::OpenParen) {
            num_paren++;
            num_paren_on = true;
        }
        else if (tokens[i].token_type == TokenType::CloseParen) {
            if (num_paren_on) {
                if (num_paren == 0) {
                    expr_end_idx = i;
                    break;
                }
                else {
                    num_paren--;
                }
            }
            else {
                expr_end_idx = i;
                break;
            }
        }
    }

    if (idx == expr_end_idx) {
        throw std::runtime_error("idx is equal expr_end_idx");
    }
    else if (idx == expr_end_idx + 1) {
        if (tokens[idx].token_type == TokenType::Object) {
            return parse_variable(tokens, idx, var_lst, fn_list);
        }
        else if (is_literal(tokens[idx].token_type)) {
            return parse_literal(tokens, idx);
        }
    }

    
    nlohmann::json op;
    op["type"] = "Expression";

    unsigned int op_idx = 0;
    unsigned int op_priority = 0;

    for (unsigned int i = idx; i < expr_end_idx; i++) {
        if (
            tokens[i].token_type != TokenType::ArithmeticOperator && 
            tokens[i].token_type != TokenType::ComparisonOperator
        ) {
            continue;
        }

        unsigned int curr_priority = get_op_priority(get_op(tokens[i].value));
        if (curr_priority > op_priority) {
            op_priority = curr_priority;
            op_idx = i;
        }
    }

    if (op_priority == 0) {
        // No operator found in the expression
        // Check if the expression is just a literal or variable
        if (expr_end_idx - idx == 1) {
            if (tokens[idx].token_type == TokenType::Object) {
                return parse_variable(tokens, idx, var_lst, fn_list);
            }
            else if (is_literal(tokens[idx].token_type)) {
                return parse_literal(tokens, idx);
            }
        }
        throw std::runtime_error("[fn parse_expression] No operation found");
    }


    op["operator"] = tokens[op_idx].value;

    op["left-operand"] = parse_expression_h(tokens, idx, op_idx-1, var_lst, fn_list);
    op["right-operand"] = parse_expression_h(tokens, op_idx+1, expr_end_idx-1, var_lst, fn_list);
    op["dtype"] = "Null (TODO)";

    idx = expr_end_idx;
    return op;
}

nlohmann::json parse_expression_h(
    std::vector<Token> tokens, 
    unsigned int start, 
    unsigned int end,
    VarLst const* var_lst,
    FuncLst const* fn_list
) {
    if (end == start) {
        if (is_literal(tokens[start].token_type)) {
            return parse_literal(tokens, start);
        }
        else if (tokens[start].token_type == TokenType::Object) {
            return parse_variable(tokens, start, var_lst, fn_list);
        }
        else if (tokens[start].token_type == TokenType::Object) {
            // Parse expression needs to be fixed
            return parse_function_call(tokens, start, var_lst, fn_list);
        }
    }
    else if (end < start) {
        throw std::runtime_error("[fn parse_expression_h] end is less than start.");
    }

    nlohmann::json op;
    op["type"] = "Expression";

    unsigned int op_idx = 0;
    unsigned int op_priority = 0;

    for (unsigned int i = start; i <=end; i++) {
        if (
            tokens[i].token_type != TokenType::ArithmeticOperator && 
            tokens[i].token_type != TokenType::ComparisonOperator
        ) {
            continue;
        }

        unsigned int curr_priority = get_op_priority(get_op(tokens[i].value));
        if (curr_priority > op_priority) {
            op_priority = curr_priority;
            op_idx = i;
        }
    }

    if (op_priority == 0) {
        throw std::runtime_error("[fn parse_expression_h] No operation found");
    }

    op["operator"] = tokens[op_idx].value;

    op["left-operand"] = parse_expression_h(tokens, start, op_idx-1, var_lst, fn_list);
    op["right-operand"] = parse_expression_h(tokens, op_idx+1, end, var_lst, fn_list);
    op["dtype"] = "Null (TODO)";

    return op;
}

/// ## DeclarationStatement
/// ```json
/// {
///     "type": "DeclarationStatement",
///     "dst": "some variable",
///     "src": "..."
/// }
/// ```
nlohmann::json parse_declaration(
    std::vector<Token> tokens, 
    unsigned int &idx,
    VarLst const* var_lst,
    FuncLst const* fn_list
) {
    bool auto_dtype = false;

    if (tokens[idx].token_type == TokenType::DataType) {
        // nothing, keep auto_dtype equal to false
    }
    else if (
        tokens[idx].token_type == TokenType::Object &&
        tokens[idx+1].token_type == TokenType::AssignmentOperator &&
        tokens[idx+1].value == ":="
    ) {
        auto_dtype == true;
    }
    else {
        throw std::runtime_error("[fn parse_declaration] called at invalid start");
    }

    BeDataType dtype = BeDataType::Null;
    if (!auto_dtype) {
        dtype = dtype_from_str(tokens[idx].value);
    }

    std::string v_name = "";
    if (auto_dtype) {
        v_name = tokens[idx].value;
        idx += 2;
    }
    else {
        v_name = tokens[idx+1].value;
        idx += 3;
    }

    nlohmann::json expr = parse_expression(tokens, idx, var_lst, fn_list);

    BeDataType expr_dtype = dtype_from_str(expr["dtype"]);
    if (auto_dtype) {
        dtype = expr_dtype;
    }
    else {
        if (!dtypes_check_valid(dtype, expr_dtype)) {
            throw std::runtime_error("error constructing declaration statement: mismatched types.");
        }
    }

    nlohmann::json decl_statement;
    decl_statement["type"] = "DeclarationStatement";

    decl_statement["dst"] = v_name;
    decl_statement["src"] = expr;
    decl_statement["dtype"] = dtype_to_str(dtype);
    return decl_statement;
}

/// ## AssignmentStatement
/// ```json
/// {
///     "type": "AssignmentStatement",
///     "dst": "some variable",
///     "src": "..."
/// }
/// ```
nlohmann::json parse_assignment(
    std::vector<Token> tokens, 
    unsigned int &idx,
    VarLst const* var_lst,
    FuncLst const* fn_list
) {
    if (tokens[idx].token_type != TokenType::Object) {
        throw std::runtime_error("[fn parse_assignment] tokens does not start with object");
    }

    nlohmann::json assignment_s;
    assignment_s["type"] = "AssignmentStatement";
    assignment_s["dst"] = tokens[idx].value;
    
    if (tokens[idx+1].token_type != TokenType::AssignmentOperator) {
        throw std::runtime_error("[fn parse_assignment] no assignment operator after variable");
    }
    idx += 2;
    assignment_s["src"] = parse_expression(tokens, idx, var_lst, fn_list);

    return assignment_s;
}

/// ## ReturnStatement
/// ```json
/// {
///     "type": "ReturnStatement",
///     "value": "...",
///     "dtype": "DataType"
/// }
/// ```
nlohmann::json parse_return(
    std::vector<Token> tokens, 
    unsigned int &idx,
    VarLst const* var_lst,
    FuncLst const* fn_list
) {
    if (!tokens[idx].equals(TokenType::Keyword, "return")) {
        throw std::runtime_error("[fn parse_return] token does not start with the return keyword");
    }

    idx++;

    nlohmann::json ret_statement;
    ret_statement["type"] = "ReturnStatement";
    ret_statement["value"] = parse_expression(tokens, idx, var_lst, fn_list);
    ret_statement["dtype"] = "Null (TODO)";

    return ret_statement;
}


/// ## Literal
/// ```json
/// {
///     "type": "Literal",
///     "value": "...",
///     "dtype": "DataType"
/// }
/// ```
nlohmann::json parse_literal(std::vector<Token> tokens, unsigned int &idx) {
    BeDataType dtype = BeDataType::Null;
    if (tokens[idx].token_type == TokenType::IntegerLiteral) {
        dtype = BeDataType::I64;
    }
    else if (tokens[idx].token_type == TokenType::FloatLiteral) {
        dtype = BeDataType::F64;
    }
    else if (tokens[idx].token_type == TokenType::StringLiteral) {
        dtype = BeDataType::String;
    }
    else if (tokens[idx].token_type == TokenType::BooleanLiteral) {
        dtype = BeDataType::Bool;
    }
    else {
        throw std::runtime_error("error parsing literal: token not a literal.");
    }

    nlohmann::json lit;
    lit["type"] = "Literal";
    lit["dtype"] = dtype_to_str(dtype);
    lit["value"] = tokens[idx].value;
    idx++;
    return lit;
}

/// ## Variable
/// ```json
/// {
///     "type": "Variable",
///     "name": "...",
///     "dtype": "DataType"
/// }
/// ```
nlohmann::json parse_variable(
    std::vector<Token> tokens, 
    unsigned int &idx,
    VarLst const* var_lst,
    FuncLst const* fn_list
) {
    if (tokens[idx].token_type != TokenType::Object) {
        throw std::runtime_error("error parsing variable: token is not an object.");
    }

    std::optional<VariableTr> v = var_lst->get(tokens[idx].value);

    if (!v.has_value()) {
        throw std::runtime_error("[fn parse_variable] variable DNE");
    }

    nlohmann::json var;
    var["type"] = "Variable";
    var["dtype"] = dtype_to_str(v.value().dtype);
    var["name"] = tokens[idx].value;
    idx++;
    return var;
}