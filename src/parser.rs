// parser.rs
use std::{collections::{HashMap, HashSet}, default, fmt::Debug};

use anyhow::{Result, anyhow};
use serde::{Deserialize, Serialize};
use serde_json::Value;

use crate::{lexer::{Token, TokenType}, utils::{FnLst, VarLst}};

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub enum DataType {
    I64,
    U64,
    F64,
    U8,
    Bool,
    Char,
    String,
    Null,
}

impl DataType {
    fn new(dt: &str) -> Self {
        match dt {
            "int" => DataType::I64,
            "uint" => DataType::U64,
            "float" => DataType::F64,
            "bool" => DataType::Bool,
            "char" => DataType::Char,
            "byte" => DataType::U8,
            "string" => DataType::String,
            "null" => DataType::Null,
            "I64" => DataType::I64,
            "U64" => DataType::U64,
            "F64" => DataType::F64,
            "Bool" => DataType::Bool,
            "Char" => DataType::Char,
            "U8" => DataType::U8,
            "String" => DataType::String,
            "Null" => DataType::Null,
            _ => panic!("No data type found for `{}`", dt),
        }
    }

    fn to_string(&self) -> String {
        match &self {
            DataType::I64 => "I64".to_string(),
            DataType::U64 => "U64".to_string(),
            DataType::F64 => "F64".to_string(),
            DataType::Bool => "Bool".to_string(),
            DataType::Char => "Char".to_string(),
            DataType::U8 => "U8".to_string(),
            DataType::String => "String".to_string(),
            DataType::Null => "Null".to_string(),
        }
    }

    fn is_numeric(&self) -> bool {
        matches!(self, Self::I64 | Self::U64 | Self::F64 | Self::U8)
    }
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum OperationType {
    Add,
    Subtract,
    Div,
    Mult,
    Mod,
    GreaterThan,
    LessThan,
    GreaterThanOrEq,
    LessThanOrEq,
    Eq,
    NotEq,
}

impl OperationType {
    fn new(op: &str) -> Result<Self> {
        match op {
            "+" => Ok(OperationType::Add),
            "-" => Ok(OperationType::Subtract),
            "*" => Ok(OperationType::Mult),
            "/" => Ok(OperationType::Div),
            "%" => Ok(OperationType::Mod),
            ">" => Ok(OperationType::GreaterThan),
            "<" => Ok(OperationType::LessThan),
            ">=" => Ok(OperationType::GreaterThanOrEq),
            "<=" => Ok(OperationType::LessThanOrEq),
            "==" => Ok(OperationType::Eq),
            "!=" => Ok(OperationType::NotEq),
            _ => Err(anyhow!("Unknown operation [fn get_op]: {}", op)),
        }
    }

