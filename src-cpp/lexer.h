#ifndef LEXER_H  // Include guard to prevent multiple inclusions
#define LEXER_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <regex>
#include <optional>
#include <stdexcept>
#include <cctype>
#include <iomanip>

// TokenType enumeration
enum class TokenType {
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
};

bool is_literal(TokenType tok);


struct Token {
    TokenType token_type;
    std::string value;

    // Function to convert TokenType to a string
    std::string token_type_to_string() const;

    // Function to return a string representation of the Token
    std::string to_string() const;

    bool equals(std::string val) const;
    bool equals(TokenType tok_type) const;
    bool equals(TokenType tok_type, std::string val) const;
};

// Lexer class declaration
class Lexer {
public:
    // Constructor that takes a source string
    Lexer(const std::string& s);

    // Function to get the next token
    std::optional<Token> next();

    // Function to validate the syntax
    std::vector<std::string> validate_syntax();

    // Data members
    const std::string& source;
    size_t pos;
    std::vector<Token> tokens;
    std::unordered_set<std::string> variables;
    std::unordered_set<std::string> functions;

private:
    // Constants for data types and keywords
    static const std::vector<std::string> DATA_TYPES;
    static const std::vector<std::string> KEYWORDS;

    // Helper methods for token parsing
    std::optional<size_t> starts_with_dt(const std::string& s);
    std::optional<size_t> starts_with_kw(const std::string& s);
    std::optional<std::pair<size_t, TokenType>> starts_with_literal(const std::string& s);
    std::optional<size_t> starts_with_cmp_op(const std::string& s);
    std::optional<size_t> starts_with_assign_op(const std::string& s);
    std::optional<std::pair<size_t, TokenType>> starts_with_dots(const std::string& s);
    std::optional<size_t> starts_with_object_name(const std::string& s);
};

#endif  // LEXER_H
