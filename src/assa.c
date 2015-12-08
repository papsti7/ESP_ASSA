//------------------------------------------------------------------------------
// assa.c
//
// This program is intended to interprete Brainfuck files and debug them.
//
// Gruppenbeispiel A
//
// Group: 4 study assistant Michael Schwarz
//
// Authors: Stefan Papst 1430868
//          Harald Deutschmann XXXXXX
//          Julia Heritsch XXXXXX
//
// Latest Changes: 08.12.2015 (by Stefan Papst)
//------------------------------------------------------------------------------
//
/*this is a special flag in visual studio for secure function errors/ Can be commented if there is a error in gcc*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define FAILED -1
#define SUCCESS 0
#define FALSE_ARGUMENTS 1
#define OUT_OF_MEMORY 2
#define PARSE_FILE_ERROR 3
#define READING_FILE_FAIL 4
#define END_OF_FILE 5

#define TRUE 0
#define FALSE 1
#define NEUTRAL -1

#define OPEN 0
#define CLOSE 1

#define COMMAND 0
#define ARG 1


//this struct contains all information about the program
typedef struct {
  unsigned char* data_segment_;
  unsigned char* code_segment_;
  int* break_points_;
  int code_segment_size_;
  int data_segment_size_;
  int code_length_;
  int number_of_loops_;
  int last_stop_in_code;
  unsigned char* last_program_counter_stop_;
  int program_counter_index_;
  int data_loaded_;
  int** bracket_index_;
  int end_reached_;
  int step_counter_;
} Environment;
//this struct saves the command line
typedef struct {
  char* command_;
  char** args_;
  int args_count_;
} Input;
//this struct is used for the evalCommand
typedef struct {
  int insert_pos_in_string_;
  int insert_string_len_;
  unsigned char* string_to_insert_;
  int number_of_loops_;
  int** bracket_index_;
} EvalData;


/*------------HEADER NEED TO BE DONE-----------*/
int readCodeFromFile(Environment* data, char* name);
int checkCodeCorrectness(unsigned char* code_segment, int* number_of_loops);
int checkCommandOrComment(unsigned char current_char);
int runCode(Environment* data, int** bracket_index, int number_of_loops);
int parseCode(unsigned char* code_segment, int*** brackets, int number_of_loops);
int getIndexOfBracket(int** bracket_index, int size_of_array, int current_command_counter, int open_or_close);
int createBracketIndex(int*** bracket_index, int number_of_loops);
int checkIfEqalWithBreakPoint(int current_command_counter, int** break_points);
int getCommandAndArgs(Input* input);
int deleteBracketIndex(int*** bracket_index, int number_of_loops);
int setBreakPoint(int** break_points, int point_pos);
int checkSteps(int* steps);
int showCode(unsigned char* code_segment, int current_command_counter, int* number_to_show);
int insertString(unsigned char** string, EvalData* eval_data);
int cutOutString(unsigned char** string, EvalData* eval_data, int* last_stop_in_code);
int initData(Environment* data);
int showMemory(Environment* data, int position, char* type);
int changeMemory(Environment* data, int position, unsigned int value);
void toBinary(int value, int bitsCount, char* output);
void freeInput(Input* input);
int loadCommand(Input* input, Environment* data);
void runCommand(Environment* data);
int breakCommand(Input* input, Environment* data);
void stepCommand(Input* input, Environment* data);
void showCommand(Input* input, Environment* data);
int evalCommand(Input* input, Environment* data, EvalData* eval_data);
int memoryCommand(Input* input, Environment* data);
int changeCommand(Input* input, Environment* data);
void handleDebugMode(Environment* data, EvalData* eval_data, int* return_value);
void handleNormalMode(Environment* data, int* return_value, char* argv[]);
int saveChartoCommandorArg(Input* input, char current_char, int* shell_count, int* allocated_size_for_string, int command_or_arg);

//-----------------------------------------------------------------------------
///
/// The main program
/// This function checks, if the program is started in normal mode or in debug
/// mode and then starts the right one.
///
/// @param argc The counter how many arguments are written on the commandline
/// @param argv The value of the command line arguments
///
/// @return 0 SUCCESS 
/// @return 1 FALSE_ARGUMENTS 
/// @return 2 OUT_OF_MEMORY 
/// @return 3 PARSE_FILE_ERROR 
/// @return 4 READING_FILE_FAIL 
//
int main(int argc, char* argv[])
{
  Environment data;
  data.break_points_ = NULL;
  data.bracket_index_ = NULL;
  data.code_segment_ = NULL;
  data.data_segment_ = NULL;
  data.data_loaded_ = FALSE;
  
  EvalData eval_data;
  eval_data.insert_pos_in_string_ = NEUTRAL;
  eval_data.insert_string_len_ = NEUTRAL;
  eval_data.string_to_insert_ = NULL;
  eval_data.bracket_index_ = NULL;
  eval_data.number_of_loops_ = NEUTRAL;
  
  int return_value = SUCCESS;

  if (argc == 3)
  {
    handleNormalMode(&data, &return_value, argv);
  }
  else if (argc == 1)
  {
    handleDebugMode(&data, &eval_data, &return_value);
  }
  else
  {
    printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
    return FALSE_ARGUMENTS;
  } 
  
  if (data.data_loaded_ == TRUE)
  {
   initData(&data);
   free(data.data_segment_);
   data.data_segment_ = NULL;
   data.data_segment_size_ = 0;
  }
  
  if (data.break_points_ != NULL)
  {
    free(data.break_points_);
    data.break_points_ = NULL;
  }
  
  return return_value;
}

