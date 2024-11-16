use std::{collections::HashMap, env, fmt::Debug, fs, sync::{Arc, Mutex}};
use anyhow::{anyhow, Result};

struct FunctionMap {
    map: HashMap<String, String>,
}

impl FunctionMap {
    fn new() -> Self {
        let map: HashMap<String, String> = [
            ("strcpy".to_string(), "strncpy".to_string()),
            ("strcat".to_string(), "strncat".to_string()),
            ("gets".to_string(), "fgets".to_string()),
            ("sprintf".to_string(), "snprintf".to_string()),
        ].into_iter().collect();

        Self {map}
    }

    fn line_contains_unsafe<'a>(&'a self, line: &str) -> Vec<(&'a str, &'a str)> {
        let mut unsafe_funcs = vec![];
        for (k, v) in self.map.iter() {
            if Self::line_contains_fn(line, k) {
                unsafe_funcs.push((k.as_str(), v.as_str()));
            }
        }
        unsafe_funcs
    }

    #[inline]
    fn line_contains_fn(line: &str, func: &str) -> bool {
        let line_bytes = line.as_bytes();
        if let Some(mut idx) = line.find(func) {
            idx += func.len();
            if line_bytes[idx] == '(' as u8 {
                return true;
            }
            else if line_bytes[idx] != ' ' as u8 {
                return false;
            }
            
        }
        false
    }
}

#[derive(Debug)]
enum WarningType {
    UnsafeFunction
}

#[derive(Debug)]
pub struct Warning {
    msg: String,
    filename: String,
    line: usize,
    warning_type: WarningType
}

impl Warning {
    pub fn to_string(&self) -> String {
        format!(
            "TrufC Warning [src/{} | Line {} ]: {:?}\n{}", 
            self.filename, 
            self.line, 
            self.warning_type, 
            self.msg
        )
    }
}

pub fn check_files(source_type: &str) -> Result<Vec<Warning>> {
    let mut warnings = vec![];
    let mut source_dir = env::current_dir()?;
    source_dir.push("src");

    if !source_dir.exists() {
        return Err(anyhow!("src/ does not exist."));
    }
    else if !source_dir.is_dir() {
        return Err(anyhow!("src is not a directory."));
    }

    let func_map = FunctionMap::new();

    for path in fs::read_dir(source_dir)? {
        if let Ok(path) = path {
            let path = path.path();
            let name = path.file_name()
                .unwrap()
                .to_str()
                .unwrap()
                .to_string();
            if !name.ends_with(source_type) {
                continue;
            }

            let source_code = fs::read_to_string(path)?;
            let mut curr_warnings = scan_file(&name, &source_code, &func_map);
            
            warnings.append(&mut curr_warnings);
        }
    }

    Ok(warnings)
}


#[allow(unused)]
pub fn check_files_threaded(source_type: &str, warn_buff: Arc<Mutex<Vec<Warning>>>) -> Result<()> {
    let mut warnings = check_files(source_type)?;
    
    let mut lock = warn_buff.lock().unwrap();
    lock.append(&mut warnings);
    
    Ok(())
}

fn scan_file(filename: &str, source_code: &str, func_map: &FunctionMap) ->Vec<Warning> {
    let mut warnings = vec![];

    let source_code = source_code
        .split("\n");

    for (i, line) in source_code.enumerate() {
        let unsafe_funcs = func_map.line_contains_unsafe(line);
        for (unsafe_f, safe_f) in unsafe_funcs {
            let warning = Warning {
                msg: format!("Function {}() is unsafe, consider using {}() instead", unsafe_f, safe_f),
                filename: filename.to_string(),
                line: i,
                warning_type: WarningType::UnsafeFunction,
            };
            warnings.push(warning);
        }
    }

    warnings
}