    /// Returns priority: higher number means higher priority
    fn get_priority(&self) -> usize {
        match self {
            OperationType::Mult | OperationType::Div | OperationType::Mod => 11,
            OperationType::Add | OperationType::Subtract => 10,
            OperationType::GreaterThan
            | OperationType::LessThan
            | OperationType::GreaterThanOrEq
            | OperationType::LessThanOrEq
            | OperationType::Eq
            | OperationType::NotEq => 9,
        }
    }
}

fn is_literal(token_type: &TokenType) -> bool {
    matches!(
        token_type,
        TokenType::IntegerLiteral |
        TokenType::FloatLiteral |
        TokenType::StringLiteral |
        TokenType::BooleanLiteral
    )
}

fn inference_type(left: DataType, right: DataType, op: &str) -> Result<DataType> {
    match op {
        "+" => match (left, right) {
            (DataType::I64, DataType::I64) => Ok(DataType::I64),
            (DataType::I64, DataType::F64)
            | (DataType::F64, DataType::I64)
            | (DataType::F64, DataType::F64) => Ok(DataType::F64),
            (DataType::String, DataType::String) => Ok(DataType::String),
            _ => Err(anyhow!("[fn inference_type] invalid operations")),
        },
        "-" | "*" => match (left, right) {
            (DataType::I64, DataType::I64) => Ok(DataType::I64),
            (DataType::I64, DataType::F64)
            | (DataType::F64, DataType::I64)
            | (DataType::F64, DataType::F64) => Ok(DataType::F64),
            _ => Err(anyhow!("[fn inference_type] invalid operations")),
        },
        "/" => match (left, right) {
            (DataType::I64, DataType::I64)
            | (DataType::I64, DataType::F64)
            | (DataType::F64, DataType::I64)
            | (DataType::F64, DataType::F64) => Ok(DataType::F64),
            _ => Err(anyhow!("[fn inference_type] invalid operations")),
        },
        "%" => match (left, right) {
            (DataType::I64, DataType::I64) => Ok(DataType::I64),
            _ => Err(anyhow!("[fn inference_type] invalid operations")),
        },
        ">" | ">=" | "<" | "<=" | "==" | "!=" => match (left, right) {
            (l, r) if l == r => Ok(DataType::Bool),
            (DataType::I64, DataType::F64) | (DataType::F64, DataType::I64) => Ok(DataType::Bool),
            _ => Err(anyhow!("[fn inference_type] invalid comparison")),
        },
        _ => Err(anyhow!("[fn inference_type] invalid operations")),
    }
}

pub fn parse_module(
    tokens: &[Token],
    idx: &mut usize,
    var_lst: &mut VarLst,
    fn_list: &mut FnLst,
) -> Result<Value> {
    var_lst.push_scope();
    fn_list.push_scope();

    let mut module_node = serde_json::json!({
        "type": "Module",
        "statements": []
    });

    let mut statements = vec![];

    while *idx < tokens.len() {
        consume_whitespace(tokens, idx);

        if tokens[*idx].token_type == TokenType::Keyword {
            if tokens[*idx].value == "fn" {
                fn_list.insert(tokens[*idx + 1].value.to_string(), DataType::Null, vec![]);

                let func_node = parse_function(tokens, idx, var_lst, fn_list)?;
                statements.push(func_node);
            } else {
                return Err(anyhow!("invalid keyword {}", tokens[*idx].value));
            }
        } else if tokens[*idx].token_type == TokenType::DataType {
            let decl_node = parse_declaration(tokens, idx, var_lst, fn_list)?;
            statements.push(decl_node);
        } else if tokens[*idx].token_type == TokenType::CloseCurlyBrace {
            *idx += 1;
        } else {
            return Err(anyhow!(
                "[fn parse_module] Invalid start token -> {}",
                tokens[*idx].value
            ));
        }
    }

    module_node["statements"] = serde_json::Value::Array(statements);

    var_lst.pop_scope();
    fn_list.pop_scope();
    Ok(module_node)
}

fn parse_code_block(
    tokens: &[Token],
    idx: &mut usize,
    var_lst: &mut VarLst,
    fn_list: &mut FnLst,
) -> Result<Value> {
    var_lst.push_scope();
    fn_list.push_scope();

    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::OpenCurlyBrace) {
        return Err(anyhow!(
            "[fn parse_code_block] called while tokens doesn't start with a curly brace."
        ));
    }

    let mut code_block = serde_json::json!({
        "type": "CodeBlock",
        "statements": []
    });

    *idx += 1;

    let mut statements = vec![];

    loop {
        if *idx >= tokens.len() {
            break;
        }

        if tokens[*idx].token_type == TokenType::NewLine {
            *idx += 1;
            continue;
        } else if tokens[*idx].token_type == TokenType::CloseCurlyBrace {
            *idx += 1;
            break;
        }

        if tokens[*idx].token_type == TokenType::Keyword {
            match tokens[*idx].value {
                "fn" => {
                    let func_node = parse_function(tokens, idx, var_lst, fn_list)?;
                    statements.push(func_node);
                }
                "if" => {
                    let if_block = parse_if_block(tokens, idx, var_lst, fn_list)?;
                    statements.push(if_block);
                }
                "while" => {
                    let loop_block = parse_loop(tokens, idx, var_lst, fn_list)?;
                    statements.push(loop_block);
                }
                "return" => {
                    let ret_statement = parse_return(tokens, idx, var_lst, fn_list)?;
                    statements.push(ret_statement);
                }
                _ => {
                    return Err(anyhow!(
                        "[fn parse_code_block] Keyword {} not supported",
                        tokens[*idx].value
                    ));
                }
            }
        } else if tokens[*idx].token_type == TokenType::DataType {
            if tokens.get(*idx + 1).map_or(true, |t| t.token_type != TokenType::Object) {
                return Err(anyhow!("type without variable declaration"));
            }
            let var_name = tokens[*idx + 1].value.to_string();
            let dtype = DataType::new(tokens[*idx].value);
            var_lst.insert(var_name.clone(), dtype);

            let decl_node = parse_declaration(tokens, idx, var_lst, fn_list)?;
            statements.push(decl_node);
        } else if tokens[*idx].token_type == TokenType::Object {
            if fn_list.get(tokens[*idx].value).is_some() {
                let func_call = parse_function_call(tokens, idx, var_lst, fn_list)?;
                statements.push(func_call);
            } else if var_lst.get(tokens[*idx].value).is_some() {
                let assignment = parse_assignment(tokens, idx, var_lst, fn_list)?;
                statements.push(assignment);
            } else {
                return Err(anyhow!("Object `{}` is undefined", tokens[*idx].value));
            }
        } else {
            return Err(anyhow!(
                "no valid parsing strategy in [fn parse_code_block] for {:?}",
                tokens[*idx]
            ));
        }
    }