int readCodeFromFile(Environment* data, char* name)
{
  //open file
  FILE* file;
  file = fopen(name, "r");;
  if (file == NULL)
  {
    printf("[ERR] reading the file failed\n");
    return READING_FILE_FAIL;
  }
  
  data->code_segment_ = (unsigned char*)calloc(500, sizeof(unsigned char));
  if (data->code_segment_ == NULL)
  {
    printf("[ERR] reading the file failed\n");
    return READING_FILE_FAIL;
  }

  data->code_segment_size_ = 500;
  data->code_length_ = 0;
  int current_char;
  int counter = 0;
  while ((current_char = getc(file)) != EOF)
  {
    if (checkCommandOrComment(current_char) == TRUE)
    {//-85 because of \0 and eval string
      if (counter < (data->code_segment_size_ - 85))
      {
        (data->code_segment_)[counter] = current_char;
        (data->code_segment_)[counter + 1] = '\0';

      }
      else
      {
        unsigned char* new_data_segment = NULL;
        data->code_segment_size_ *= 2;

        new_data_segment = (unsigned char*) realloc(data->code_segment_, data->code_segment_size_);
        if (new_data_segment == NULL)
        {
          printf("[ERR] out of memory\n");
          return OUT_OF_MEMORY;
        }
        unsigned char* start = &(new_data_segment[data->code_segment_size_ / 2]);
        memset(start, 0, data->code_segment_size_ / 2);

        data->code_segment_ = new_data_segment;
        (data->code_segment_)[counter] = current_char;
        (data->code_segment_)[counter + 1] = '\0';
      }
      counter++;
      data->code_length_++;
    }
  }
  fclose(file);
  data->data_loaded_ = TRUE;
  
  return SUCCESS;
}

int checkCodeCorrectness(unsigned char* code_segment, int* number_of_loops)
{
  int counter = 0;
  int open_brackets_count = 0;
  int close_brackets_count = 0;
  while (code_segment[counter] != '\0')
  {
    if (code_segment[counter] == '[')
      open_brackets_count++;
    else if (code_segment[counter] == ']')
      close_brackets_count++;
    counter++;
  }
  *number_of_loops = open_brackets_count;
  return open_brackets_count - close_brackets_count;
}

int checkCommandOrComment(unsigned char current_char)
{
  switch (current_char)
  {
  case '<':
    return TRUE;
    break;
  case '>':
    return TRUE;
    break;
  case '+':
    return TRUE;
    break;
  case '-':
    return TRUE;
    break;
  case '.':
    return TRUE;
    break;
  case ',':
    return TRUE;
    break;
  case '[':
    return TRUE;
    break;
  case ']':
    return TRUE;
    break;
  default:
    return FALSE;
    break;
  }
}

int runCode(Environment* data, int** bracket_index, int number_of_loops)
{
  unsigned char* program_counter = data->data_segment_ + data->program_counter_index_;
  int current_command_counter = data->last_stop_in_code;
    
  while ((data->code_segment_)[current_command_counter] != '\0')
  { 
    if (checkIfEqalWithBreakPoint(current_command_counter, &data->break_points_) == TRUE || checkSteps(&data->step_counter_) == TRUE)
    {
      break;
    }
  
    if (program_counter >= ((data->data_segment_) + data->data_segment_size_ - 1))
    {
      printf("out of memory now\n");
      data->data_segment_size_ *= 2;

      unsigned char* new_data_segment = (unsigned char*) realloc(data->data_segment_, data->data_segment_size_);
      if (new_data_segment == NULL)
      {
        printf("[ERR] out of memory\n");
      
        deleteBracketIndex(&data->bracket_index_, data->number_of_loops_);
        return OUT_OF_MEMORY;
      }
      //should simulate reCalloc()
      unsigned char* start = &(new_data_segment[data->data_segment_size_ / 2]);
      memset(start, 0, data->data_segment_size_ / 2);
      data->data_segment_ = new_data_segment;
      printf("new memory done\n");
    }

    switch ((data->code_segment_)[current_command_counter])
    {
    case '<'://check invalide access like negativ index
      if (program_counter > data->data_segment_)
      {
        --program_counter;
      }
      break;
    case '>':
      ++program_counter;
      break;
    case '+':
      ++(*program_counter);
      break;
    case '-':
      --(*program_counter);
      break;
    case '.':
      putchar(*program_counter);
      break;
    case ',':
      *program_counter = getchar();
      break;
    case '[':
      if (*program_counter == 0)
      {
        int index_of_current_command = getIndexOfBracket(bracket_index, number_of_loops, current_command_counter, OPEN);
        current_command_counter = data->bracket_index_[index_of_current_command][CLOSE];
      }
      break;
    case ']':
    {
      int index_of_current_command = getIndexOfBracket(bracket_index, number_of_loops, current_command_counter, CLOSE);
      current_command_counter = data->bracket_index_[(index_of_current_command)][OPEN];
      current_command_counter--;
      break;
    }

    default:
      break;
    }
    current_command_counter++;
  }
  data->last_stop_in_code = current_command_counter;
  data->last_program_counter_stop_ = program_counter;//can be usefull for memory and change!!! points to the current char of the data_segment
  data->program_counter_index_ = program_counter - data->data_segment_;//to get the differnce between the start and the current pos --> index 
  

  if(current_command_counter == data->code_length_)
    data->end_reached_ = TRUE;

  return SUCCESS;
}

