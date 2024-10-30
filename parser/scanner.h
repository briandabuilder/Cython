#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include "token.h"

// Initializes the scanner with the given line, column, and value references
void scanner_init(int* line, int* column, char* tokenValue);

// Retrieves the next token from the input stream
struct Token scanner_nextToken(FILE* input, int* line, int* column, char* tokenValue);

#endif // SCANNER_H
