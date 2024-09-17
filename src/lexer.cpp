#include "lexer.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <regex>
#include <optional>
#include <stdexcept>
#include <cctype>
#include <iomanip>
#include <fstream>

// TokenType utils
bool is_literal(TokenType tok) {
    if (tok == TokenType::IntegerLiteral) return true;
    if (tok == TokenType::FloatLiteral) return true;
    if (tok == TokenType::StringLiteral) return true;
    if (tok == TokenType::BooleanLiteral) return true;
    return false;
}
// ---------------

std::string Token::token_type_to_string() const {
    switch (token_type) {
        case TokenType::ArithmeticOperator: return "ArithmeticOperator";
        case TokenType::AssignmentOperator: return "AssignmentOperator";
        case TokenType::ComparisonOperator: return "ComparisonOperator";
        case TokenType::IntegerLiteral: return "IntegerLiteral";
        case TokenType::FloatLiteral: return "FloatLiteral";
        case TokenType::StringLiteral: return "StringLiteral";
        case TokenType::BooleanLiteral: return "BooleanLiteral";
        case TokenType::Unknown: return "Unknown";
        case TokenType::Keyword: return "Keyword";
        case TokenType::Object: return "Object";
        case TokenType::OpenParen: return "OpenParen";
        case TokenType::CloseParen: return "CloseParen";
        case TokenType::OpenCurlyBrace: return "OpenCurlyBrace";
        case TokenType::CloseCurlyBrace: return "CloseCurlyBrace";
        case TokenType::OpenSquareBracket: return "OpenSquareBracket";
        case TokenType::CloseSquareBracket: return "CloseSquareBracket";
        case TokenType::DataType: return "DataType";
        case TokenType::Comma: return "Comma";
        case TokenType::Period: return "Period";
        case TokenType::RangeDescriptor: return "RangeDescriptor";
        case TokenType::SemiColon: return "SemiColon";
        case TokenType::NewLine: return "NewLine";
        default: return "Invalid";
    }
}

std::string Token::to_string() const {
    std::string val_output = value;
    if (val_output == "\n") {
        val_output = "\\n";  // Handle newline representation
    }

    std::stringstream ss;
    ss << std::left
       << std::setw(25) << token_type_to_string()
       << std::setw(10) << ("'" + val_output + "'");

    return "Token\t[  " + ss.str() + "  ]";
}

bool Token::equals(std::string val) const {
    return val == value;
}

bool Token::equals(TokenType tok_type) const {
    return tok_type == token_type;
}
bool Token::equals(TokenType tok_type, std::string val) const {
    return val == value && tok_type == token_type;
}

const std::vector<std::string> Lexer::DATA_TYPES = {
    "int",
    "float",
    "bool",
    "char",
    "byte",
    "string",
};

const std::vector<std::string> Lexer::KEYWORDS = {
    "fn",
    "if",
    "else",
    "for",
    "while",
    "return",
    "in",
};

Lexer::Lexer(const std::string& s)
    : source(s), pos(0) {
}

