#include "dtype_utils.h"

#include <string>
#include <stdexcept>


BeDataType dtype_from_str(std::string s) {
    if (s == "byte") return BeDataType::U8;
    if (s == "int") return BeDataType::I64;
    if (s == "uint") return BeDataType::U64;
    if (s == "float") return BeDataType::F64;
    if (s == "bool") return BeDataType::Bool;
    if (s == "string") return BeDataType::String;
    if (s == "char") return BeDataType::Char;
    if (s == "null") return BeDataType::Null;

    if (s == "U8") return BeDataType::U8;
    if (s == "I64") return BeDataType::I64;
    if (s == "U64") return BeDataType::U64;
    if (s == "F64") return BeDataType::F64;
    if (s == "Bool") return BeDataType::Bool;
    if (s == "String") return BeDataType::String;
    if (s == "Char") return BeDataType::Char;
    if (s == "Null") return BeDataType::Null;

    throw std::runtime_error("Unknown data type [fn dtype_from_str]: " + s);
}

std::string dtype_to_str(BeDataType dtype) {
    if (dtype == BeDataType::U8) return "U8";
    if (dtype == BeDataType::I64) return "I64";
    if (dtype == BeDataType::U64) return "U64";
    if (dtype == BeDataType::F64) return "F64";
    if (dtype == BeDataType::Bool) return "Bool";
    if (dtype == BeDataType::String) return "String";
    if (dtype == BeDataType::Char) return "Char";
    if (dtype == BeDataType::Null) return "Null";

    throw std::runtime_error("Unknown data type [fn dtype_to_str]: " + dtype);
}

bool dtypes_check_valid(BeDataType actual, BeDataType inferenced) {
    return true;
}