use std::{cell::OnceCell, collections::HashSet};
use serde::{Serialize, Deserialize};
use regex::Regex;


#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq, Eq)]
pub enum TokenType {
    ArithmeticOperator,
    AssignmentOperator,
    ComparisonOperator,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    BooleanLiteral,
    Unknown,
    Keyword,
    Object,
    OpenParen,
    CloseParen,
    OpenCurlyBrace,
    CloseCurlyBrace,
    OpenSquareBracket,
    CloseSquareBracket,
    DataType,
    Comma,
    Period,
    RangeDescriptor,
    SemiColon,
    NewLine,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Token<'a> {
    pub token_type: TokenType,
    pub value: &'a str,
}

/// Preconditions:
/// - All commented text must be striped before feeding source code to the lexer
/// - Not required, but removing redundant whitespace characters could improve compilation speed
#[derive(Default)]
pub struct Lexer<'a> {
    pub source: &'a str,
    pub pos: usize,
    pub tokens: Vec<Token<'a>>,
    pub variables: HashSet<&'a str>,
    pub functions: HashSet<&'a str>,
}

impl<'a> Lexer<'a> {
    pub fn new(s: &'a str) -> Self {
        Lexer {
            source: s,
            ..Lexer::default()
        }
    }

    pub fn next(&mut self) -> Option<Token> {
        if self.pos >= self.source.len() {
            return None;
        }

        let source_bytes = self.source.as_bytes();
        let mut counter = self.pos;

        let mut token_type = TokenType::Unknown;

        while counter < self.source.len() {
            let curr_char = source_bytes[counter] as char;
            match token_type {
                TokenType::Unknown => {
                    if let Some((literal, lit_type)) = Self::starts_with_literal(&self.source[self.pos..]) {
                        counter += literal;
                        token_type = lit_type;
                        break;
                    }
                    else if let Some((literal, lit_type)) = Self::starts_with_dots(&self.source[self.pos..]) {
                        counter += literal;
                        token_type = lit_type;
                        break;
                    }
                    else if let Some(keyword_len) = Self::starts_with_kw(&self.source[self.pos..]) {
                        token_type = TokenType::Keyword;
                        counter += keyword_len;
                        break;
                    }
                    else if let Some(dt_len) = Self::starts_with_dt(&self.source[self.pos..]) {
                        token_type = TokenType::DataType;
                        counter += dt_len;
                        break;
                    }
                    else if let Some(op_len) = Self::starts_with_cmp_op(&self.source[self.pos..]) {
                        token_type = TokenType::ComparisonOperator;
                        counter += op_len;
                        break;
                    }
                    else if let Some(op_len) = Self::starts_with_assign_op(&self.source[self.pos..]) {
                        token_type = TokenType::AssignmentOperator;
                        counter += op_len;
                        break;
                    }
                    else if let Some(object_name_len) = Self::starts_with_object_name(&self.source[self.pos..]) {
                        counter += object_name_len;
                        let object_name = &self.source[self.pos..counter];

                        if KEYWORDS.contains(&object_name) || DATA_TYPES.contains(&object_name) {
                            panic!("object name cannot be a keyword or data-type. ")
                        }

                        if let Some(last_token) = self.tokens.last() {
                            match last_token.token_type {
                                TokenType::Keyword => {
                                    match last_token.value {
                                        "fn" => {
                                            token_type = TokenType::Object;
                                            self.functions.insert(object_name);

                                        }
                                        "for" => {
                                            token_type = TokenType::Object;     
                                            self.variables.insert(object_name);                                   
                                        }
                                        _ => {
                                            if self.variables.contains(object_name) {
                                                token_type = TokenType::Object;
                                            }
                                            else if self.functions.contains(object_name) {
                                                token_type = TokenType::Object;
                                            }
                                            else {
                                                token_type = TokenType::Object;
                                                // panic!("Uninitialized object: `{}`", object_name);
                                            }
                                        }
                                    }
                                }
                                TokenType::DataType => {
                                    token_type = TokenType::Object;
                                    let func_name = &self.source[self.pos..counter];
                                    self.variables.insert(func_name);
                                }                                
                                _ => {
                                    if self.variables.contains(object_name) {
                                        token_type = TokenType::Object;
                                    }
                                    else if self.functions.contains(object_name) {
                                        token_type = TokenType::Object;
                                    }
                                    else {
                                        token_type = TokenType::Object;
                                        // panic!("Uninitialized object: `{}`", object_name);
                                    }
                                },
                            }
                        }
                        break;
                    }
                    else if "[{()}],\n;".contains(curr_char) {
                        counter += 1;
                        match curr_char {
                            '{' => token_type = TokenType::OpenCurlyBrace,
                            '}' => token_type = TokenType::CloseCurlyBrace,
                            '(' => token_type = TokenType::OpenParen,
                            ')' => token_type = TokenType::CloseParen,
                            '[' => token_type = TokenType::OpenSquareBracket,
                            ']' => token_type = TokenType::CloseSquareBracket,
                            ',' => token_type = TokenType::Comma,
                            '\n' => token_type = TokenType::NewLine,
                            ';' => token_type = TokenType::SemiColon,
                            _ => panic!("This should never run"),
                        }
                        break;
                    }
                    else if "+-*/%".contains(curr_char) {
                        counter += 1;
                        token_type = TokenType::ArithmeticOperator;
                        break;
                    }
                    else if "\t ".contains(curr_char) {
                        counter += 1;
                        self.pos += 1;
                        assert_eq!(self.pos, counter);
                        continue;
                    }
                    else {
                        panic!("No condition parsing met!\nString: `{}`\nFaulty Index: `{}`", &self.source[self.pos..], counter-self.pos);
                    }
                }
                _ => panic!("Not implemented"),
            }
        }

        let res = &self.source[self.pos..counter];
        self.pos = counter;

        let token = Token { token_type: token_type, value: res };

        self.tokens.push(token.clone());
        Some(token)
    }

