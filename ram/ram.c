#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "ram.h"

//
// ram_init
//
// Returns a pointer to a dynamically-allocated memory
// for storing nuPython variables and their values. All
// memory cells are initialized to the value None.
//
struct RAM* ram_init(void)
{
  struct RAM* ram = (struct RAM*)malloc(sizeof(struct RAM));
  ram->num_values = 0;
  ram->capacity = 4;
  ram->cells = (struct RAM_CELL*)malloc(ram->capacity * sizeof(struct RAM_CELL));

  for (int i = 0; i < ram->capacity; i++) {
    ram->cells[i].identifier = NULL;
    ram->cells[i].value.value_type = RAM_TYPE_NONE;
  }
  
  return ram;
}


//
// ram_destroy
//
// Frees the dynamically-allocated memory associated with
// the given memory.
//
void ram_destroy(struct RAM* memory)
{
  for (int i = 0; i < memory->num_values; i++) {
    if (memory->cells[i].value.value_type == RAM_TYPE_STR) free(memory->cells[i].value.types.s);
    free(memory->cells[i].identifier);
  }
  free(memory->cells);
  free(memory);
  return;
}


//
// ram_get_addr
// 
// If the given identifier has been written to 
// memory, return the address of this value 
// Returns -1 if no such identifier exists 
// in memory. 
//
int ram_get_addr(struct RAM* memory, char* identifier)
{
  for (int i = 0; i < memory->num_values; i++) {
    if (memory->cells[i].identifier != NULL && strcmp(memory->cells[i].identifier, identifier) == 0) return i;
  }
  return -1;
}


//
// ram_read_cell_by_addr
//
// Given a memory address returns a COPY of 
// the value contained in that memory cell.
// Returns null if the address is not valid.
//
struct RAM_VALUE* ram_read_cell_by_addr(struct RAM* memory, int address)
{
  if (address < 0 || address >= memory->num_values) return NULL;
  
  struct RAM_VALUE* value = (struct RAM_VALUE*)malloc(sizeof(struct RAM_VALUE));
  *value = memory->cells[address].value;
  if (value->value_type == RAM_TYPE_STR) {
    char* temp_string = (char*)malloc(strlen(memory->cells[address].value.types.s) + 1);
    strcpy(temp_string, memory->cells[address].value.types.s);
    value->types.s = temp_string;
  }

  return value;
}


// 
// ram_read_cell_by_name
//
// If the given name has been written to 
// memory, returns a copy of the value contained in memory.
// Returns null if no such name exists in memory.
//
struct RAM_VALUE* ram_read_cell_by_name(struct RAM* memory, char* name)
{
  int index = ram_get_addr(memory, name);
  if (index == -1) return NULL;
  return ram_read_cell_by_addr(memory, index);
}


//
// ram_free_value
//
// Frees the memory value returned by ram_read_cell_by_name and
// ram_read_cell_by_addr.
//
void ram_free_value(struct RAM_VALUE* value)
{
  if (value == NULL) return;
  if (value->value_type == RAM_TYPE_STR) free(value->types.s);
  free(value);
  return;
}


//
// ram_write_cell_by_addr
//
// Writes the given value to the memory cell at the given 
// address. If a value already exists at this address, that
// value is overwritten by this new value. Returns true if 
// the value was successfully written, false if not
//
bool ram_write_cell_by_addr(struct RAM* memory, struct RAM_VALUE value, int address) {
  if (address < 0 || address >= memory->capacity || memory->cells[address].identifier == NULL) return false;
  if (memory->cells[address].value.value_type == RAM_TYPE_STR) free(memory->cells[address].value.types.s);

  memory->cells[address].value.value_type = value.value_type;
  if (value.value_type == RAM_TYPE_INT || value.value_type == RAM_TYPE_BOOLEAN || value.value_type == RAM_TYPE_PTR) memory->cells[address].value.types.i = value.types.i;
  else if (value.value_type == RAM_TYPE_REAL) memory->cells[address].value.types.d = value.types.d;
  else if (value.value_type == RAM_TYPE_STR) {
    char* temp_string = (char*)malloc(strlen(value.types.s) + 1);
    strcpy(temp_string, value.types.s);
    memory->cells[address].value.types.s = temp_string;
  }
  return true;
}


//
// ram_write_cell_by_name
//
// Writes the given value to a memory cell named by the given
// name. If a memory cell already exists with this name, the
// existing value is overwritten by the given value. Returns
// true since this operation always succeeds.
//
bool ram_write_cell_by_name(struct RAM* memory, struct RAM_VALUE value, char* name)
{
  int index = ram_get_addr(memory, name);
  if (index != -1) return ram_write_cell_by_addr(memory, value, index);

  if (memory->num_values >= memory->capacity) {
    memory->capacity *= 2;
    memory->cells = (struct RAM_CELL*)realloc(memory->cells, memory->capacity * sizeof(struct RAM_CELL));
    for (int i = memory->num_values; i < memory->capacity; i++) {
      memory->cells[i].identifier = NULL;
      memory->cells[i].value.value_type = RAM_TYPE_NONE;
    }
  }
  memory->num_values++;
  char* temp_identifier = (char*)malloc(strlen(name) + 1);
  strcpy(temp_identifier, name);
  memory->cells[memory->num_values-1].identifier = temp_identifier;
  return ram_write_cell_by_addr(memory, value, memory->num_values-1);
}


//
// ram_print
//
// Prints the contents of memory to the console.
//
void ram_print(struct RAM* memory)
{
  printf("**MEMORY PRINT**\n");

  printf("Capacity: %d\n", memory->capacity);
  printf("Num values: %d\n", memory->num_values);
  printf("Contents:\n");

  for (int i = 0; i < memory->num_values; i++)
  {
      printf(" %d: %s, ", i, memory->cells[i].identifier);
      if (memory->cells[i].value.value_type == RAM_TYPE_INT) printf("int, %d", memory->cells[i].value.types.i);
      else if (memory->cells[i].value.value_type == RAM_TYPE_REAL) printf("real, %lf", memory->cells[i].value.types.d);
      else if (memory->cells[i].value.value_type == RAM_TYPE_STR) printf("str, '%s'", memory->cells[i].value.types.s);
      else if (memory->cells[i].value.value_type == RAM_TYPE_PTR) printf("ptr, %d", memory->cells[i].value.types.i);
      else if (memory->cells[i].value.value_type == RAM_TYPE_BOOLEAN && memory->cells[i].value.types.i == 0) printf("boolean, False");
      else if (memory->cells[i].value.value_type == RAM_TYPE_BOOLEAN && memory->cells[i].value.types.i == 1) printf("boolean, False");
      else if (memory->cells[i].value.value_type == RAM_TYPE_NONE) printf("none, None");
      printf("\n");
  }

  printf("**END PRINT**\n");
}