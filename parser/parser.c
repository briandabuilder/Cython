#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "scanner.h"
#include "parser.h"

static void printSyntaxError(char* expected, char* actual, struct Token foundToken);
static bool verifyToken(struct TokenQueue* tokens, int expectedID, char* expectedValue);

static bool parseExpression(struct TokenQueue* tokens);
static bool parseBlock(struct TokenQueue* tokens);
static bool parseElse(struct TokenQueue* tokens);

static bool parseIfElse(struct TokenQueue* tokens);
static bool parsePass(struct TokenQueue* tokens);
static bool parseEmpty(struct TokenQueue* tokens);
static bool isStatementStart(struct TokenQueue* tokens);
static bool parseStatement(struct TokenQueue* tokens);
static bool parseStatementList(struct TokenQueue* tokens);
static bool parseProgram(struct TokenQueue* tokens);

static void printSyntaxError(char* expected, char* actual, struct Token foundToken) {
    printf("**SYNTAX ERROR @ (%d,%d): expected %s, found '%s'\n", foundToken.line, foundToken.col, expected, actual);
}

static bool verifyToken(struct TokenQueue* tokens, int expectedID, char* expectedValue) {
    struct Token curToken = tokenqueue_peekToken(tokens);
    if (curToken.id != expectedID) {
        printSyntaxError(expectedValue, tokenqueue_peekValue(tokens), curToken);
        return false;
    }
    tokenqueue_dequeue(tokens);
    return true;
}

static bool parseElement(struct TokenQueue* tokens) {
    struct Token curToken = tokenqueue_peekToken(tokens);
    if (curToken.id == nuPy_IDENTIFIER || curToken.id == nuPy_INT_LITERAL || 
        curToken.id == nuPy_REAL_LITERAL || curToken.id == nuPy_STR_LITERAL || 
        curToken.id == nuPy_KEYW_TRUE || curToken.id == nuPy_KEYW_FALSE || 
        curToken.id == nuPy_KEYW_NONE) {
        tokenqueue_dequeue(tokens);
        return true;
    }
    return false;
}

static bool parseUnaryExpression(struct TokenQueue* tokens) {
    struct Token curToken = tokenqueue_peekToken(tokens);
    if (curToken.id == nuPy_ASTERISK || curToken.id == nuPy_AMPERSAND ||
        curToken.id == nuPy_PLUS || curToken.id == nuPy_MINUS) {
        tokenqueue_dequeue(tokens);
        if (curToken.id == nuPy_PLUS || curToken.id == nuPy_MINUS) {
            curToken = tokenqueue_peekToken(tokens);
            if (curToken.id == nuPy_IDENTIFIER || curToken.id == nuPy_INT_LITERAL || curToken.id == nuPy_REAL_LITERAL) {
                tokenqueue_dequeue(tokens);
                return true;
            }
            return false;
        }
        return verifyToken(tokens, nuPy_IDENTIFIER, "IDENTIFIER");
    }
    return parseElement(tokens);
}

static bool parseOperator(struct TokenQueue* tokens) {
    struct Token curToken = tokenqueue_peekToken(tokens);
    if (curToken.id == nuPy_ASTERISK || curToken.id == nuPy_PLUS || curToken.id == nuPy_MINUS ||
        curToken.id == nuPy_POWER || curToken.id == nuPy_PERCENT || curToken.id == nuPy_SLASH ||
        curToken.id == nuPy_EQUALEQUAL || curToken.id == nuPy_NOTEQUAL || curToken.id == nuPy_LT ||
        curToken.id == nuPy_LTE || curToken.id == nuPy_GT || curToken.id == nuPy_GTE ||
        curToken.id == nuPy_KEYW_IS || curToken.id == nuPy_KEYW_IN) {
        tokenqueue_dequeue(tokens);
        return true;
    }
    return false;
}

static bool parseFunctionCall(struct TokenQueue* tokens) {
    if (!verifyToken(tokens, nuPy_IDENTIFIER, "IDENTIFIER")) return false;
    if (!verifyToken(tokens, nuPy_LEFT_PAREN, "(")) return false;
    if (parseElement(tokens)) {}
    return verifyToken(tokens, nuPy_RIGHT_PAREN, ")");
}

static bool parseValue(struct TokenQueue* tokens) {
    struct Token nextToken = tokenqueue_peek2Token(tokens);
    if (nextToken.id == nuPy_LEFT_PAREN) return parseFunctionCall(tokens);
    if (!parseExpression(tokens)) {
        struct Token currentToken = tokenqueue_peekToken(tokens);
        char* curValue = tokenqueue_peekValue(tokens);
        if (currentToken.id != nuPy_IDENTIFIER && currentToken.id != nuPy_REAL_LITERAL &&
            currentToken.id != nuPy_INT_LITERAL && currentToken.id != nuPy_STR_LITERAL) {
            printSyntaxError("expected an identifier, numeric literal, or valid expression", curValue, currentToken);
        }
        return false;
    }
    return true;
}