int parseCode(unsigned char* code_segment, int*** brackets, int number_of_loops)
{
  if (number_of_loops == 0)
  {
    return SUCCESS;
  }
  int counter = 0;
  int latest_open_bracket = 0;
  int bracket_queue_counter = 0;
  int* bracket_queue = (int*) calloc(number_of_loops, sizeof (int));
  if (bracket_queue == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }

  while (code_segment[counter] != '\0')
  {
    if (number_of_loops == 0)
    {
      free(bracket_queue);
      return SUCCESS;
    }
    if (code_segment[counter] == '[')
    {
      (*brackets)[latest_open_bracket][0] = counter;
      *(bracket_queue + bracket_queue_counter++) = latest_open_bracket++;
    }
    else if (code_segment[counter] == ']')
    {
      (*brackets)[*(bracket_queue + --bracket_queue_counter)][1] = counter;
      number_of_loops--;
    }
    counter++;
  }
  free(bracket_queue);
  printf("[ERR] parsing of input failed\n");
  return PARSE_FILE_ERROR;
}

int getIndexOfBracket(int** bracket_index, int size_of_array, int current_command_counter, int open_or_close)
{
  int counter;

  for (counter = 0; counter < size_of_array; counter++)
  {
    if (current_command_counter == bracket_index[counter][open_or_close])
      return counter;
  }
  return NEUTRAL;
}

int createBracketIndex(int*** bracket_index, int number_of_loops)
{
  if (number_of_loops == 0)
  {
    return SUCCESS;
  }
  *bracket_index = (int**) calloc(number_of_loops, sizeof (int*));
  if (*bracket_index == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }
  int col_count;
  for (col_count = 0; col_count < number_of_loops; col_count++)
  {
    (*bracket_index)[col_count] = (int*) calloc(2, sizeof(int));
    if ((*bracket_index)[col_count] == NULL)
    {
      printf("[ERR] out of memory\n");
      col_count--;
      while (col_count >= 0)
      {
        free((*bracket_index)[col_count]);
        (*bracket_index)[col_count--] = NULL;
      }
      free(*bracket_index);
      *bracket_index = NULL;
      return OUT_OF_MEMORY;
    }
  }
  return SUCCESS;
}

int checkIfEqalWithBreakPoint(int current_command_counter, int** break_points)
{
  int counter = 0;
  int current_break_point = 0;

  if (*break_points == NULL)
  {
    return FALSE;
  }
    
  while((current_break_point = (*break_points)[counter]) != -2)
  {
    if (current_break_point == NEUTRAL)
    {
      counter++;
      continue;
    }
      
    
    if(current_command_counter == current_break_point)
    {
      (*break_points)[counter] = NEUTRAL;
      return TRUE;
    }
      
    counter++;
  }
  return FALSE;
}

int getCommandAndArgs(Input* input)
{
  int current_char;
  input->args_count_ = 0;
  input->args_ = NULL;
  input->command_ = NULL;
  int command_setted = FALSE;
  
  //max command is 6+nullbyte
  input->command_ = (char*)calloc(7, sizeof(char));
  if(input->command_ == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }
  
  int allocated_size_for_string = 7;
  
  
  int shell_count = 0;
  int last_char_was_space = FALSE;
  //starts the real read from stdin
  while((current_char = (char)getchar()) != '\n')
  {
    if(current_char == EOF)
      return END_OF_FILE;
    //check if there is space or more
    if(current_char == ' ' && last_char_was_space == FALSE)
    {
      command_setted = TRUE;
      if (input->args_ == NULL)
      {
        input->args_ = (char**)malloc(sizeof(char*));
        if (input->args_ == NULL)
        {
          printf("[ERR] out of memory\n");
          input->args_count_ = 0;
          freeInput(input);
          return OUT_OF_MEMORY;
        }
      }
      else
      {
        char** new_memory_for_strings = (char**)realloc(input->args_, sizeof(char*) * (input->args_count_ + 1));
        if (new_memory_for_strings == NULL)
        {
          printf("[ERR] out of memory\n");
          freeInput(input);
          return OUT_OF_MEMORY;
        }
        input->args_ = new_memory_for_strings;
      }
      
      allocated_size_for_string = 5;
      //most args are smaller than 4 + nullbyte
      input->args_[input->args_count_] = (char*) calloc(allocated_size_for_string, sizeof(char));
      if(input->args_[input->args_count_] == NULL)
      {
        printf("[ERR] out of memory\n");
        freeInput(input);
        return OUT_OF_MEMORY;
      }
      input->args_[input->args_count_][0] = '\0';
      input->args_count_++;
      
      last_char_was_space = TRUE;
      shell_count = 0;
      continue;
    }
    else if(current_char == ' ' && last_char_was_space == TRUE)
    {
        return SUCCESS;
    }
    //because the if's above would catch it
    last_char_was_space = FALSE;
    
    //check for right string to write.
    if(command_setted == FALSE)
    {
      if(saveChartoCommandorArg(input, current_char, &shell_count, &allocated_size_for_string, COMMAND) != SUCCESS)
        return OUT_OF_MEMORY;
    }
    else
    {
      if(saveChartoCommandorArg(input, current_char, &shell_count, &allocated_size_for_string, ARG) != SUCCESS)
        return OUT_OF_MEMORY;
    }
  }
  return SUCCESS;
}

