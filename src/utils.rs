use std::collections::HashMap;
use crate::parser::DataType;


pub struct VarLst {
    vars: Vec<HashMap<String, DataType>>
}

impl VarLst {
    pub fn new() -> Self {
        VarLst {
            vars: vec![HashMap::new()]
        }
    }

    pub fn insert(&mut self, var: String, dtype: DataType) {
        self.vars.last_mut().unwrap().insert(var, dtype);
    }

    pub fn get(&self, var: &str) -> Option<DataType> {
        for scope in self.vars.iter().rev() {
            if let Some(d) = scope.get(var) {
                return Some(d.clone());
            }
        }
        None
    }

    pub fn push_scope(&mut self) {
        self.vars.push(HashMap::new());
    }

    pub fn pop_scope(&mut self) {
        self.vars.pop();
    }
}

pub struct FnLst {
    funcs: Vec<HashMap<String, (Vec<(String, DataType)>, DataType)>>
}

impl FnLst {
    pub fn new() -> Self {
        FnLst {
            funcs: vec![HashMap::new()]
        }
    }

    pub fn insert(&mut self, var: String, res_type: DataType, args: Vec<(String, DataType)>) {
        self.funcs.last_mut().unwrap().insert(var, (args, res_type));
    }

    pub fn get(&self, var: &str) -> Option<(&Vec<(String, DataType)>, DataType)> {
        for scope in self.funcs.iter().rev() {
            if let Some(d) = scope.get(var) {
                return Some((&d.0, d.1.clone()));
            }
        }
        None
    }

    pub fn push_scope(&mut self) {
        self.funcs.push(HashMap::new());
    }

    pub fn pop_scope(&mut self) {
        self.funcs.pop();
    }
}