std::optional<Token> Lexer::next() {
    if (pos >= source.size()) {
        return std::nullopt;
    }

    size_t counter = pos;
    TokenType token_type = TokenType::Unknown;

    while (counter < source.size()) {
        char curr_char = source[counter];

        if (token_type == TokenType::Unknown) {
            auto lit_res = starts_with_literal(source.substr(pos));
            if (lit_res.has_value()) {
                size_t literal_len = lit_res->first;
                token_type = lit_res->second;
                counter += literal_len;
                break;
            } else {
                auto dot_res = starts_with_dots(source.substr(pos));
                if (dot_res.has_value()) {
                    size_t dot_len = dot_res->first;
                    token_type = dot_res->second;
                    counter += dot_len;
                    break;
                } else {
                    auto kw_len_opt = starts_with_kw(source.substr(pos));
                    if (kw_len_opt.has_value()) {
                        token_type = TokenType::Keyword;
                        counter += kw_len_opt.value();
                        break;
                    } else {
                        auto dt_len_opt = starts_with_dt(source.substr(pos));
                        if (dt_len_opt.has_value()) {
                            token_type = TokenType::DataType;
                            counter += dt_len_opt.value();
                            break;
                        } else {
                            auto cmp_op_len_opt = starts_with_cmp_op(source.substr(pos));
                            if (cmp_op_len_opt.has_value()) {
                                token_type = TokenType::ComparisonOperator;
                                counter += cmp_op_len_opt.value();
                                break;
                            } else {
                                auto assign_op_len_opt = starts_with_assign_op(source.substr(pos));
                                if (assign_op_len_opt.has_value()) {
                                    token_type = TokenType::AssignmentOperator;
                                    counter += assign_op_len_opt.value();
                                    break;
                                } else {
                                    auto obj_name_len_opt = starts_with_object_name(source.substr(pos));
                                    if (obj_name_len_opt.has_value()) {
                                        counter += obj_name_len_opt.value();
                                        std::string object_name = source.substr(pos, counter - pos);

                                        if (std::find(KEYWORDS.begin(), KEYWORDS.end(), object_name) != KEYWORDS.end() ||
                                            std::find(DATA_TYPES.begin(), DATA_TYPES.end(), object_name) != DATA_TYPES.end()) {
                                            throw std::runtime_error("object name cannot be a keyword or data-type.");
                                        }

                                        if (!tokens.empty()) {
                                            const Token& last_token = tokens.back();
                                            if (last_token.token_type == TokenType::Keyword) {
                                                if (last_token.value == "fn") {
                                                    token_type = TokenType::Object;
                                                    functions.insert(object_name);
                                                } else if (last_token.value == "for") {
                                                    token_type = TokenType::Object;
                                                    variables.insert(object_name);
                                                } else {
                                                    if (variables.find(object_name) != variables.end()) {
                                                        token_type = TokenType::Object;
                                                    } else if (functions.find(object_name) != functions.end()) {
                                                        token_type = TokenType::Object;
                                                    } else {
                                                        token_type = TokenType::Object;
                                                        std::cout << "Warning: Uninitialized object: " << object_name << "\n";
                                                    }
                                                }
                                            } else if (last_token.token_type == TokenType::DataType) {
                                                token_type = TokenType::Object;
                                                variables.insert(object_name);
                                            } else {
                                                if (variables.find(object_name) != variables.end()) {
                                                    token_type = TokenType::Object;
                                                } else if (functions.find(object_name) != functions.end()) {
                                                    token_type = TokenType::Object;
                                                } else {
                                                    token_type = TokenType::Object;
                                                    std::cout << "Warning: Uninitialized object: " << object_name << "\n";
                                                }
                                            }
                                        } else {
                                            token_type = TokenType::Object;
                                            std::cout << "Warning: Uninitialized object: " << object_name << "\n";
                                        }
                                        break;
                                    } else if (std::string("[{()}],\n;").find(curr_char) != std::string::npos) {
                                        counter += 1;
                                        switch (curr_char) {
                                            case '{':
                                                token_type = TokenType::OpenCurlyBrace;
                                                break;
                                            case '}':
                                                token_type = TokenType::CloseCurlyBrace;
                                                break;
                                            case '(':
                                                token_type = TokenType::OpenParen;
                                                break;
                                            case ')':
                                                token_type = TokenType::CloseParen;
                                                break;
                                            case '[':
                                                token_type = TokenType::OpenSquareBracket;
                                                break;
                                            case ']':
                                                token_type = TokenType::CloseSquareBracket;
                                                break;
                                            case ',':
                                                token_type = TokenType::Comma;
                                                break;
                                            case '\n':
                                                token_type = TokenType::NewLine;
                                                break;
                                            case ';':
                                                token_type = TokenType::SemiColon;
                                                break;
                                            default:
                                                throw std::runtime_error("This should never run");
                                        }
                                        break;
                                    } else if (std::string("+-*/%").find(curr_char) != std::string::npos) {
                                        counter += 1;
                                        token_type = TokenType::ArithmeticOperator;
                                        break;
                                    } else if (std::isspace(curr_char)) {
                                        counter += 1;
                                        pos += 1;
                                        continue;
                                    } else {
                                        throw std::runtime_error("No condition parsing met!\nString: `" + source.substr(pos) + "`\nFaulty Index: `" + std::to_string(counter - pos) + "`");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else {
            throw std::runtime_error("Not implemented");
        }
    }

    std::string res = source.substr(pos, counter - pos);
    pos = counter;
    Token token = {token_type, res};
    tokens.push_back(token);
    return token;
}

std::optional<size_t> Lexer::starts_with_dt(const std::string& s) {
    for (const auto& dt : DATA_TYPES) {
        if (s.substr(0, dt.size()) == dt) {
            size_t dt_len = dt.size();
            if (dt_len >= s.size()) continue;
            char next_char = s[dt_len];
            if (!(std::isspace(next_char) || next_char == '[' || next_char == '>')) {
                continue;
            }

            while (dt_len + 1 < s.size() && s[dt_len] == '[' && s[dt_len + 1] == ']') {
                dt_len += 2;
            }
            return dt_len;
        }
    }
    return std::nullopt;
}

std::optional<size_t> Lexer::starts_with_kw(const std::string& s) {
    for (const auto& kw : KEYWORDS) {
        if (s.substr(0, kw.size()) == kw) {
            if (kw.size() >= s.size()) continue;
            char next_char = s[kw.size()];
            if (!std::isspace(next_char)) {
                continue;
            }
            return kw.size();
        }
    }
    return std::nullopt;
}

std::optional<std::pair<size_t, TokenType>> Lexer::starts_with_literal(const std::string& s) {
    // Integer literal
    static const std::regex re_int(R"(^-?[0-9]+)");

    std::smatch match_int;
    if (std::regex_search(s, match_int, re_int) && match_int.position() == 0) {
        size_t l = match_int.str().size();
        // Ensure that the next character is not a decimal point (to prevent matching floats)
        if (l >= s.size() || s[l] != '.') {
            return std::make_pair(l, TokenType::IntegerLiteral);
        }
    }

    // Float literal
    static const std::regex re_fp(R"(^-?([0-9]+\.[0-9]*|\.[0-9]+))");

    std::smatch match_fp;
    if (std::regex_search(s, match_fp, re_fp) && match_fp.position() == 0) {
        std::string mat_str = match_fp.str();
        return std::make_pair(mat_str.size(), TokenType::FloatLiteral);
    }

    // String literal
    static const std::regex re_str(R"(^"[^\n]*")");
    std::smatch match_str;
    if (std::regex_search(s, match_str, re_str) && match_str.position() == 0) {
        return std::make_pair(match_str.str().size(), TokenType::StringLiteral);
    }

    // Boolean literal
    static const std::regex re_bool(R"(^(true|false))");
    std::smatch match_bool;
    if (std::regex_search(s, match_bool, re_bool) && match_bool.position() == 0) {
        return std::make_pair(match_bool.str().size(), TokenType::BooleanLiteral);
    }

    return std::nullopt;
}

std::optional<size_t> Lexer::starts_with_cmp_op(const std::string& s) {
    std::vector<std::string> operators = {
        "<=",
        ">=",
        "==",
        "!=",
        "<",
        ">",
    };

    for (const auto& op : operators) {
        if (s.substr(0, op.size()) == op) {
            return op.size();
        }
    }
    return std::nullopt;
}

std::optional<size_t> Lexer::starts_with_assign_op(const std::string& s) {
    std::vector<std::string> assign_ops = {":=", "="};
    for (const auto& op : assign_ops) {
        if (s.substr(0, op.size()) == op) {
            return op.size();
        }
    }
    return std::nullopt;
}

std::optional<std::pair<size_t, TokenType>> Lexer::starts_with_dots(const std::string& s) {
    static const std::regex re_period(R"(^\.[a-zA-Z_])");


    std::smatch match_period;
    if (std::regex_search(s, match_period, re_period)) {
        return std::make_pair(1, TokenType::Period);
    }

    static const std::regex re_range(R"(^\.\.=?)");
    std::smatch match_range;
    if (std::regex_search(s, match_range, re_range)) {
        return std::make_pair(match_range.str().size(), TokenType::RangeDescriptor);
    }

    return std::nullopt;
}

std::optional<size_t> Lexer::starts_with_object_name(const std::string& s) {
    static const std::regex re(R"(^[a-zA-Z_]\w*)");

    std::smatch match;
    if (std::regex_search(s, match, re)) {
        if (match.position() == 0) {
            return match.str().size();
        }
    }
    return std::nullopt;
}

std::vector<std::string> Lexer::validate_syntax() {
    std::vector<std::string> errors;

    int num_paren = 0;
    int num_brace = 0;
    int num_brack = 0;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& tok = tokens[i];
        switch (tok.token_type) {
            case TokenType::OpenParen:
                num_paren += 1;
                break;
            case TokenType::OpenCurlyBrace:
                num_brace += 1;
                break;
            case TokenType::OpenSquareBracket:
                num_brack += 1;
                break;
            case TokenType::CloseParen:
                num_paren -= 1;
                if (num_paren < 0) {
                    errors.push_back("[Token " + std::to_string(i) + "] Error: too many close parenthesis");
                }
                break;
            case TokenType::CloseCurlyBrace:
                num_brace -= 1;
                if (num_brace < 0) {
                    errors.push_back("[Token " + std::to_string(i) + "] Error: too many close curly braces");
                }
                break;
            case TokenType::CloseSquareBracket:
                num_brack -= 1;
                if (num_brack < 0) {
                    errors.push_back("[Token " + std::to_string(i) + "] Error: too many close square brackets");
                }
                break;
            case TokenType::RangeDescriptor:
                if (i == 0 || i >= tokens.size() - 1) {
                    errors.push_back("[Token " + std::to_string(i) + "] Error: Invalid range descriptor");
                    continue;
                }
                if (!(tokens[i - 1].token_type == TokenType::IntegerLiteral || tokens[i - 1].token_type == TokenType::Object) ||
                    !(tokens[i + 1].token_type == TokenType::IntegerLiteral || tokens[i + 1].token_type == TokenType::Object)) {
                    errors.push_back("[Token " + std::to_string(i) + "] Error: Invalid range descriptor");
                }
                break;
            default:
                break;
        }
    }

    return errors;
}