int deleteBracketIndex(int*** bracket_index, int number_of_loops)
{
  int counter;
  for (counter = 0; counter < number_of_loops; counter++)
  {
    free((*bracket_index)[counter]);
    (*bracket_index)[counter] = NULL;
  }
  if((*bracket_index) != NULL)
  {
    free((*bracket_index));
    *bracket_index = NULL;
  }
    
  return SUCCESS;
}

int setBreakPoint(int** break_points, int point_pos)
{
  int counter = 0;
  if (*break_points != NULL)
  {
    for (counter = 1; (*break_points)[counter - 1] != -2; counter++)
      printf("breakpoint[%d] = %d\n", counter - 1, (*break_points)[counter - 1]);
  }
  
  int array_size = counter;

  if (array_size == 0)
  {
    *break_points = (int*)malloc(sizeof(int) * 2);
    if (*break_points == NULL)
    {
      printf("[ERR] out of memory\n");
      return OUT_OF_MEMORY;
    }
    if(point_pos >= -1)
      (*break_points)[0] = point_pos;

    (*break_points)[1] = -2;
    return SUCCESS;
  }
  else
  {
    for (counter = 0; counter < array_size; counter++)
    {
      if((*break_points)[counter] == point_pos)
        return SUCCESS;
    }
    
    for (counter = 0; counter < array_size; counter++)
    {
      if ((*break_points)[counter] == -1)
      {
        (*break_points)[counter] = point_pos;
        return SUCCESS;
      }
    }
    
    int* new_array = (int*)realloc(*break_points, sizeof(int) * (array_size + 1));
    if (new_array == NULL)
    {
      printf("[ERR] out of memory\n");
      return OUT_OF_MEMORY;
    }
    *break_points = new_array;
    (*break_points)[array_size - 1] = point_pos;
    (*break_points)[array_size] = -2;

    return SUCCESS;
  }
}

int checkSteps(int* steps)
{
  if(*steps == NEUTRAL)
  {
    return NEUTRAL;
  }
  else if(*steps == 0)
  {
    *steps = NEUTRAL;
    return TRUE;
  }
  else
  {
    --*steps;
    return FALSE;
  }
}

int showCode(unsigned char* code_segment, int current_command_counter, int* number_to_show)
{
  if(*number_to_show > 0)
  {
    int counter;
    for(counter = 0; counter < *number_to_show; counter++)
    {
      if (code_segment[current_command_counter + counter] == '\0')
      {
        printf("\n");
        return SUCCESS;
      }
        
      printf("%c", code_segment[current_command_counter + counter]);
    }
    printf("\n");
    return SUCCESS;
  }   
  else
    return SUCCESS;
  
}

int insertString(unsigned char** string, EvalData* eval_data)
{
  eval_data->insert_string_len_ = strlen((char*)eval_data->string_to_insert_) + 1;

  unsigned char* string_copy = (unsigned char*)calloc(1, sizeof(*string));
  if (string_copy == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }
  
  strcpy((char*)string_copy, (char*)*string);
  int counter;
  for (counter = 0; counter < eval_data->insert_string_len_; counter++)
  {
    (*string)[eval_data->insert_pos_in_string_ + counter] = eval_data->string_to_insert_[counter];
  }
  (*string)[eval_data->insert_pos_in_string_ + counter - 1] = '\0';
  counter = 0;
  while (string_copy[eval_data->insert_pos_in_string_ + counter] != '\0')
  {
    (*string)[eval_data->insert_pos_in_string_ + eval_data->insert_string_len_ + counter] = string_copy[eval_data->insert_pos_in_string_ + counter];
    counter++;
  }

  (*string)[eval_data->insert_pos_in_string_ + eval_data->insert_string_len_ + counter] = '\0';
  
  if(string_copy != NULL)
  {
    free(string_copy);
    string_copy = NULL;
  }
  return SUCCESS;
}