    code_block["statements"] = serde_json::Value::Array(statements);

    var_lst.pop_scope();
    fn_list.pop_scope();

    Ok(code_block)
}

fn parse_function(
    tokens: &[Token],
    idx: &mut usize,
    var_lst: &mut VarLst,
    fn_list: &mut FnLst,
) -> Result<Value> {
    var_lst.push_scope();
    fn_list.push_scope();

    // Check for 'fn' keyword
    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::Keyword || t.value != "fn") {
        return Err(anyhow!(
            "[fn parse_function] Expected 'fn' keyword at the beginning of function definition."
        ));
    }
    *idx += 1; // Move to function name

    let mut func = serde_json::json!({ "type": "Function" });

    // Expect function name
    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::Object) {
        return Err(anyhow!(
            "[fn parse_function] Expected function name after 'fn' keyword."
        ));
    }
    let function_name = tokens[*idx].value.to_string();
    func["name"] = serde_json::Value::String(function_name.clone());
    *idx += 1; // Move to '('

    // Expect '('
    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::OpenParen) {
        return Err(anyhow!(
            "[fn parse_function] Expected '(' after function name."
        ));
    }
    *idx += 1; // Move to parameters or ')'

    // Parse parameters
    let mut params = vec![];
    let mut param_types = vec![];

    while tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::CloseParen) {
        // Skip commas
        if tokens[*idx].token_type == TokenType::Comma {
            *idx += 1;
            continue;
        }

        // Expect DataType
        if tokens[*idx].token_type != TokenType::DataType {
            return Err(anyhow!(
                "[fn parse_function] Expected data type in parameter list."
            ));
        }
        let param_dtype_str = tokens[*idx].value;
        let param_dtype = DataType::new(param_dtype_str);
        let param_dtype_json = param_dtype.to_string();
        *idx += 1; // Move to parameter name

        // Expect parameter name
        if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::Object) {
            return Err(anyhow!(
                "[fn parse_function] Expected parameter name after data type."
            ));
        }
        let param_name = tokens[*idx].value.to_string();
        var_lst.insert(param_name.clone(), param_dtype.clone());
        *idx += 1; // Move to ',' or ')'

        // Store parameter as an object with name and dtype
        let param_obj = serde_json::json!({
            "name": param_name.clone(),
            "dtype": param_dtype_json
        });
        params.push(param_obj);
        param_types.push((param_name, param_dtype));
    }

    func["parameters"] = serde_json::Value::Array(params);

    // Now tokens[idx] should be ')'
    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::CloseParen) {
        return Err(anyhow!(
            "[fn parse_function] Expected ')' after parameters list."
        ));
    }
    *idx += 1; // Move past ')'

    // Check for return type
    let ret_type = if tokens.get(*idx).map_or(false, |t| t.token_type == TokenType::DataType) {
        let ret_type_str = tokens[*idx].value;
        let ret_type = DataType::new(ret_type_str);
        *idx += 1; // Move to '{'
        ret_type
    } else {
        // Default return type is 'Null' if not specified
        DataType::Null
    };
    func["ret-type"] = serde_json::Value::String(ret_type.to_string());

    // Register the function in the function list
    fn_list.insert(function_name.clone(), ret_type.clone(), param_types);

    // Parse the function body using parse_code_block
    let code_block = parse_code_block(tokens, idx, var_lst, fn_list)?;
    func["code-block"] = code_block;

    var_lst.pop_scope();
    fn_list.pop_scope();
    Ok(func)
}

