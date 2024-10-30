#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "programgraph.h"
#include "ram.h"
#include "execute.h"

//
// Used to help return 2 values in the execute_binary_expression function
//
struct RESULT {
    bool success;
    int result;
};

//
// Retrieves an integer representing the value specified
//
int retrieve_value(struct UNARY_EXPR* value, struct RAM* memory) {
  // if value isn't in memory (check one at a time), return -1
  if (value->element->element_type == ELEMENT_IDENTIFIER &&
      ram_read_cell_by_name(memory, value->element->element_value) == NULL) return -1;
  else if (value->element->element_type == ELEMENT_IDENTIFIER) return ram_read_cell_by_name(memory, value->element->element_value)->types.i;
  // else if the element is an identifier, get its value
  // return as an integer
  return atoi(value->element->element_value);
}

//
// Given an expression as well as the memory of the computer,
// calculate the value of the binary expression
//
struct RESULT execute_binary_expression(struct EXPR* expr, struct RAM* memory) {
  struct RESULT r;

  // get lhs, rhs
  int a = retrieve_value(expr->lhs, memory);
  int b = retrieve_value(expr->rhs, memory);

  if (a == -1) {
    r.result = -1;
    r.success = false;
    return r;
  }
  if (b == -1) {
    r.result = -2;
    r.success = false;
    return r;
  }

  if      (expr->operator == OPERATOR_PLUS) r.result = a + b;
  else if (expr->operator == OPERATOR_MINUS) r.result = a - b;
  else if (expr->operator == OPERATOR_ASTERISK) r.result = a * b;
  else if (expr->operator == OPERATOR_DIV) r.result = a / b;
  else if (expr->operator == OPERATOR_MOD) r.result = a % b;
  else if (expr->operator == OPERATOR_POWER) r.result = pow(a, b);
  r.success = true;
  return r;
}

//
// Given a statement and the computer memory,
// call the function with the specified input
// to print out the input.
//
bool execute_function_call(struct STMT* statement, struct RAM* memory) {
  if (statement->types.function_call->parameter == NULL) {
    printf("\n");
    return true;
  }
  char* param = statement->types.function_call->parameter->element_value;
  if (statement->types.function_call->parameter->element_type == ELEMENT_INT_LITERAL) {
    int int_literal = atoi(param);
    printf("%d\n", int_literal);
    return true;
  }
  else if (statement->types.function_call->parameter->element_type == ELEMENT_IDENTIFIER) {
    if (ram_read_cell_by_name(memory, statement->types.function_call->parameter->element_value) == NULL) {
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", statement->types.function_call->parameter->element_value, statement->line);
      return false;
    }
    int num = ram_read_cell_by_name(memory, statement->types.function_call->parameter->element_value)->types.i;
    printf("%d\n", num);
    return true;
  }
  if (strlen(param) == 0) printf("\n");
  else printf("%s\n", param);
  return true;
}

//
// Given a statement and the computer memory,
// execute assignment statements that can contain
// integers, or variables storing integers
//
bool execute_assignment(struct STMT* statement, struct RAM* memory) {
  // Check if it's an identifier first
  if (statement->types.assignment->rhs->types.expr->lhs->element->element_type == ELEMENT_IDENTIFIER &&
      ram_read_cell_by_name(memory, statement->types.assignment->rhs->types.expr->lhs->element->element_value) == NULL) {
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", statement->types.assignment->rhs->types.expr->lhs->element->element_value, statement->line);
      return false;
  }

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_INT;
  if (statement->types.assignment->rhs->types.expr->rhs != NULL) {
    struct RESULT temp = execute_binary_expression(statement->types.assignment->rhs->types.expr, memory);
    if (!temp.success) {
      char* identifier;
      if (temp.result == -1) identifier = statement->types.assignment->rhs->types.expr->lhs->element->element_value;
      else identifier = statement->types.assignment->rhs->types.expr->rhs->element->element_value;
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", identifier, statement->line);
      return false;
    }
    value.types.i = temp.result;
  }
  else {
    char* literal = statement->types.assignment->rhs->types.expr->lhs->element->element_value;
    value.types.i = atoi(literal);
  }
  ram_write_cell_by_name(memory, value, statement->types.assignment->var_name);
  return true;
}

// execute
//
// Given a Cython program graph and a memory, 
// executes the statements in the program graph.
// If a semantic error occurs (e.g. type error),
// and error message is output, execution stops,
// and the function returns.
//
void execute(struct STMT* program, struct RAM* memory)
{
  struct STMT* stmt = program;

  while (stmt != NULL) {
    if (stmt->stmt_type == STMT_ASSIGNMENT) {
      int line = stmt->line;
      char* var_name = stmt->types.assignment->var_name;
      bool temp = execute_assignment(stmt, memory);
      if (!temp) break;
      stmt = stmt->types.assignment->next_stmt;
    }
    else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
      int line = stmt->line;
      char* func_name = stmt->types.function_call->function_name;
      bool temp = execute_function_call(stmt, memory);
      if (!temp) break;
      stmt = stmt->types.function_call->next_stmt;
    } 
    else {
      assert(stmt->stmt_type == STMT_PASS);
      stmt = stmt->types.pass->next_stmt;
    }
  }
  }