int cutOutString(unsigned char** string, EvalData* eval_data, int* last_stop_in_code)
{
  int counter = 0;
  while ((*string)[eval_data->insert_pos_in_string_ + eval_data->insert_string_len_ + counter] != '\0')
  {
    (*string)[eval_data->insert_pos_in_string_ + counter] = (*string)[eval_data->insert_pos_in_string_ + eval_data->insert_string_len_ + counter];
    counter++;
  }

  (*string)[eval_data->insert_pos_in_string_ + counter] = '\0';

  //because of nullbyte at the end of the insertstring
  *last_stop_in_code -= (eval_data->insert_string_len_ - 1);
  
  if(eval_data->string_to_insert_ != NULL)
  {
    printf("String is freed!\n");
    free(eval_data->string_to_insert_);
    eval_data->string_to_insert_ = NULL;
  }
  eval_data->insert_pos_in_string_ = NEUTRAL;
  eval_data->insert_string_len_ = NEUTRAL;
  
  return SUCCESS;
}

int initData(Environment* data)
{
  if (data->break_points_ != NULL)
  {
    free(data->break_points_);
    data->break_points_ = NULL;
  }
  setBreakPoint(&data->break_points_, -1);
  data->code_segment_size_ = 0;
  data->data_segment_size_ = 1024;
  data->last_stop_in_code = 0;
  data->last_program_counter_stop_ = NULL;
  data->data_loaded_ = FALSE;
  data->end_reached_ = FALSE;
  data->step_counter_ = NEUTRAL;
  data->program_counter_index_ = 0;
  if (data->bracket_index_ != NULL)
  {
    deleteBracketIndex(&data->bracket_index_, data->number_of_loops_);
    data->bracket_index_ = NULL;
  }
  data->number_of_loops_ = 0;

  if (data->code_segment_ != NULL)
  {
    free(data->code_segment_);
    data->code_segment_ = NULL;
  }
  
  if (data->data_segment_ != NULL)
  {
    free(data->data_segment_);
    data->data_segment_ = NULL;
  }

  data->data_segment_ = (unsigned char*)calloc(data->data_segment_size_, sizeof(unsigned char));
  if (data->data_segment_ == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }
  return SUCCESS;
}

int showMemory(Environment* data, int position, char* type)
{
  //Description for user MAYBE DEFINES?!
  char description[100];
  char value_as_type[100];

  unsigned char value = *(data->data_segment_ + position);

  if (strcmp(type, "hex") == TRUE)
  {
    strcpy(description, "Hex");
    sprintf(value_as_type, "%x", value);
  }
  else if (strcmp(type, "int") == TRUE)
  {
    strcpy(description, "Integer");
    sprintf(value_as_type, "%d", value);
  }
  else if (strcmp(type, "bin") == TRUE)
  {
    strcpy(description, "Binary");
    toBinary((int)value, 8, value_as_type);
  }
  else if (strcmp(type, "char") == TRUE)
  {
    strcpy(description, "Character");
    value_as_type[0] = value;
    value_as_type[1] = '\0';
  }
  else
  {
    //no valid type
    return SUCCESS;
  }

  printf("%s at %d: %s\n", description, position, value_as_type);
  return SUCCESS;
}

int changeMemory(Environment* data, int position, unsigned int value)
{
  data->data_segment_[position] = (unsigned char)value;
  return SUCCESS;
}

void toBinary(int value, int bitsCount, char* output)
{
  int counter;
  output[bitsCount] = '\0';
  for (counter = bitsCount - 1; counter >= 0; --counter, value >>= 1)
  {
    output[counter] = (value & 1) + '0';
  }
}

void freeInput(Input* input)
{
  //free the input
  if (input->command_ != NULL)
  {
    free(input->command_);
    input->command_ = NULL;
  }

  int counter;
  for (counter = 0; counter < input->args_count_; counter++)
  {
    if (input->args_[counter] != NULL)
    {
      free(input->args_[counter]);
      input->args_[counter] = NULL;
    }

  }
  if (input->args_ != NULL)
  {
    free(input->args_);
    input->args_ = NULL;
  }
}

