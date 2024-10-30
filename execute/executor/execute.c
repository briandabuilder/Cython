#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "execute.h"

static bool execute_function_call(struct STMT* stmt, struct RAM* memory);
static struct RAM_VALUE execute_get_value(struct UNARY_EXPR* unary, struct STMT* stmt, struct RAM* memory, bool* success);
static struct RAM_VALUE execute_binary_expression(struct RAM_VALUE lhs, int operator, struct RAM_VALUE rhs, struct STMT* stmt, bool* success);
static bool execute_assignment(struct STMT* stmt, struct RAM* memory);

//
// execute_function_call
//
// Executes a function call statement, returning true if 
// successful and false if not
//
static bool execute_function_call(struct STMT* stmt, struct RAM* memory)
{
  struct STMT_FUNCTION_CALL* call = stmt->types.function_call;
  char* function_name = call->function_name;
  assert(strcmp(function_name, "print") == 0);

  if (call->parameter == NULL)
    printf("\n");
  else {
    char* element_value = call->parameter->element_value;
    if (call->parameter->element_type == ELEMENT_STR_LITERAL) printf("%s\n", element_value);
    else if (call->parameter->element_type == ELEMENT_INT_LITERAL) {
      char* literal = element_value;
      int i = atoi(literal);
      printf("%d\n", i);
    }
    else if (call->parameter->element_type == ELEMENT_REAL_LITERAL) {
      char* literal = element_value;
      double i = atof(literal);
      printf("%lf\n", i);
    }
    else if (call->parameter->element_type == ELEMENT_TRUE || call->parameter->element_type == ELEMENT_FALSE) {
      char* literal = element_value;
      printf("%s\n", literal);
    }
    else {
      assert(call->parameter->element_type == ELEMENT_IDENTIFIER);
      char* var_name = element_value;
      struct RAM_VALUE* value = ram_read_cell_by_name(memory, var_name);
      if (value == NULL) {
        printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, stmt->line);
        return false;
      } // print depending on the value type
      else if (value->value_type == RAM_TYPE_INT) printf("%d\n", value->types.i);
      else if (value->value_type == RAM_TYPE_REAL) printf("%lf\n", value->types.d);
      else if (value->value_type == RAM_TYPE_STR) printf("%s\n", value->types.s);
      else if (value->value_type == RAM_TYPE_BOOLEAN && value->types.i == 1) printf("True\n");
      else if (value->value_type == RAM_TYPE_BOOLEAN && value->types.i == 0) printf("False\n");
    }
  }

  return true;
}


//
// execute_get_value
//
// Given a unary expr, returns the value that it represents.
//
static struct RAM_VALUE execute_get_value(struct UNARY_EXPR* unary, struct STMT* stmt, struct RAM* memory, bool* success)
{
  assert(unary->expr_type == UNARY_ELEMENT);

  struct RAM_VALUE value;  // initialize so we always return something

  struct ELEMENT* element = unary->element;

  if (element->element_type == ELEMENT_INT_LITERAL) {
    value.value_type = RAM_TYPE_INT;
    char* literal = element->element_value;

    value.types.i = atoi(literal);
    *success = true;
  }
  else if (element->element_type == ELEMENT_REAL_LITERAL) {
    value.value_type = RAM_TYPE_REAL;
    char* literal = element->element_value;
    double temp = atof(literal);
    value.types.d = temp;
    *success = true;
  }
  else if (element->element_type == ELEMENT_STR_LITERAL) {
    value.value_type = RAM_TYPE_STR;
    char* literal = element->element_value;
    value.types.s = literal;
    *success = true;
  }
  else if (element->element_type == ELEMENT_TRUE || element->element_type == ELEMENT_FALSE) {
    value.value_type = RAM_TYPE_BOOLEAN;
    if (strcmp(element->element_value, "True") == 0) value.types.i = 1;
    else value.types.i = 0;
    *success = true;
  }
  else {
    assert(element->element_type == ELEMENT_IDENTIFIER);
    char* var_name = element->element_value;
    struct RAM_VALUE* ram_value = ram_read_cell_by_name(memory, var_name);
    if (ram_value == NULL) {
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, stmt->line);
      *success = false;
    }
    else {
      if (ram_value->value_type == RAM_TYPE_INT) {
            value.value_type = RAM_TYPE_INT;
            value.types.i = ram_value->types.i;
            *success = true;
      }
      else if (ram_value->value_type == RAM_TYPE_BOOLEAN) {
          value.value_type = RAM_TYPE_BOOLEAN;
          value.types.i = ram_value->types.i;
          *success = true;
      }
      else if (ram_value->value_type == RAM_TYPE_REAL) {
        value.value_type = RAM_TYPE_REAL;
        value.types.d = ram_value->types.d;
        *success = true;
      }
      else if (ram_value->value_type == RAM_TYPE_STR) {
        value.value_type = RAM_TYPE_STR;
        value.types.s = ram_value->types.s;
        *success = true;
      }
      else {
        value.value_type = RAM_TYPE_INT;
        value.types.i = -1;
        *success = false;
      }
    }
  }

  return value;
}

