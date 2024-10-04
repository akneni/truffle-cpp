use serde_json::Value;

fn gen_c_source(ast: Value) -> String {
    let mut c_source = "".to_string();

    assert!(ast.get("type").unwrap() == "module");

    let statements = ast.get("statements").unwrap();

    if let Value::Array(statements) = statements {
        for statement in statements.iter() {

        }
    }
    else {
        panic!("Error, field `statements` is not an array");
    }

    c_source
}

fn gen_c_func(ast_node: &Value) -> (String, String) {
    // Check that ast_node is a Function node
    if ast_node.get("type").unwrap() != "Function" {
        panic!("Expected Function node");
    }

    // Get function name (assuming it's provided in the AST)
    let func_name = ast_node.get("name").unwrap().as_str().unwrap();

    // Get parameters
    let parameters = ast_node.get("parameters").unwrap();

    // Get return type
    let ret_type = ast_node.get("ret-type").unwrap().as_str().unwrap();

    // Get code-block
    let code_block = ast_node.get("code-block").unwrap();

    // Generate parameter list
    let mut param_list = Vec::new();

    if let Value::Array(params) = parameters {
        for (i, param_type) in params.iter().enumerate() {
            let dtype = param_type.as_str().unwrap();
            // Generate parameter name as arg0, arg1, etc.
            let param_name = format!("arg{}", i);
            let c_param = format!("{} {}", dtype, param_name);
            param_list.push(c_param);
        }
    } else {
        panic!("Expected parameters to be an array");
    }

    let param_list_str = param_list.join(", ");

    // Generate function signature
    let mut function_code = format!("{} {}({}) {{\n", ret_type, func_name, param_list_str);

    // Generate function body by processing code-block
    let code_block_str = gen_c_codeblock(code_block);

    // Append code block to function code
    function_code.push_str(&code_block_str);

    // Close function
    function_code.push_str("}\n");

    // Return function code
    (function_code, "".to_string())
}

fn gen_c_codeblock(ast_node: &Value) -> String {
    let mut code = String::new();

    if ast_node.get("type").unwrap() != "CodeBlock" {
        panic!("Expected CodeBlock node");
    }

    let statements = ast_node.get("statements").unwrap();

    if let Value::Array(statements) = statements {
        for statement in statements.iter() {
            let stmt_type = statement.get("type").unwrap().as_str().unwrap();
            match stmt_type {
                "DeclarationStatement" => {
                    let decl_code = gen_c_declaration(statement);
                    code.push_str(&decl_code);
                    code.push_str(";\n");
                }
                "AssignmentStatement" => {
                    let assign_code = gen_c_assignment(statement);
                    code.push_str(&assign_code);
                    code.push_str(";\n");
                }
                "IfBlock" => {
                    let if_code = gen_c_ifblock(statement);
                    code.push_str(&if_code);
                }
                "Loop" => {
                    let loop_code = gen_c_loop(statement);
                    code.push_str(&loop_code);
                }
                "ReturnStatement" => {
                    let return_code = gen_c_return(statement);
                    code.push_str(&return_code);
                    code.push_str(";\n");
                }
                _ => {
                    panic!("Unsupported statement type: {}", stmt_type);
                }
            }
        }
    } else {
        panic!("Expected statements to be an array");
    }

    code
}

fn gen_c_return(ast_node: &Value) -> String {
    if ast_node.get("type").unwrap() != "ReturnStatement" {
        panic!("Expected ReturnStatement node");
    }

    let value = ast_node.get("value").unwrap();
    let value_code = gen_c_expression(value);

    format!("    return {}", value_code)
}

fn gen_c_expression(ast_node: &Value) -> String {
    let expr_type = ast_node.get("type").unwrap().as_str().unwrap();
    match expr_type {
        "Literal" => ast_node.get("value").unwrap().as_str().unwrap().to_string(),
        "Variable" => ast_node.get("name").unwrap().as_str().unwrap().to_string(),
        "Expression" => {
            let left = gen_c_expression(ast_node.get("left-operand").unwrap());
            let operator = ast_node.get("operator").unwrap().as_str().unwrap();
            let right = gen_c_expression(ast_node.get("right-operand").unwrap());
            format!("({} {} {})", left, operator, right)
        }
        "FunctionCall" => {
            let func_name = ast_node.get("function-name").unwrap().as_str().unwrap();
            let params = ast_node.get("parameters").unwrap();
            let mut param_codes = Vec::new();
            if let Value::Array(params) = params {
                for param in params {
                    param_codes.push(gen_c_expression(param));
                }
            }
            format!("{}({})", func_name, param_codes.join(", "))
        }
        _ => panic!("Unsupported expression type: {}", expr_type),
    }
}

// Placeholder implementations for other helper functions
fn gen_c_declaration(ast_node: &Value) -> String {
    let dtype = ast_node.get("dtype").unwrap().as_str().unwrap();
    let dst = ast_node.get("dst").unwrap().as_str().unwrap();
    let src = gen_c_expression(ast_node.get("src").unwrap());
    format!("    {} {} = {}", dtype, dst, src)
}

fn gen_c_assignment(ast_node: &Value) -> String {
    let dst = ast_node.get("dst").unwrap().as_str().unwrap();
    let src = gen_c_expression(ast_node.get("src").unwrap());
    format!("    {} = {}", dst, src)
}

fn gen_c_ifblock(ast_node: &Value) -> String {
    let mut code = String::new();
    code.push_str("    if (");
    let condition = gen_c_expression(ast_node.get("condition").unwrap());
    code.push_str(&condition);
    code.push_str(") {\n");
    let code_block = gen_c_codeblock(ast_node.get("code-block").unwrap());
    code.push_str(&code_block);
    code.push_str("    }\n");
    code
}

fn gen_c_loop(ast_node: &Value) -> String {
    let mut code = String::new();
    code.push_str("    while (");
    let condition = gen_c_expression(ast_node.get("condition").unwrap());
    code.push_str(&condition);
    code.push_str(") {\n");
    let code_block = gen_c_codeblock(ast_node.get("code-block").unwrap());
    code.push_str(&code_block);
    code.push_str("    }\n");
    code
}


fn gen_c_ifblock(ast_node: &Value) -> String {

}

fn gen_c_loop(ast_node: &Value) -> String {

}

fn gen_c_assignment(ast_node: &Value) -> String {

}

fn gen_c_declaration(ast_node: &Value) -> String {

}