static bool parseAssignment(struct TokenQueue* tokens) {
    struct Token currentToken = tokenqueue_peekToken(tokens);
    if (currentToken.id == nuPy_ASTERISK) tokenqueue_dequeue(tokens);
    if (!verifyToken(tokens, nuPy_IDENTIFIER, "IDENTIFIER")) return false;
    if (!verifyToken(tokens, nuPy_EQUAL, "=")) return false;
    if (!parseValue(tokens)) return false;
    return verifyToken(tokens, nuPy_EOLN, "EOLN");
}

static bool parseWhileLoop(struct TokenQueue* tokens) {
    if (!verifyToken(tokens, nuPy_KEYW_WHILE, "while")) return false;
    if (!parseExpression(tokens)) return false;
    if (!verifyToken(tokens, nuPy_COLON, ":")) return false;
    if (!verifyToken(tokens, nuPy_EOLN, "EOLN")) return false;
    return parseBlock(tokens);
}

static bool parseExpression(struct TokenQueue* tokens) {
    if (!parseUnaryExpression(tokens)) return false;
    if (parseOperator(tokens)) {
        if (!parseUnaryExpression(tokens)) return false;
    }
    return true;
}

static bool parseBlock(struct TokenQueue* tokens) {
    if (!verifyToken(tokens, nuPy_LEFT_BRACE, "{")) return false;
    if (!verifyToken(tokens, nuPy_EOLN, "EOLN")) return false;
    if (!parseStatementList(tokens)) return false;
    return verifyToken(tokens, nuPy_RIGHT_BRACE, "}");
}

static bool parseElse(struct TokenQueue* tokens) {
    struct Token currentToken = tokenqueue_peekToken(tokens);
    if (currentToken.id == nuPy_KEYW_ELIF) {
        tokenqueue_dequeue(tokens);
        if (!parseExpression(tokens)) return false;
        if (!verifyToken(tokens, nuPy_COLON, ":")) return false;
        if (!verifyToken(tokens, nuPy_EOLN, "EOLN")) return false;
        if (!parseBlock(tokens)) return false;
        struct Token nextToken = tokenqueue_peekToken(tokens);
        if (nextToken.id == nuPy_KEYW_ELIF || nextToken.id == nuPy_KEYW_ELSE) return parseElse(tokens);
        return true;
    } else if (currentToken.id == nuPy_KEYW_ELSE) {
        tokenqueue_dequeue(tokens);
        if (!verifyToken(tokens, nuPy_COLON, ":")) return false;
        if (!verifyToken(tokens, nuPy_EOLN, "EOLN")) return false;
        return parseBlock(tokens);
    }
    return false;
}

static bool parseIfElse(struct TokenQueue* tokens) {
    if (!verifyToken(tokens, nuPy_KEYW_IF, "if")) return false;
    if (!parseExpression(tokens)) return false;
    if (!verifyToken(tokens, nuPy_COLON, ":")) return false;
    if (!verifyToken(tokens, nuPy_EOLN, "EOLN")) return false;
    if (!parseBlock(tokens)) return false;
    struct Token currentToken = tokenqueue_peekToken(tokens);
    if (currentToken.id == nuPy_KEYW_ELIF || currentToken.id == nuPy_KEYW_ELSE) return parseElse(tokens);
    return true;
}

static bool parseStatementList(struct TokenQueue* tokens) {
    if (!parseStatement(tokens)) return false;
    if (isStatementStart(tokens)) return parseStatementList(tokens);
    return true;
}

static bool parseProgram(struct TokenQueue* tokens) {
    if (!parseStatementList(tokens)) return false;
    return verifyToken(tokens, nuPy_EOS, "$");
}

struct TokenQueue* parser_parse(FILE* input) {
    if (!input) {
        printf("**INTERNAL ERROR: null input stream\n");
        return NULL;
    }

    int line, column;
    char tokenValue[256];
    struct Token token;
    struct TokenQueue* tokens = tokenqueue_create();
    
    scanner_init(&line, &column, tokenValue);
    token = scanner_nextToken(input, &line, &column, tokenValue);

    while (token.id != nuPy_EOS) {
        tokenqueue_enqueue(tokens, token, tokenValue);
        token = scanner_nextToken(input, &line, &column, tokenValue);
    }

    tokenqueue_enqueue(tokens, token, tokenValue);
    struct TokenQueue* duplicate = tokenqueue_duplicate(tokens);
    bool success = parseProgram(tokens);

    tokenqueue_destroy(tokens);

    if (success) {
        return duplicate;
    } else {
        tokenqueue_destroy(duplicate);
        return NULL;
    }
}
