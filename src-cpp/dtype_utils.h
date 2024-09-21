#ifndef DTYPE_UTILS_H
#define DTYPE_UTILS_H

#include <string>

// Backend data types
enum BeDataType {
    I64,
    U64,
    U8,
    F64,
    Bool,
    Char,
    String,
    Null,
};

BeDataType dtype_from_str(std::string s);
std::string dtype_to_str(BeDataType dtype);
bool dtypes_check_valid(BeDataType actual, BeDataType inferenced);

#endif