fn parse_if_block(
    tokens: &[Token],
    idx: &mut usize,
    var_lst: &mut VarLst,
    fn_list: &mut FnLst,
) -> Result<Value> {
    var_lst.push_scope();
    fn_list.push_scope();

    let mut if_block = serde_json::json!({
        "type": "IfBlock",
        "statements": []
    });

    let mut statements = vec![];

    while *idx < tokens.len() {
        if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::Keyword || t.value != "if") {
            if tokens.get(*idx).map_or(false, |t| t.value == "else") {
                break;
            }
            return Err(anyhow!(
                "[fn parse_if_block] invalid start token: {}",
                tokens[*idx].value
            ));
        }
        *idx += 1;

        let condition = parse_expression(tokens, idx, var_lst, fn_list)?;

        if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::OpenCurlyBrace) {
            return Err(anyhow!(
                "[fn parse_if_block] invalid token after conditional expression: {}",
                tokens[*idx].value
            ));
        }

        let code_block = parse_code_block(tokens, idx, var_lst, fn_list)?;

        let statement = serde_json::json!({
            "condition": condition,
            "code-block": code_block
        });

        statements.push(statement);
    }

    if_block["statements"] = serde_json::Value::Array(statements);

    consume_whitespace(tokens, idx);
    if tokens.get(*idx).map_or(false, |t| t.token_type == TokenType::Keyword && t.value == "else") {
        *idx += 1;
        let default_block = parse_code_block(tokens, idx, var_lst, fn_list)?;
        if_block["default"] = default_block;
    }

    var_lst.pop_scope();
    fn_list.pop_scope();
    Ok(if_block)
}

fn parse_loop(
    tokens: &[Token],
    idx: &mut usize,
    var_lst: &mut VarLst,
    fn_list: &mut FnLst,
) -> Result<Value> {
    var_lst.push_scope();
    fn_list.push_scope();

    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::Keyword || t.value != "while") {
        return Err(anyhow!(
            "[fn parse_loop] error parsing, tokens do not start with `while`."
        ));
    }
    *idx += 1;

    let condition = parse_expression(tokens, idx, var_lst, fn_list)?;

    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::OpenCurlyBrace) {
        return Err(anyhow!(
            "[fn parse_loop] error parsing, conditional expression not followed by `{{`"
        ));
    }

    let code_block = parse_code_block(tokens, idx, var_lst, fn_list)?;

    let loop_block = serde_json::json!({
        "type": "Loop",
        "condition": condition,
        "code-block": code_block
    });

    var_lst.pop_scope();
    fn_list.pop_scope();
    Ok(loop_block)
}

fn parse_function_call(
    tokens: &[Token],
    idx: &mut usize,
    var_lst: &VarLst,
    fn_list: &FnLst,
) -> Result<Value> {
    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::Object) {
        return Err(anyhow!(
            "[fn parse_function_call] error parsing, tokens do not start TokenType::Object"
        ));
    }

    let function_name = tokens[*idx].value.to_string();
    *idx += 1;

    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::OpenParen) {
        return Err(anyhow!(
            "[fn parse_function_call] Expected `(` after function name."
        ));
    }
    *idx += 1;

    let mut arguments = vec![];
    while tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::CloseParen) {
        if tokens[*idx].token_type == TokenType::NewLine || tokens[*idx].token_type == TokenType::Comma {
            *idx += 1;
            continue;
        }

        let arg = parse_expression(tokens, idx, var_lst, fn_list)?;
        arguments.push(arg);
    }

    *idx += 1;

    let ret_type = if let Some((_, ret_type)) = fn_list.get(&function_name) {
        ret_type.clone()
    } else {
        DataType::Null // Or handle as an error if needed
    };

    let func_call = serde_json::json!({
        "type": "FunctionCall",
        "function-name": function_name,
        "parameters": arguments,
        "dtype": ret_type.to_string()
    });

    Ok(func_call)
}

