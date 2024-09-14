#include "parser.h"
#include <string>
#include <vector>

struct VariableTr {
    std::string name;
    DataType dtype;
};

struct VarLst {
    std::vector<std::vector<VariableTr>> vars;

    
    void push_stack();

    void pop_stack();

    void push_back(VariableTr v);

    bool contains(VariableTr v);
};

struct FunctionTr {
    std::string name;
    std::vector<DataType> param_type;
    DataType ret_type;
};

struct FuncLst {
    std::vector<std::vector<FunctionTr>> funcs;

    void push_stack();

    void pop_stack();

    void push_back(FunctionTr f);

    bool contains(FunctionTr f);
};