//
// assign_types
//
// Given two elements, assign the resulting value type if the two elements were operated on
//
static void assign_types(struct RAM_VALUE lhs, int operator, struct RAM_VALUE rhs, struct RAM_VALUE* result) {
  bool is_rel_operator = operator == OPERATOR_EQUAL
                      || operator == OPERATOR_NOT_EQUAL
                      || operator == OPERATOR_LT
                      || operator == OPERATOR_LTE
                      || operator == OPERATOR_GT
                      || operator == OPERATOR_GTE;
  if (((lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) ||
      (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) ||
      (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) ||
      (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) ||
      (lhs.value_type == RAM_TYPE_STR && rhs.value_type == RAM_TYPE_STR)) && is_rel_operator) result->value_type = RAM_TYPE_BOOLEAN;
  else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) result->value_type = RAM_TYPE_INT;
  else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) result->value_type = RAM_TYPE_REAL;
  else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) result->value_type = RAM_TYPE_REAL;
  else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) result->value_type = RAM_TYPE_REAL;
  else if (lhs.value_type == RAM_TYPE_STR && rhs.value_type == RAM_TYPE_STR) result->value_type = RAM_TYPE_STR;
}

//
// calc_rel_perator
// Given two elements and a relational operator, calcuate the value of the expression
///
static void calc_rel_operator(struct RAM_VALUE lhs, int operator, struct RAM_VALUE rhs, struct RAM_VALUE* result, struct STMT* stmt, bool* success) {
  switch (operator)
  {
    case OPERATOR_EQUAL:
      if (lhs.value_type ==  RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.i == rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) result->types.i = lhs.types.d == rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) result->types.i = lhs.types.i == rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.d == rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_STR && rhs.value_type == RAM_TYPE_STR) result->types.i = strcmp(lhs.types.s, rhs.types.s) == 0;
      else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        *success = false;
      }
      break;
    case OPERATOR_NOT_EQUAL:
      if (lhs.value_type ==  RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.i != rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) result->types.i = lhs.types.d != rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) result->types.i = lhs.types.i != rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.d != rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_STR && rhs.value_type == RAM_TYPE_STR) result->types.i = strcmp(lhs.types.s, rhs.types.s) != 0;
      else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        *success = false;
      }
      break;
    case OPERATOR_LT:
      if (lhs.value_type ==  RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.i < rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) result->types.i = lhs.types.d < rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) result->types.i = lhs.types.i < rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.d < rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_STR && rhs.value_type == RAM_TYPE_STR) result->types.i = strcmp(lhs.types.s, rhs.types.s) < 0;
      else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        *success = false;
      }
      break;
    case OPERATOR_LTE:
      if (lhs.value_type ==  RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.i <= rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) result->types.i = lhs.types.d <= rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) result->types.i = lhs.types.i <= rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.d <= rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_STR && rhs.value_type == RAM_TYPE_STR) result->types.i = strcmp(lhs.types.s, rhs.types.s) <= 0;
      else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        *success = false;
      }
      break;
    case OPERATOR_GT:
      if (lhs.value_type ==  RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.i > rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) result->types.i = lhs.types.d > rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) result->types.i = lhs.types.i > rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.d > rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_STR && rhs.value_type == RAM_TYPE_STR) result->types.i = strcmp(lhs.types.s, rhs.types.s) > 0;
      else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        *success = false;
      }
      break;
    case OPERATOR_GTE:
      if (lhs.value_type ==  RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.i >= rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) result->types.i = lhs.types.d >= rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) result->types.i = lhs.types.i >= rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.d >= rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_STR && rhs.value_type == RAM_TYPE_STR) result->types.i = strcmp(lhs.types.s, rhs.types.s) >= 0;
      else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        *success = false;
      }
      break;
    default:
      printf("**SEMANTIC ERROR: invalid operator type (line %d)\n", stmt->line);
      *success = false;
      break;
  }
}
//
// calculate
//
// Given two elements and an operator, perform the operation between the two elements
//
static void calculate(struct RAM_VALUE lhs, int operator, struct RAM_VALUE rhs, struct RAM_VALUE* result, struct STMT* stmt, bool* success) {
  assign_types(lhs, operator, rhs, result);
  switch (operator)
  {
    case OPERATOR_PLUS:
      if (lhs.value_type ==  RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.i + rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) result->types.d = lhs.types.d + rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) result->types.d = lhs.types.i + rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) result->types.d = lhs.types.d + rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_STR && rhs.value_type == RAM_TYPE_STR) {
        char *s = malloc(strlen(rhs.types.s) + strlen(lhs.types.s) + 1);
        strcpy(s, lhs.types.s);
        strcat(s, rhs.types.s);
        result->types.s = s;
      }
      else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        *success = false;
      }
      break;
    case OPERATOR_MINUS:
      if (lhs.value_type ==  RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.i - rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) result->types.d = lhs.types.d - rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) result->types.d = lhs.types.i - rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) result->types.d = lhs.types.d - rhs.types.i;
      else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        *success = false;
      }
      break;
    case OPERATOR_ASTERISK:
      if (lhs.value_type ==  RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.i * rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) result->types.d = lhs.types.d * rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) result->types.d = lhs.types.i * rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) result->types.d = lhs.types.d * rhs.types.i;
      else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        *success = false;
      }
      break;
    case OPERATOR_POWER:
      if (lhs.value_type ==  RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) result->types.i = (int) pow(lhs.types.i, rhs.types.i);
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type ==RAM_TYPE_REAL) result->types.d = pow(lhs.types.d, rhs.types.d);
      else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) result->types.d = pow(lhs.types.i, rhs.types.d);
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) result->types.d = pow(lhs.types.d, rhs.types.i);
      else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        *success = false;
      }
      break;
    case OPERATOR_MOD:
      if (lhs.value_type ==  RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.i % rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) result->types.d = fmod(lhs.types.d, rhs.types.d);
      else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) result->types.d = fmod(lhs.types.i, rhs.types.d);
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) result->types.d = fmod(lhs.types.d, rhs.types.i);
      else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        *success = false;
      }
      break;
    case OPERATOR_DIV:
      if (lhs.value_type ==  RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) result->types.i = lhs.types.i / rhs.types.i;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) result->types.d = lhs.types.d / rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_REAL) result->types.d = lhs.types.i / rhs.types.d;
      else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_INT) result->types.d = lhs.types.d / rhs.types.i;
      else {
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        *success = false;
      }
      break;
    case OPERATOR_EQUAL:
      calc_rel_operator(lhs, operator, rhs, result, stmt, success);
      break;
    case OPERATOR_NOT_EQUAL:
      calc_rel_operator(lhs, operator, rhs, result, stmt, success);
      break;
    case OPERATOR_LT:
      calc_rel_operator(lhs, operator, rhs, result, stmt, success);
      break;
    case OPERATOR_LTE:
      calc_rel_operator(lhs, operator, rhs, result, stmt, success);
      break;
    case OPERATOR_GT:
      calc_rel_operator(lhs, operator, rhs, result, stmt, success);
      break;
    case OPERATOR_GTE:
      calc_rel_operator(lhs, operator, rhs, result, stmt, success);
      break;
    default:
      printf("**SEMANTIC ERROR: invalid operator type (line %d)\n", stmt->line);
      break;
  }
}

