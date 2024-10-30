#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>
#include "token.h"
#include "tokenqueue.h"

// Initializes the parsing process and returns a queue of tokens representing the parsed program.
// If parsing fails, returns NULL.
struct TokenQueue* parser_parse(FILE* input);

#endif // PARSER_H
