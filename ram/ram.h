#ifndef RAM_H
#define RAM_H

#include <stdbool.h>

typedef enum {
    RAM_TYPE_NONE,
    RAM_TYPE_INT,
    RAM_TYPE_REAL,
    RAM_TYPE_STR,
    RAM_TYPE_PTR,
    RAM_TYPE_BOOLEAN
} RAM_VALUE_TYPE;

// Define the structure for RAM_VALUE, which can hold different types of values
struct RAM_VALUE {
    RAM_VALUE_TYPE value_type;
    union {
        int i;
        double d;
        char* s;
    } types;
};

// Define the structure for a memory cell, which has an identifier and a RAM_VALUE
struct RAM_CELL {
    char* identifier;
    struct RAM_VALUE value;
};

// Define the RAM structure, which includes a dynamic array of cells
struct RAM {
    int num_values;
    int capacity;
    struct RAM_CELL* cells;
};

// Function declarations for ram.c
struct RAM* ram_init(void);
void ram_destroy(struct RAM* memory);
int ram_get_addr(struct RAM* memory, char* identifier);
struct RAM_VALUE* ram_read_cell_by_addr(struct RAM* memory, int address);
struct RAM_VALUE* ram_read_cell_by_name(struct RAM* memory, char* name);
void ram_free_value(struct RAM_VALUE* value);
bool ram_write_cell_by_addr(struct RAM* memory, struct RAM_VALUE value, int address);
bool ram_write_cell_by_name(struct RAM* memory, struct RAM_VALUE value, char* name);
void ram_print(struct RAM* memory);

#endif // RAM_H