//
// execute_binary_expression
//
// Given two values and an operator, performs the operation
// and returns the result.
//
static struct RAM_VALUE execute_binary_expression(struct RAM_VALUE lhs, int operator, struct RAM_VALUE rhs, struct STMT* stmt, bool* success)
{
  assert(operator != OPERATOR_NO_OP);
  struct RAM_VALUE result;
  if (operator == OPERATOR_PLUS || 
      operator == OPERATOR_MINUS || 
      operator == OPERATOR_ASTERISK || 
      operator == OPERATOR_POWER || 
      operator == OPERATOR_MOD || 
      operator == OPERATOR_DIV ||
      operator == OPERATOR_EQUAL ||
      operator == OPERATOR_NOT_EQUAL ||
      operator == OPERATOR_LT ||
      operator == OPERATOR_LTE ||
      operator == OPERATOR_GT ||
      operator == OPERATOR_GTE) calculate(lhs, operator, rhs, &result, stmt, success);
  else {
    printf("**INTERNAL ERROR: unexpected operator (%d) in execute_binary_expr\n", operator);
    assert(false);
  }
  return result;
}

//
// input_function
//
// Given a message to tell the user, receive an input and store it to memeory
//
static struct RAM_VALUE input_function(char* msg, struct RAM* memory, struct STMT* stmt, char* var_name) {
  struct RAM_VALUE result;
  result.value_type = RAM_TYPE_STR;
  printf("%s ", msg);
  char *line = malloc(256);
  char* literal = fgets(line, 256, stdin);
  line[strcspn(line, "\r\n")] = '\0';
  result.types.s = literal;
  ram_write_cell_by_name(memory, result, var_name);
  return result;
}

