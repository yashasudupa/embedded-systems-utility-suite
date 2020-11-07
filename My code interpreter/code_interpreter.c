// void brainfuck(const char *code, const char *input, char *output)
// code - The Brainfuck program to be executed
// input - A stream of input bytes to the Brainfuck program being executed
// output - A writable character buffer large enough to hold the expected output of the
//          Brainfuck program being executed
// Your task: Execute the Brainfuck program with the given input, writing the program output
// to the output buffer provided as a NUL-terminated C-string

#include <string.h>
#define LENGTH 256
#include <stdlib.h>
#include <stdio.h>

struct dpointer{
  char* data_pointer;
  char* left_bound;
  char* right_bound;
};

struct dpointer expand_memory_left(struct dpointer data_pointer) {
  int dptr_offset = data_pointer.right_bound - data_pointer.data_pointer;
  int block_size_old = data_pointer.right_bound - data_pointer.left_bound;
  int block_size_new = block_size_old + block_size_old/2;
  struct dpointer return_data_pointer;
  return_data_pointer.left_bound = realloc(data_pointer.left_bound, block_size_new);
  return_data_pointer.right_bound = return_data_pointer.left_bound + block_size_new;
  return_data_pointer.data_pointer = return_data_pointer.right_bound - dptr_offset;
  
  return return_data_pointer;
}

struct dpointer expand_memory_right(struct dpointer data_pointer) {
  int dptr_offset = data_pointer.data_pointer - data_pointer.left_bound;
  int block_size_old = data_pointer.right_bound - data_pointer.left_bound;
  int block_size_new = block_size_old + block_size_old/2;
  struct dpointer return_data_pointer;
  return_data_pointer.left_bound = realloc(data_pointer.left_bound, block_size_new);
  return_data_pointer.right_bound = return_data_pointer.left_bound + block_size_new;
  return_data_pointer.data_pointer = return_data_pointer.left_bound + dptr_offset;
  
  return return_data_pointer;
}

struct dpointer increment_data_pointer(struct dpointer data_pointer) {
  struct dpointer return_data_pointer = data_pointer;
  if (data_pointer.data_pointer < data_pointer.right_bound - 1) {
    return_data_pointer.data_pointer += 1;
  }
  else {
    return_data_pointer = expand_memory_right(return_data_pointer);
    return_data_pointer.data_pointer += 1;
  }
  return return_data_pointer;
}

struct dpointer decrement_data_pointer(struct dpointer data_pointer) {
  struct dpointer return_data_pointer = data_pointer;
  if (data_pointer.data_pointer > data_pointer.left_bound) {
    return_data_pointer.data_pointer -= 1;
  }
  else {
    return_data_pointer = expand_memory_left(return_data_pointer);
    return_data_pointer.data_pointer -= 1;
  }
  return return_data_pointer;
}

const char* forward_bracket_handler(struct dpointer data_pointer, const char* token){

  int fwd_bracket_count = 0;
  
  if (*data_pointer.data_pointer == 0) {
    do {
      if ( *token == '[') {
        fwd_bracket_count++;
      }
      else if ( *token == ']' ) {
        fwd_bracket_count--;
      }
      token++;
    } while (fwd_bracket_count > 0);
    return token;
  }
  return ++token;
 }

const char* reverse_bracket_handler(struct dpointer data_pointer, const char* token){

  int rvs_bracket_count = 0;
  if(*data_pointer.data_pointer != 0){
    do{
      if(*token == ']'){
        rvs_bracket_count++;
      }
      else if(*token == '['){
        rvs_bracket_count--;
      }
      token--;
    } while (rvs_bracket_count > 0);
    token += 2;
    return token;
  }
  return ++token;
}

void brainfuck(const char *code, const char *input, char *output) {

  char *data = (char *) malloc(3 * sizeof(char));
  
  char* data_bound_left = data;
  char* data_bound_right = data_bound_left + 3 * sizeof(char);
  
  struct dpointer data_pointer;
  data_pointer.data_pointer = data;
  data_pointer.left_bound = data_bound_left;
  data_pointer.right_bound = data_bound_right;

  const char* token = code;
  
  while(*token != '\0'){
    switch(*token){
        case '>':
          data_pointer=increment_data_pointer(data_pointer);
          token++;
          break;
        case '<':
          data_pointer=decrement_data_pointer(data_pointer);
          token++;
          break;
        case '+':
          *data_pointer.data_pointer += 1;
          token++;
          break;
        case '-':
          *data_pointer.data_pointer -= 1;
          token++;
          break;
        case '.':
          *output = *data_pointer.data_pointer;
          output++;
          token++;
          break; 
        case ',':
          *data_pointer.data_pointer = *input;
          input++;
          token++;
          break;
        case '[':
          token = forward_bracket_handler(data_pointer, token);
          break;
        case ']':
          token = reverse_bracket_handler(data_pointer, token);
          break;
    }
  }
  *output = '\0';
}