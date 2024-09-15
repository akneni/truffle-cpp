#ifndef SCOPE_TR_H
#define SCOPE_TR_H

#include "dtype_utils.h"
#include <string>
#include <vector>
#include <optional>

struct VariableTr {
    std::string name;
    BeDataType dtype;

    VariableTr clone() const;
};

struct VarLst {
    std::vector<std::vector<VariableTr>> vars;

    VarLst();
    
    void push_stack();

    void pop_stack();

    void push_back(VariableTr v);

    std::optional<VariableTr> get(std::string v) const;
    
    bool contains(std::string v);

    bool contains(VariableTr v);
};

struct FunctionTr {
    std::string name;
    std::vector<BeDataType> param_type;
    BeDataType ret_type;
    
    FunctionTr clone() const;
};

struct FuncLst {
    std::vector<std::vector<FunctionTr>> funcs;

    FuncLst();

    void push_stack();

    void pop_stack();

    void push_back(FunctionTr f);

    std::optional<FunctionTr> get(std::string f) const;

    bool contains(std::string f);

    bool contains(FunctionTr f);
};

#endif