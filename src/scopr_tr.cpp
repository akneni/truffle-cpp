#include "parser.h"
#include "scope_tr.h"

#include <string>
#include <vector>
#include <algorithm>


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