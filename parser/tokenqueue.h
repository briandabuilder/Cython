#ifndef TOKENQUEUE_H
#define TOKENQUEUE_H

#include "token.h"

struct TokenQueue;

// Creates a new token queue
struct TokenQueue* tokenqueue_create(void);

// Enqueues a token in the queue
void tokenqueue_enqueue(struct TokenQueue* queue, struct Token token, const char* value);

// Duplicates a token queue
struct TokenQueue* tokenqueue_duplicate(struct TokenQueue* queue);

// Peeks at the next token in the queue without dequeuing it
struct Token tokenqueue_peekToken(struct TokenQueue* queue);

// Peeks at the second token in the queue without dequeuing it
struct Token tokenqueue_peek2Token(struct TokenQueue* queue);

// Gets the value of the token at the front of the queue
char* tokenqueue_peekValue(struct TokenQueue* queue);

// Removes the token at the front of the queue
void tokenqueue_dequeue(struct TokenQueue* queue);

// Destroys a token queue and frees associated memory
void tokenqueue_destroy(struct TokenQueue* queue);

#endif // TOKENQUEUE_H