fn parse_expression(
    tokens: &[Token],
    idx: &mut usize,
    var_lst: &VarLst,
    fn_list: &FnLst,
) -> Result<Value> {
    let mut num_paren = 0;
    let mut num_paren_on = false;

    let mut expr_end_idx = *idx;

    for i in *idx..tokens.len() {
        match tokens[i].token_type {
            TokenType::CloseCurlyBrace
            | TokenType::CloseSquareBracket
            | TokenType::OpenCurlyBrace
            | TokenType::SemiColon
            | TokenType::Comma
            | TokenType::NewLine => {
                expr_end_idx = i;
                break;
            }
            TokenType::OpenParen => {
                num_paren += 1;
                num_paren_on = true;
            }
            TokenType::CloseParen => {
                if num_paren_on {
                    if num_paren == 0 {
                        expr_end_idx = i;
                        break;
                    } else {
                        num_paren -= 1;
                    }
                } else {
                    expr_end_idx = i;
                    break;
                }
            }
            _ => {}
        }
    }

    if *idx == expr_end_idx {
        return Err(anyhow!("idx is equal to expr_end_idx"));
    } else if expr_end_idx - *idx == 1 {
        // Only one token
        if tokens[*idx].token_type == TokenType::Object {
            return parse_variable(tokens, idx, var_lst);
        } else if is_literal(&tokens[*idx].token_type) {
            return parse_literal(tokens, idx);
        }
    }

    // Now we need to find the operator with the highest priority
    let mut op_idx = 0;
    let mut op_priority = 0;

    for i in *idx..expr_end_idx {
        if tokens[i].token_type != TokenType::ArithmeticOperator
            && tokens[i].token_type != TokenType::ComparisonOperator
        {
            continue;
        }

        let op_type = OperationType::new(tokens[i].value)?;
        let curr_priority = op_type.get_priority();
        if curr_priority > op_priority {
            op_priority = curr_priority;
            op_idx = i;
        }
    }

    if op_priority == 0 {
        // No operator found
        if expr_end_idx - *idx == 1 {
            if tokens[*idx].token_type == TokenType::Object {
                return parse_variable(tokens, idx, var_lst);
            } else if is_literal(&tokens[*idx].token_type) {
                return parse_literal(tokens, idx);
            }
        }
        return Err(anyhow!(
            "[fn parse_expression] No operation found. Curr token: {:?}",
            tokens[*idx]
        ));
    }

    let operator = tokens[op_idx].value;
    let left_operand = parse_expression_h(tokens, *idx, op_idx - 1, var_lst, fn_list)?;
    let right_operand = parse_expression_h(tokens, op_idx + 1, expr_end_idx - 1, var_lst, fn_list)?;
    let dtype_res = inference_type(
        DataType::new(left_operand["dtype"].as_str().unwrap()),
        DataType::new(right_operand["dtype"].as_str().unwrap()),
        operator,
    )?;

    let op_node = serde_json::json!({
        "type": "Expression",
        "operator": operator,
        "left-operand": left_operand,
        "right-operand": right_operand,
        "dtype": dtype_res.to_string()
    });

    *idx = expr_end_idx;
    Ok(op_node)
}

fn parse_expression_h(
    tokens: &[Token],
    start: usize,
    end: usize,
    var_lst: &VarLst,
    fn_list: &FnLst,
) -> Result<Value> {
    if start == end {
        if is_literal(&tokens[start].token_type) {
            let mut idx = start;
            return parse_literal(tokens, &mut idx);
        } else if tokens[start].token_type == TokenType::Object {
            let mut idx = start;
            return parse_variable(tokens, &mut idx, var_lst);
        } else {
            // Handle function calls if necessary
            return Err(anyhow!(
                "[fn parse_expression_h] Unexpected token at position {}: {:?}",
                start, tokens[start]
            ));
        }
    } else if end < start {
        return Err(anyhow!("[fn parse_expression_h] end is less than start."));
    }

    // Find operator with highest priority
    let mut op_idx = 0;
    let mut op_priority = 0;

    for i in start..=end {
        if tokens[i].token_type != TokenType::ArithmeticOperator
            && tokens[i].token_type != TokenType::ComparisonOperator
        {
            continue;
        }

        let op_type = OperationType::new(tokens[i].value)?;
        let curr_priority = op_type.get_priority();
        if curr_priority > op_priority {
            op_priority = curr_priority;
            op_idx = i;
        }
    }

    if op_priority == 0 {
        if start == end {
            if tokens[start].token_type == TokenType::Object {
                let mut idx = start;
                return parse_variable(tokens, &mut idx, var_lst);
            } else if is_literal(&tokens[start].token_type) {
                let mut idx = start;
                return parse_literal(tokens, &mut idx);
            }
        }
        return Err(anyhow!(
            "[fn parse_expression_h] No operation found. Curr token: {:?}",
            tokens[start]
        ));
    }

    let operator = tokens[op_idx].value;
    let left_operand = parse_expression_h(tokens, start, op_idx - 1, var_lst, fn_list)?;
    let right_operand = parse_expression_h(tokens, op_idx + 1, end, var_lst, fn_list)?;
    let dtype_res = inference_type(
        DataType::new(left_operand["dtype"].as_str().unwrap()),
        DataType::new(right_operand["dtype"].as_str().unwrap()),
        operator,
    )?;

    let op_node = serde_json::json!({
        "type": "Expression",
        "operator": operator,
        "left-operand": left_operand,
        "right-operand": right_operand,
        "dtype": dtype_res.to_string()
    });

    Ok(op_node)
}

