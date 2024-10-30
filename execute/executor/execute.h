#ifndef EXECUTE_H
#define EXECUTE_H

#include <stdbool.h>

struct STMT_FUNCTION_CALL;

enum STMT_TYPE {
    STMT_ASSIGNMENT,
    STMT_FUNCTION_CALL,
    STMT_BINARY_EXPR,
};

struct STMT {
    enum STMT_TYPE type;
    char* variable;
    struct UNARY_EXPR* unary;
    struct RAM_VALUE* lhs;
    struct RAM_VALUE* rhs;
    struct STMT_FUNCTION_CALL* function_call;
};

struct STMT_FUNCTION_CALL {
    char* function_name;
    struct RAM_VALUE** args;
    size_t num_args;
};

struct UNARY_EXPR {
    char* variable;
};

struct ELEMENT {
    struct STMT* stmt;
};

#endif // EXECUTE_H