int loadCommand(Input* input, Environment* data)
{
  int return_value = SUCCESS;
  if(input->args_count_ == 0 || input->args_[0][0] == '\0')
  {
    printf("[ERR] wrong parameter count\n");
    if (input->command_ != NULL)
    {
      freeInput(input);
    }
    return return_value;
  }
  if (data->data_loaded_ == TRUE)
  {
    printf("loaded\n");
    initData(data);
  }
  else
  {
    printf("not loaded\n");
    data->break_points_ = NULL;
    data->bracket_index_ = NULL;
    data->code_segment_ = NULL;
    data->data_segment_ = NULL;
    data->code_segment_size_ = 0;
    data->data_segment_size_ = 1024;
    data->last_stop_in_code = 0;
    data->last_program_counter_stop_ = NULL;
    data->data_loaded_ = FALSE;
    data->end_reached_ = FALSE;
    data->step_counter_ = NEUTRAL;
    data->program_counter_index_ = 0;
    setBreakPoint(&data->break_points_, -1);
    data->data_segment_ = (unsigned char*)calloc(data->data_segment_size_, sizeof(unsigned char));
    if (data->data_segment_ == NULL)
    {
      printf("[ERR] out of memory\n");
      freeInput(input);
      return_value = OUT_OF_MEMORY;
      return return_value;
    }
  }
  if((return_value = readCodeFromFile(data, input->args_[0])) != SUCCESS)
  {
    if (return_value == OUT_OF_MEMORY)
    {
      freeInput(input);
      return return_value;
    }
    else if(return_value == READING_FILE_FAIL)
    {
      freeInput(input);
      return return_value;
    }
  }

  if (checkCodeCorrectness(data->code_segment_, &(data->number_of_loops_)) != SUCCESS)
  {
    printf("[ERR] parsing of input failed\n");
    freeInput(input);
    return return_value;
  } 
  if (createBracketIndex(&data->bracket_index_, data->number_of_loops_) != SUCCESS)
  {
    freeInput(input);
    return_value = OUT_OF_MEMORY;
    return return_value;
  }
  if ((return_value = parseCode(data->code_segment_, &data->bracket_index_, data->number_of_loops_)) != SUCCESS)
  {
    freeInput(input);
    if (return_value == OUT_OF_MEMORY)
    {
      return return_value;
    }
    else
    {
      return_value = SUCCESS;
      return return_value;
    }
  }
  data->data_loaded_ = TRUE;
  return return_value;
}

void runCommand(Environment* data)
{
  if (data->data_loaded_ == TRUE && data->end_reached_ == FALSE)
  {
    runCode(data, data->bracket_index_, data->number_of_loops_);
  }   
  else
    printf("[ERR] no program loaded\n");
}

int breakCommand(Input* input, Environment* data)
{
  int return_value = SUCCESS;
  if (input->args_count_ == 0)
  {
    printf("[ERR] wrong parameter count\n");
    freeInput(input);
    return return_value;
  }
  if (data->data_loaded_ == TRUE && data->end_reached_ == FALSE)
  {
    int break_point = atoi(input->args_[0]);
    if(break_point < 0)
      break_point *= -1;

    if ((return_value = setBreakPoint(&data->break_points_, break_point)) != SUCCESS)
    {
      freeInput(input);
      return return_value;
    }  
  }
  else
  {
    printf("[ERR] no program loaded\n");
  }
  return return_value;
}

void stepCommand(Input* input, Environment* data)
{
   if (data->data_loaded_ == TRUE && data->end_reached_ == FALSE)
  {
    if (input->args_count_ == 0)
    {
      data->step_counter_ = 1;
    }
    else
    {
      data->step_counter_ = atoi(input->args_[0]);
      if(data->step_counter_ < 0)
          data->step_counter_ *= -1;
    }
    runCode(data, data->bracket_index_, data->number_of_loops_);
  }
  else
    printf("[ERR] no program loaded\n");
}

void showCommand(Input* input, Environment* data)
{
  if (data->data_loaded_ == TRUE && data->end_reached_ == FALSE)
  {
    int number_to_show = 0;
    if (input->args_count_ == 0)
    {
      number_to_show = 10;
    }
    else
    {
      number_to_show = atoi(input->args_[0]);
      if(number_to_show < 0)
        number_to_show *= -1;
    }

    showCode(data->code_segment_, data->last_stop_in_code, &number_to_show);
  }
  else
    printf("[ERR] no program loaded\n");
}