fn parse_declaration(
    tokens: &[Token],
    idx: &mut usize,
    var_lst: &VarLst,
    fn_list: &FnLst,
) -> Result<Value> {
    let dtype = DataType::new(tokens[*idx].value);
    let v_name = tokens.get(*idx + 1).ok_or_else(|| {
        anyhow!("[fn parse_declaration] Expected variable name after data type.")
    })?.value.to_string();
    *idx += 2; // Move past data type and variable name

    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::AssignmentOperator) {
        return Err(anyhow!(
            "[fn parse_declaration] Expected assignment operator after variable name."
        ));
    }
    *idx += 1; // Move past assignment operator

    let expr = parse_expression(tokens, idx, var_lst, fn_list)?;

    let decl_statement = serde_json::json!({
        "type": "DeclarationStatement",
        "dst": v_name,
        "src": expr,
        "dtype": dtype.to_string()
    });

    Ok(decl_statement)
}

fn parse_assignment(
    tokens: &[Token],
    idx: &mut usize,
    var_lst: &VarLst,
    fn_list: &FnLst,
) -> Result<Value> {
    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::Object) {
        return Err(anyhow!(
            "[fn parse_assignment] tokens do not start with object"
        ));
    }

    let dst = tokens[*idx].value.to_string();
    *idx += 1;

    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::AssignmentOperator) {
        return Err(anyhow!(
            "[fn parse_assignment] no assignment operator after variable"
        ));
    }
    *idx += 1;

    let src = parse_expression(tokens, idx, var_lst, fn_list)?;

    let assignment_s = serde_json::json!({
        "type": "AssignmentStatement",
        "dst": dst,
        "src": src
    });

    Ok(assignment_s)
}

fn parse_return(
    tokens: &[Token],
    idx: &mut usize,
    var_lst: &VarLst,
    fn_list: &FnLst,
) -> Result<Value> {
    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::Keyword || t.value != "return") {
        return Err(anyhow!(
            "[fn parse_return] token does not start with the return keyword"
        ));
    }

    *idx += 1;

    let value = parse_expression(tokens, idx, var_lst, fn_list)?;
    let dtype = value["dtype"].as_str().unwrap_or("Null").to_string();

    let ret_statement = serde_json::json!({
        "type": "ReturnStatement",
        "value": value,
        "dtype": dtype
    });

    Ok(ret_statement)
}

fn parse_literal(tokens: &[Token], idx: &mut usize) -> Result<Value> {
    let token = tokens.get(*idx).ok_or_else(|| anyhow!("No token found"))?;
    let dtype = match token.token_type {
        TokenType::IntegerLiteral => DataType::I64,
        TokenType::FloatLiteral => DataType::F64,
        TokenType::StringLiteral => DataType::String,
        TokenType::BooleanLiteral => DataType::Bool,
        _ => {
            return Err(anyhow!(
                "error parsing literal: token not a literal."
            ));
        }
    };

    let lit = serde_json::json!({
        "type": "Literal",
        "dtype": dtype.to_string(),
        "value": token.value
    });
    *idx += 1;
    Ok(lit)
}

fn parse_variable(
    tokens: &[Token],
    idx: &mut usize,
    var_lst: &VarLst,
) -> Result<Value> {
    if tokens.get(*idx).map_or(true, |t| t.token_type != TokenType::Object) {
        return Err(anyhow!(
            "error parsing variable: token is not an object."
        ));
    }

    let var_name = tokens[*idx].value;
    let v = var_lst.get(var_name).ok_or_else(|| {
        anyhow!("[fn parse_variable] variable `{}` does not exist", var_name)
    })?;

    let var = serde_json::json!({
        "type": "Variable",
        "dtype": v.to_string(),
        "name": var_name
    });
    *idx += 1;
    Ok(var)
}

fn consume_whitespace(tokens: &[Token], idx: &mut usize) {
    while *idx < tokens.len() && tokens[*idx].token_type == TokenType::NewLine {
        *idx += 1;
    }
}
