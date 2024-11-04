#![allow(unused)]
mod lexer;
mod parser;
mod utils;
mod c_gen;

use std::{collections::HashSet, default, fs, io::Stdout, process, time::Instant};
use std::sync::OnceLock;
use serde::{Deserialize, Serialize};
use serde_json::{json, Value};
use regex::Regex;
use lexer::Lexer;
use utils::{FnLst, VarLst};


fn main() {
    let code = fs::read_to_string("truffle/main.tr")
        .unwrap()
        .split("\n")
        .filter(|&line| !line.trim().starts_with("//"))
        .map(|s| s.to_string())
        .collect::<Vec<String>>()
        .join("\n")
        .replace("  ", " ")
        .replace("\n\n", "\n");



    let timer = Instant::now();

    let mut lexer = Lexer::new(&code);

    while let Some(token) = lexer.next() {
        // println!("{:?}", token);
    }

    let errors = lexer.validate_syntax();
    println!("\nLexer Errors: {:#?}\n", errors);

    println!("Time Elapsed: {:?}", timer.elapsed());
    // process::exit(0);


    let mut var_lst = VarLst::new();
    let mut fn_list = FnLst::new();

    fn_list.insert("print".to_string(), parser::DataType::Null, vec![]);
    fn_list.insert("__some_c_func".to_string(), parser::DataType::Null, vec![]);

    let mut idx = 0;

    let s = parser::parse_module(&lexer.tokens, &mut idx, &mut var_lst, &mut fn_list)
        .unwrap();

    
    let mut s = serde_json::to_string_pretty(&s)
        .unwrap();

    s = s.replace("  ", "\t");

    fs::write("./ast.json", &s).unwrap();

    // println!("{:#?}", s);
}