int evalCommand(Input* input, Environment* data, EvalData* eval_data)
{
  int return_value = SUCCESS;
  if (input->args_count_ == 0)
  {
    printf("[ERR] wrong parameter count\n");
    freeInput(input);
    return return_value;
  }
  if (data->data_loaded_ != TRUE)
  {
    printf("not loaded\n");
    data->break_points_ = NULL;
    data->bracket_index_ = NULL;
    data->code_segment_ = NULL;
    data->data_segment_ = NULL;
    data->code_segment_size_ = 0;
    data->data_segment_size_ = 1024;
    data->last_stop_in_code = 0;
    data->last_program_counter_stop_ = NULL;
    data->data_loaded_ = TRUE;
    data->end_reached_ = FALSE;
    data->step_counter_ = NEUTRAL;
    data->program_counter_index_ = 0;
    setBreakPoint(&data->break_points_, -1);
    data->data_segment_ = (unsigned char*)calloc(data->data_segment_size_, sizeof(unsigned char));
    if (data->data_segment_ == NULL)
    {
      printf("[ERR] out of memory\n");
      freeInput(input);
      return_value = OUT_OF_MEMORY;
      return return_value;
    }
    data->code_segment_ = (unsigned char*)calloc(85, sizeof(unsigned char));
    if (data->code_segment_ == NULL)
    {
      printf("[ERR] out of memory\n");
      freeInput(input);
      return_value = OUT_OF_MEMORY;
      return return_value;
    }
    data->code_segment_size_ = 85; 
  }
  int eval_len = strlen(input->args_[0]);
  eval_data->string_to_insert_ = (unsigned char*) malloc((eval_len + 1) * sizeof(unsigned char));
  strcpy((char*)eval_data->string_to_insert_, input->args_[0]);
  if (checkCodeCorrectness(eval_data->string_to_insert_, &(eval_data->number_of_loops_)) != SUCCESS)
  {
    printf("[ERR] parsing of input failed\n");
    freeInput(input);
    if(eval_data->string_to_insert_ != NULL)
    {
      printf("String is freed!\n");
      free(eval_data->string_to_insert_);
      eval_data->string_to_insert_ = NULL;
    }
    return return_value;
  }

  if (createBracketIndex(&eval_data->bracket_index_, eval_data->number_of_loops_) != SUCCESS)
  {
    freeInput(input);
    if(eval_data->string_to_insert_ != NULL)
    {
      printf("String is freed!\n");
      free(eval_data->string_to_insert_);
      eval_data->string_to_insert_ = NULL;
    }
    return_value = OUT_OF_MEMORY;
    return return_value;
  }

  if ((return_value = parseCode(eval_data->string_to_insert_, &eval_data->bracket_index_, eval_data->number_of_loops_)) != SUCCESS)
  {
    freeInput(input);
    if(eval_data->string_to_insert_ != NULL)
    {
      printf("String is freed!\n");
      free(eval_data->string_to_insert_);
      eval_data->string_to_insert_ = NULL;
    }
    if (return_value == OUT_OF_MEMORY)
    {
      return return_value;
    }
    else
    {
      return_value = SUCCESS;
      return return_value;
    }
  }

  if ((eval_data->insert_string_len_ = (strlen((char*)eval_data->string_to_insert_) + 1)) > 80)
  {
    eval_data->insert_string_len_ = 81;
    eval_data->string_to_insert_[80] = '\0';
  }
  eval_data->insert_pos_in_string_ = data->last_stop_in_code;
  if (insertString(&(data->code_segment_), eval_data) != SUCCESS)
  {
    return_value = OUT_OF_MEMORY;
    freeInput(input);
    if(eval_data->string_to_insert_ != NULL)
    {
      printf("String is freed!\n");
      free(eval_data->string_to_insert_);
      eval_data->string_to_insert_ = NULL;
    }
    return return_value;
  }
  runCode(data, eval_data->bracket_index_, eval_data->number_of_loops_);
  cutOutString(&data->code_segment_, eval_data, &data->last_stop_in_code);
  return return_value;
}

int memoryCommand(Input* input, Environment* data)
{
  if (data->data_loaded_ == TRUE)
  {
    int position = data->program_counter_index_;
    char type[5] = "hex\0";

    if (input->args_count_ >= 2)
    {
      //load type
      sscanf(input->args_[1], "%4s", type);
    }
    if (input->args_count_ >= 1)
    {
      //load number
      if ((position = atoi(input->args_[0])) == 0)
      {
        printf("starts not with an int\n");
        position = 0;
      }
    }
    if (position < 0 || position >= data->data_segment_size_)
    {
      freeInput(input);
      return FAILED;
    }
    showMemory(data, position, type);
  }
  else
    printf("[ERR] no program loaded\n");
  
  return SUCCESS;
}

int changeCommand(Input* input, Environment* data)
{
  int return_value = SUCCESS;
  if (data->data_loaded_ == TRUE)
  {
    int position = data->program_counter_index_;
    unsigned int hex_byte = 0;
        
    if (input->args_count_ >= 2)
    {
      //load hex_byte
      sscanf(input->args_[1], "%2x", (unsigned int*)&hex_byte);
    }
    if (input->args_count_ >= 1)
    {
      //load number
      sscanf(input->args_[0], "%d", &position);
    }
    if (position < 0 || position >= (data->data_segment_size_ - 1))
    {
      freeInput(input);
      return FAILED;
    }
    return_value = changeMemory(data, position, hex_byte);
  }
  else
    printf("[ERR] no program loaded\n");
  
  return return_value;
}