//
// int_function
//
// Given a value stored in memory, convert it to an integer.
//
static struct RAM_VALUE int_function(struct RAM_VALUE* val, struct RAM* memory, struct STMT* stmt, bool* mini_success) {
  struct RAM_VALUE result;
  struct RAM_VALUE v1;
  v1.value_type = RAM_TYPE_STR;
  v1.types.s = val->types.s;
  bool is_zero_string = true;
  char *temp_string = v1.types.s;
  while (*temp_string != '\0') {
    if (*temp_string != '0') {
      is_zero_string = false;
      break;
    }
    temp_string++;
  }
  if (is_zero_string) {
    result.value_type = RAM_TYPE_INT;
    result.types.i = 0;
    *mini_success = true;
  }
  else if (atoi(v1.types.s) != 0) {
    result.value_type = RAM_TYPE_INT;
    result.types.i = atoi(v1.types.s);
    *mini_success = true;
  }
  else {
    result.value_type = RAM_TYPE_INT;
    result.types.i = -1;
    *mini_success = false;
    printf("**SEMANTIC ERROR: invalid string for int() (line %d)\n", stmt->line);
  }
  return result;
}

//
// float_function
//
// Given a value stored in memory, convert it to a float.
//
static struct RAM_VALUE float_function(struct RAM_VALUE* val, struct RAM* memory, struct STMT* stmt, bool* mini_success) {
  struct RAM_VALUE result;
  struct RAM_VALUE v2;
  v2.value_type = RAM_TYPE_STR;
  v2.types.s = val->types.s;
  bool is_zero_string = true;
  char *temp_string = v2.types.s;
  while (*temp_string != '\0') {
    if (*temp_string != '0' && *temp_string != '.') {
      is_zero_string = false;
      break;
    }
    temp_string++;
  }
  if (is_zero_string) {
    result.value_type = RAM_TYPE_REAL;
    result.types.d = (double) 0;
    *mini_success = true;
  }
  // Perform the conversion
  else if (atof(v2.types.s) != 0) {
    result.value_type = RAM_TYPE_REAL;
    result.types.d = atof(v2.types.s);
    *mini_success = true;
  }
  else {
    result.value_type = RAM_TYPE_REAL;
    result.types.d = -1.0;
    *mini_success = false;
    printf("**SEMANTIC ERROR: invalid string for float() (line %d)\n", stmt->line);
  }
  return result;
}