    fn starts_with_dt(s: &str) -> Option<usize> {
        let s_bytes: &[u8] = s.as_bytes();

        for &dt in DATA_TYPES.iter() {
            if s.starts_with(dt) {
                let mut dt_len = dt.len();
                if !(char::is_whitespace(s_bytes[dt.len()] as char) || "[>".contains(s_bytes[dt.len()] as char)) {
                    continue;
                }

                while dt_len+1 < s.len() && s_bytes[dt_len] as char == '[' && s_bytes[dt_len+1] as char == ']' {
                    dt_len += 2;
                }
                
                return Some(dt_len);
            }
        }

        None
    }

    fn starts_with_kw(s: &str) -> Option<usize> {
        let s_bytes = s.as_bytes();
        for &kw in KEYWORDS.iter() {
            if s.starts_with(kw) {
                if !char::is_whitespace(s_bytes[kw.len()] as char) {
                    continue;
                }
                return Some(kw.len());
            }
        }

        None
    }

    fn starts_with_literal(s: &str) -> Option<(usize, TokenType)> {
        
        let re_int = Regex::new(r"^(([0-9])([0-9]|_)*)|(-([0-9]|_)+)").unwrap();
        if let Some(mat) = re_int.find(s) {
            let l = mat.as_str().len();
            let s_bytes = s.as_bytes();
            if s_bytes[l] as char != '.' {
                return Some((l, TokenType::IntegerLiteral));
            }
        }

        if !s.starts_with("..") {
            let re_fp = Regex::new(r"^([0-9]|-|\.)([0-9]|\.|_)*").unwrap();
            if let Some(mat) = re_fp.find(s) {
                if mat.as_str().chars().map(|c| if c == '.' {1} else {0}).sum::<i32>() > 1 {
                    let num = mat.as_str().split_once(".").unwrap().0;
                    return Some((num.len(), TokenType::IntegerLiteral));
                }
                return Some((mat.as_str().len(), TokenType::FloatLiteral));
            }
        }

        let re_str = Regex::new(r#"^"[^\n]*""#).unwrap();
        if let Some(mat) = re_str.find(s) {
            return Some((mat.as_str().len(), TokenType::StringLiteral));
        }

        let re_bool = Regex::new(r"^(true|false)").unwrap();
        if let Some(mat) = re_bool.find(s) {
            return Some((mat.as_str().len(), TokenType::BooleanLiteral));
        }

        None
    }

    fn starts_with_cmp_op(s: &str) -> Option<usize> {
        let operators = vec![
            "<=",
            ">=",
            "==",
            "!=",
            "<",
            ">",
        ];

        for &op in operators.iter() {
            if s.starts_with(op) {               
                return Some(op.len());
            }
        }

        None
    }

    fn starts_with_assign_op(s: &str) -> Option<usize> {
        let assign_ops = [":=", "="];
        for &op in assign_ops.iter() {
            if s.starts_with(op) {
                return Some(op.len());
            }
        }

        None
    }

    fn starts_with_dots(s: &str) -> Option<(usize, TokenType)> {
        let re_period = Regex::new(r"^\.[a-zA-Z_]").unwrap();
        if let Some(mat) = re_period.find(s) {
            return Some((1, TokenType::Period));
        }

        let re_range = Regex::new(r"^\.\.=?").unwrap();
        if let Some(mat) = re_range.find(s) {
            return Some((mat.as_str().len(), TokenType::RangeDescriptor));
        }

        None
    }

    fn starts_with_object_name(s: &str) -> Option<usize> {
        let re = Regex::new(r"^[a-zA-Z_](\w|_)*").unwrap();
        let mat = re.find(s);

        if let Some(mat) = mat {
            let r: std::ops::Range<usize> = mat.range();
            if r.start == 0 {
                return Some(mat.as_str().len());
            }
        }

        None
    }

    pub fn validate_syntax(&self) -> Vec<String> {
        let mut errors = vec![];

        let mut num_paren = 0;
        let mut num_brace = 0;
        let mut num_brack = 0;

        for (i, tok) in self.tokens.iter().enumerate() {
            match tok.token_type {
                TokenType::OpenParen => num_paren += 1,                
                TokenType::OpenCurlyBrace => num_brace += 1,
                TokenType::OpenSquareBracket => num_brack += 1,
                TokenType::CloseParen => {
                    num_paren -= 1;
                    if num_paren < 0 {
                        errors.push(format!("[Token {}] Error: too many close parenthesis", i));
                    }
                },                
                TokenType::CloseCurlyBrace => {
                    num_brace -= 1;
                    if num_brace < 0 {
                        errors.push(format!("[Token {}] Error: too many close curly braces", i));
                    }
                },             
                TokenType::CloseSquareBracket => {
                    num_brack -= 1;
                    if num_brack < 0 {
                        errors.push(format!("[Token {}] Error: too many close square brackets", i));
                    }
                },
                TokenType::RangeDescriptor => {
                    if i == 0 || i >= self.tokens.len()-1 {
                        errors.push(format!("[Token {}] Error: Invalid range descriptor", i));
                        continue;
                    }

                    if 
                        ![TokenType::IntegerLiteral, TokenType::Object].contains(&self.tokens[i-1].token_type) || 
                        ![TokenType::IntegerLiteral, TokenType::Object].contains(&self.tokens[i+1].token_type) 
                    {
                        errors.push(format!("[Token {}] Error: Invalid range descriptor", i));
                    }
                }          

                _ => {}
            }
        }

        errors
    }

}

pub const DATA_TYPES: [&str; 6] = [
    "int",
    "float",
    "bool",
    "char",
    "byte",
    "string",
];

pub const KEYWORDS: [&str; 7] = [
    "fn",
    "if",
    "else",
    "for",
    "while",
    "return",
    "in",
];
