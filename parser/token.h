#ifndef TOKEN_H
#define TOKEN_H

// Enum for token types
enum TokenType {
    nuPy_IDENTIFIER,
    nuPy_INT_LITERAL,
    nuPy_REAL_LITERAL,
    nuPy_STR_LITERAL,
    nuPy_KEYW_TRUE,
    nuPy_KEYW_FALSE,
    nuPy_KEYW_NONE,
    nuPy_ASTERISK,
    nuPy_AMPERSAND,
    nuPy_PLUS,
    nuPy_MINUS,
    nuPy_POWER,
    nuPy_PERCENT,
    nuPy_SLASH,
    nuPy_EQUALEQUAL,
    nuPy_NOTEQUAL,
    nuPy_LT,
    nuPy_LTE,
    nuPy_GT,
    nuPy_GTE,
    nuPy_KEYW_IS,
    nuPy_KEYW_IN,
    nuPy_KEYW_IF,
    nuPy_KEYW_WHILE,
    nuPy_KEYW_ELIF,
    nuPy_KEYW_ELSE,
    nuPy_LEFT_PAREN,
    nuPy_RIGHT_PAREN,
    nuPy_LEFT_BRACE,
    nuPy_RIGHT_BRACE,
    nuPy_COLON,
    nuPy_EQUAL,
    nuPy_EOLN,
    nuPy_EOS
};

// Struct representing a token
struct Token {
    int id;
    int line;
    int col;
};

#endif // TOKEN_H