//
// execute_assignment
//
// Executes an assignment statement, returning true if 
// successful and false if not
static bool execute_assignment(struct STMT* stmt, struct RAM* memory)
{
  struct STMT_ASSIGNMENT* assign = stmt->types.assignment;
  char* var_name = assign->var_name;
  assert(assign->isPtrDeref == false);
  bool success;
  struct RAM_VALUE  result;

  if (assign->rhs->value_type == VALUE_EXPR) {
    assert(assign->rhs->value_type == VALUE_EXPR);

    struct EXPR* expr = assign->rhs->types.expr;
    assert(expr->lhs != NULL);
    struct RAM_VALUE lhs_value = execute_get_value(expr->lhs, stmt, memory, &success);
    if (!success)  // semantic error
      return false;
    if (!expr->isBinaryExpr) {
      result = lhs_value;
    }
    else {
      assert(expr->operator != OPERATOR_NO_OP);  // we must have an operator
      struct RAM_VALUE rhs_value = execute_get_value(expr->rhs, stmt, memory, &success);
      if (!success)  // semantic error
        return false;
      result = execute_binary_expression(lhs_value, expr->operator, rhs_value, stmt, &success);
      if (!success) return false;
    }
  }
  else {
    assert(assign->rhs->value_type == VALUE_FUNCTION_CALL);
    char* func_name = assign->rhs->types.function_call->function_name; // Check what type of function it is
    if (strcmp(func_name, "input") == 0) result = input_function(assign->rhs->types.function_call->parameter->element_value, memory, stmt, var_name); 
    else if (strcmp(func_name, "int") == 0) {
      struct RAM_VALUE* temp = ram_read_cell_by_name(memory, assign->rhs->types.function_call->parameter->element_value);
      if (temp == NULL) return false; // the value doesn't exist in memory
      result = int_function(temp, memory, stmt, &success);
      if (!success) return false;
    }
    else if (strcmp(func_name, "float") == 0) {
      struct RAM_VALUE* temp = ram_read_cell_by_name(memory, assign->rhs->types.function_call->parameter->element_value);
      if (temp == NULL) return false; // the value doesn't exist in memory
      result = float_function(temp, memory, stmt, &success);
      if (!success) return false;
    }
    else {
      printf("**SEMANTIC ERROR: invalid function name (line %d)", stmt->line);
      return false;
    }
  }
  success = ram_write_cell_by_name(memory, result, var_name);
  return success;
}

// execute
//
// Given a CPython program graph and a memory, 
// executes the statements in the program graph
// If a semantic error occurs, an error message is 
// output, execution stops, and the function returns
//
void execute(struct STMT* program, struct RAM* memory)
{
  struct STMT* stmt = program;
  while (stmt != NULL) {
    if (stmt->stmt_type == STMT_ASSIGNMENT) {
      bool success = execute_assignment(stmt, memory);
      if (!success)
        return;
      stmt = stmt->types.assignment->next_stmt;  // advance
    }
    else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
      bool success = execute_function_call(stmt, memory);
      if (!success)
        return;
      stmt = stmt->types.function_call->next_stmt;
    }
    else if (stmt->stmt_type == STMT_WHILE_LOOP) {
      bool success;
      struct RAM_VALUE condition;
      // use execute binary_expression and execute_get_value to check the while loop condition
      if (!stmt->types.while_loop->condition->isBinaryExpr) condition = execute_get_value(stmt->types.while_loop->condition->lhs, stmt, memory, &success);
      else condition = execute_binary_expression(execute_get_value(stmt->types.while_loop->condition->lhs, stmt, memory, &success), stmt->types.while_loop->condition->operator, execute_get_value(stmt->types.while_loop->condition->rhs, stmt, memory, &success), stmt, &success);
      if (!success) return;
      if (condition.types.i == 1) { // recursively call execute to simulate while loop
        execute(stmt->types.while_loop->loop_body, memory);
        return;
      }
      if (!success) return;
      stmt = stmt->types.while_loop->next_stmt;
    }
    else {
      assert(stmt->stmt_type == STMT_PASS);
      stmt = stmt->types.pass->next_stmt;
    }
  }

  return;
}