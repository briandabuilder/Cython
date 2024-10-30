#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "token.h"
#include "scanner.h" 
#include "parser.h"

#include "programgraph.h" 
#include "ram.h"
#include "execute.h"


//
// main
// 
// If a filename is given, the file is opened and serves as
// input to the program. If a filename is not given, then 
// input is taken from the keyboard until $ is input.
//
int main(int argc, char* argv[])
{
  FILE* input = NULL;
  bool  keyboardInput = false;
  if (argc < 2) {
    input = stdin;
    keyboardInput = true;
  }
  else {
    char* filename = argv[1];
    input = fopen(filename, "r");
    if (input == NULL) {
      printf("**ERROR: unable to open input file '%s' for input.\n", filename);
      return 0;
    }

    keyboardInput = false;
  }

  if (keyboardInput)
  {
    printf("nuPython input (enter $ when you're done)>\n");
  }

  struct TokenQueue* tokens = parser_parse(input);

  if (tokens == NULL)
  {
    printf("**parsing failed...\n");
  }
  else
  {
    printf("**parsing successful, valid syntax\n");
    printf("**building program graph...\n");

    struct STMT* program = programgraph_build(tokens);
    printf("**executing...\n");
    struct RAM* memory = ram_init();
    execute(program, memory);
    printf("**done\n");
    ram_print(memory);
    programgraph_destroy(program);
    ram_destroy(memory);
    tokenqueue_destroy(tokens);
  }

  if (!keyboardInput)
    fclose(input);

  return 0;
}