void handleDebugMode(Environment* data, EvalData* eval_data, int* return_value)
{
  Input input;
  input.args_ = NULL;
  input.command_ = NULL;
  input.args_count_ = 0;
  /*-----START DEBUG MODE-----*/
  int debug_mode_on = TRUE;
  while(debug_mode_on == TRUE)
  {
    printf("esp> ");


    if((*return_value = getCommandAndArgs(&input)) == OUT_OF_MEMORY)
    {
      freeInput(&input);
      break;
    }

    if (*return_value == END_OF_FILE)
    {
      freeInput(&input);
      break;
    }


    if(strcmp(input.command_, "load") == TRUE)
    {
      *return_value = loadCommand(&input, data);
      if(*return_value == OUT_OF_MEMORY)
      {
        break;
      }
    }
    else if(strcmp(input.command_, "run") == TRUE)
    { 
      runCommand(data);
    }
    else if (strcmp(input.command_, "quit") == TRUE)
    {
      printf("Bye.\n");
      debug_mode_on = FALSE;
    }
    else if (strcmp(input.command_, "break") == TRUE)
    {
      *return_value = breakCommand(&input, data);
      if(*return_value == OUT_OF_MEMORY)
      {
        break;
      }
    }
    else if (strcmp(input.command_, "step") == TRUE)
    {
      stepCommand(&input, data);
    }
    else if (strcmp(input.command_, "show") == TRUE)
    {
      showCommand(&input, data);
    }
    else if (strcmp(input.command_, "eval") == TRUE)
    {
      *return_value = evalCommand(&input, data, eval_data);
      if(*return_value == OUT_OF_MEMORY)
      {
        break;
      }
    }
    else if (strcmp(input.command_, "memory") == TRUE)
    {
      if(memoryCommand(&input, data) == FAILED)
      {
        continue;
      }
    }
    else if (strcmp(input.command_, "change") == TRUE)
    {
      if(changeCommand(&input, data) == FAILED)
      {
        continue;
      }
    }

    freeInput(&input);
  }
  
}

void handleNormalMode(Environment* data, int* return_value, char* argv[])
{
  if (!strcmp(argv[1], "-e"))
  {
    data->break_points_ = NULL;
    data->bracket_index_ = NULL;
    data->code_segment_ = NULL;
    data->data_segment_ = NULL;
    data->code_segment_size_ = 0;
    data->data_segment_size_ = 1024;
    data->last_stop_in_code = 0;
    data->last_program_counter_stop_ = NULL;
    data->data_loaded_ = FALSE;
    data->end_reached_ = FALSE;
    data->step_counter_ = NEUTRAL;
    data->program_counter_index_ = 0;
    setBreakPoint(&data->break_points_, -1);
    data->data_segment_ = (unsigned char*)calloc(data->data_segment_size_, sizeof(unsigned char));
    if (data->data_segment_ == NULL)
    {
      printf("[ERR] out of memory\n");
      *return_value = OUT_OF_MEMORY;
      return;
    }

    if((*return_value = readCodeFromFile(data, argv[2])) != SUCCESS)
    {
      if (*return_value == OUT_OF_MEMORY)
      {
        return;
      }
      else if(*return_value == READING_FILE_FAIL)
      {
        return;
      }
    }

    if (checkCodeCorrectness(data->code_segment_, &(data->number_of_loops_)) != SUCCESS)
    {
      printf("[ERR] parsing of input failed\n");
      *return_value = PARSE_FILE_ERROR;
      return;
    } 
    if (createBracketIndex(&data->bracket_index_, data->number_of_loops_) != SUCCESS)
    {
      *return_value = OUT_OF_MEMORY;
      return;
    }
    if ((*return_value = parseCode(data->code_segment_, &data->bracket_index_, data->number_of_loops_)) != SUCCESS)
    {
      if (*return_value == OUT_OF_MEMORY)
      {
        return;
      }
      else
      {
        *return_value = PARSE_FILE_ERROR;
        return;
      }
    }
    
    runCode(data, data->bracket_index_, data->number_of_loops_);
  }
  else
  {
    printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
    *return_value = FALSE_ARGUMENTS;
  }
}

int saveChartoCommandorArg(Input* input, char current_char, int* shell_count, int* allocated_size_for_string, int command_or_arg)
{
  if(((*shell_count) + 1) < *allocated_size_for_string)
  {
    if(command_or_arg == ARG)
    {
      input->args_[input->args_count_ - 1][*shell_count] = current_char;
      ++*shell_count;
      input->args_[input->args_count_ - 1][*shell_count] = '\0';
    }
    else if(command_or_arg == COMMAND)
    {
      input->command_[*shell_count] = current_char;
      ++*shell_count;
      input->command_[*shell_count] = '\0';
    } 
  }
  else
  {
    //allocated_size_for_string double memory space
    *allocated_size_for_string *= 2;
    if(command_or_arg == ARG)
    {
      char* new_memory_for_input = (char*)realloc(input->args_[input->args_count_ - 1], *allocated_size_for_string);
      if(new_memory_for_input == NULL)
      {
        printf("[ERR] out of memory\n");
        input->args_count_--;
        freeInput(input);
        return OUT_OF_MEMORY;
      }
      input->args_[input->args_count_ - 1] = new_memory_for_input;
      input->args_[input->args_count_ - 1][*shell_count] = current_char;
      ++*shell_count;
      input->args_[input->args_count_ - 1][*shell_count] = '\0';
    }
    else if(command_or_arg == COMMAND)
    {
      char* new_memory_for_input = (char*)realloc(input->command_, *allocated_size_for_string);
      if(new_memory_for_input == NULL)
      {
        printf("[ERR] out of memory\n");
        input->args_count_--;
        freeInput(input);
        return OUT_OF_MEMORY;
      }
      input->command_ = new_memory_for_input;
      input->command_[*shell_count] = current_char;
      ++*shell_count;
      input->command_[*shell_count] = '\0';
    }
  }
  return SUCCESS;
}
