#include "lexer.h"
#include "parser.h"
#include "scope_tr.h"

#include <string>
#include <vector>
#include <algorithm>
#include <optional>

VariableTr VariableTr::clone() const {
    return VariableTr {
        .name = name,
        .dtype = dtype
    };
}

VarLst::VarLst() {
    vars.push_back(std::vector<VariableTr>()); 
}

void VarLst::push_stack() {
    std::vector<VariableTr> v = {};
    vars.push_back(v);
}

void VarLst::pop_stack() {
    vars.pop_back();
}

void VarLst::push_back(VariableTr v) {
    vars[vars.size()-1].push_back(v);
}

std::optional<VariableTr> VarLst::get(std::string v) const {
    for (int i = vars.size()-1; i >= 0; i--) {
        for (int j = 0; j < vars[i].size(); j++) {
            if (vars[i][j].name == v) {
                return vars[i][j].clone();
            }
        }
    }
    return std::nullopt;
}

bool VarLst::contains(std::string v) {
    for (int i = vars.size()-1; i >= 0; i--) {
        for (int j = 0; j < vars[i].size(); j++) {
            if (vars[i][j].name == v) {
                return true;
            }
        }
    }
    return false;
}

bool VarLst::contains(VariableTr v) {
    for (int i = vars.size()-1; i >= 0; i--) {
        for (int j = 0; j < vars[i].size(); j++) {
            if (vars[i][j].name == v.name) {
                return true;
            }
        }
    }
    return false;
}

FunctionTr FunctionTr::clone() const {
    FunctionTr copy;
    copy.name = this->name;
    copy.param_type = this->param_type;
    copy.ret_type = this->ret_type;

    return copy;
}

FuncLst::FuncLst() {
    funcs.push_back(std::vector<FunctionTr>());  
}

void FuncLst::push_stack() {
    std::vector<FunctionTr> f = {};
    funcs.push_back(f);
}

void FuncLst::pop_stack() {
    funcs.pop_back();
}

void FuncLst::push_back(FunctionTr f) {
    funcs[funcs.size()-1].push_back(f);
}

std::optional<FunctionTr> FuncLst::get(std::string f) const {
    for (int i = funcs.size()-1; i >= 0; i--) {
        for (int j = 0; j < funcs[i].size(); j++) {
            if (funcs[i][j].name == f) {
                return funcs[i][j].clone();
            }
        }
    }
    return std::nullopt;
}

bool FuncLst::contains(std::string f) {
    for (int i = funcs.size()-1; i >= 0; i--) {
        for (int j = 0; j < funcs[i].size(); j++) {
            if (funcs[i][j].name == f) {
                return true;
            }
        }
    }
    return false;
}

bool FuncLst::contains(FunctionTr f) {
    for (int i = funcs.size()-1; i >= 0; i--) {
        for (int j = 0; j < funcs[i].size(); j++) {
            if (
                funcs[i][j].name == f.name &&
                funcs[i][j].ret_type == f.ret_type &&
                funcs[i][j].param_type == f.param_type
            ) {
                return true;
            }
        }
    }
    return false;
}