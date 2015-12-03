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
// Latest Changes: 22.11.2015 (by Stefan Papst)
//------------------------------------------------------------------------------
//
/*this is a special flag in visual studio for secure function errors/ Can be commented if there is a error in gcc*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//for error message in fopen()
//#include <errno.h>



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
  int eval_stop_point_;
} Environment;

typedef struct {
  char* command_;
  char** args_;
  int args_count_;
} Input;

typedef struct {
  int insert_pos_in_string_;
  int insert_string_len_;
  unsigned char* string_to_insert_;
} EvalData;


/*------------HEADER NEED TO BE DONE-----------*/
int readCodeFromFile(Environment* data, char* name);
int checkCodeCorrectness(unsigned char* code_segment, int* number_of_loops);
int checkCommandOrComment(unsigned char current_char);
int runCode(Environment* data);
int parseCode(unsigned char* code_segment, int*** brackets, int number_of_loops);
int getIndexOfBracket(int** bracket_index, int size_of_array, int current_command_counter, int open_or_close);
int createBracketIndex(int*** bracket_index, int number_of_loops);
int checkIfEqalWithBreakPoint(int current_command_counter, int** break_points, int* eval_stop_point);
int getCommandAndArgs(Input* input);
int deleteBracketIndex(int*** bracket_index, int number_of_loops);
int setBreakPoint(int** break_points, int point_pos);
int checkSteps(int* steps);
int showCode(unsigned char* code_segment, int current_command_counter, int* number_to_show);
int insertString(unsigned char** string, EvalData* eval_data);
int cutOutString(unsigned char** string, EvalData* eval_data, int* last_stop_in_code);
int initData(Environment* data);

//-----------------------------------------------------------------------------
//
/// The main program
/// SOME TEXT
///
/// @param argc The counter how many arguments are written on the commandline
/// @param argv The value of the command line arguments
///
/// @return 0
/// @return 1
/// @return 2 
/// @return 3
///

int main(int argc, char* argv[])
{
  Environment data;
  EvalData eval_data;
  int return_value = SUCCESS;

  if (argc == 3)
  {
    if (!strcmp(argv[1], "-e"))
    {
      data.break_points_ == NULL;
      data.bracket_index_ = NULL;
      data.code_segment_ = NULL;
      data.data_segment_ = NULL;
      return_value = readCodeFromFile(&data, argv[2]);
      runCode(&data);

    }
    else
    {
      printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
      return FALSE_ARGUMENTS;
    }
  }
  else if (argc == 1)
  {

    /*-----START DEBUG MODE-----*/
    int debug_mode_on = TRUE;
    while(debug_mode_on == TRUE)
    {
      printf("esp> ");
      Input input;
      input.args_ = NULL;

      //input need to be freed !!!!!!!!!
      if((return_value = getCommandAndArgs(&input)) == OUT_OF_MEMORY)
        return return_value;

      if(return_value == END_OF_FILE)
        return return_value;

      printf("command: %s\n", input.command_);
      
      
    
      if(strcmp(input.command_, "load") == TRUE)
      {
        if(input.args_count_ == 0 || input.args_[0][0] == '\0')
        {
          printf("[ERR] wrong parameter count\n");
          free(input.command_);
          continue;
        }
        if (data.data_loaded_ == TRUE)
        {
          printf("loaded\n");
          initData(&data);
        }
        else
        {
          printf("not loaded\n");
          data.break_points_ = NULL;
          data.bracket_index_ = NULL;
          data.code_segment_ = NULL;
          data.data_segment_ = NULL;
          data.code_segment_size_ = 0;
          data.data_segment_size_ = 1024;
          data.last_stop_in_code = 0;
          data.last_program_counter_stop_ = NULL;
          data.data_loaded_ = FALSE;
          data.end_reached_ = FALSE;
          data.step_counter_ = NEUTRAL;
          data.eval_stop_point_ = NEUTRAL;
          data.program_counter_index_ = 0;
          setBreakPoint(&data.break_points_, 0);
          data.data_segment_ = (unsigned char*)calloc(data.data_segment_size_, sizeof(unsigned char));
          if (data.data_segment_ == NULL)
          {
            printf("[ERR] out of memory\n");
            return OUT_OF_MEMORY;
          }
        }
        if((return_value = readCodeFromFile(&data, input.args_[0])) != SUCCESS)
        {
          if(return_value == OUT_OF_MEMORY)
            return return_value;
          else if(return_value == READING_FILE_FAIL)
          {
            //need to be freed here(INPUT!!))
            continue;
          }
        }

        if (checkCodeCorrectness(data.code_segment_, &(data.number_of_loops_)) != SUCCESS)
        {
          printf("[ERR] parsing of input failed\n");
          //here is a leak -> the input is not be freed after this continue!!!!!
          continue;
        } 
        data.bracket_index_ = NULL;
        if (createBracketIndex(&data.bracket_index_, data.number_of_loops_) != SUCCESS)
          return_value = OUT_OF_MEMORY;

        parseCode(data.code_segment_, &data.bracket_index_, data.number_of_loops_);
        data.data_loaded_ = TRUE;
      }
      else if(strcmp(input.command_, "run") == TRUE)
      { 
        if(data.data_loaded_ == TRUE && data.end_reached_ == FALSE)
          return_value = runCode(&data);
        else
          printf("[ERR] no program loaded\n");
        if (return_value == OUT_OF_MEMORY || return_value == END_OF_FILE)
        {
          debug_mode_on = FALSE;
        }
      }
      else if (strcmp(input.command_, "quit") == TRUE)
      {
        printf("Bye.\n");
        debug_mode_on = FALSE;
      }
      else if (strcmp(input.command_, "break") == TRUE)
      {
        if (input.args_count_ == 0)
        {
          printf("[ERR] wrong parameter count\n");
          free(input.command_);
          continue;
        }
        if (data.data_loaded_ == TRUE && data.end_reached_ == FALSE)
        {
          int break_point = atoi(input.args_[0]);
          if(break_point < 0)
            break_point *= -1;

          if ((return_value = setBreakPoint(&data.break_points_, break_point)) != SUCCESS)
            return return_value;
        }
        else
        {
          printf("[ERR] no program loaded\n");
        }
        //is a leak above !!!
      }
      else if (strcmp(input.command_, "step") == TRUE)
      {
        if (data.data_loaded_ == TRUE && data.end_reached_ == FALSE)
        {
          if (input.args_count_ == 0)
          {
            data.step_counter_ = 1;
          }
          else
          {
            data.step_counter_ = atoi(input.args_[0]);
            if(data.step_counter_ < 0)
                data.step_counter_ *= -1;
          }
          
          return_value = runCode(&data);
        }
        else
          printf("[ERR] no program loaded\n");
        
        if (return_value == OUT_OF_MEMORY || return_value == END_OF_FILE)
        {
          debug_mode_on = FALSE;
        }
        //is a leak above !!!
      }
      else if (strcmp(input.command_, "show") == TRUE)
      {
       if (data.data_loaded_ == TRUE && data.end_reached_ == FALSE)
        {
         int number_to_show = 0;
          if (input.args_count_ == 0)
          {
            number_to_show = 10;
          }
          else
          {
            number_to_show = atoi(input.args_[0]);
            if(number_to_show < 0)
              number_to_show *= -1;
          }
          //reference maybe not needed
          return_value = showCode(data.code_segment_, data.last_stop_in_code, &number_to_show);
        }
        else
          printf("[ERR] no program loaded\n");
      }
      /*
      else if (strcmp(input.command_, "eval") == TRUE)
      {
        if (data.data_loaded_ == FALSE)
        {
          initData(&data);
          data.code_segment_size_ = (int)(1024 * 0.8) + 1;
          data.last_program_counter_stop_ = data.data_segment_ + data.code_segment_size_;
          data.data_loaded_ = TRUE;
          data.end_reached_ = FALSE;
        }
        
        if (input.args_count_ == 0)
        {
          //maybe a leak here!!!
          printf("[ERR] wrong parameter count\n");
          continue;
        }
          //printf("Command: %s\n", input.command_);

        eval_data.string_to_insert_ = input.args_[0];
        if ((eval_data.insert_string_len_ = strlen(eval_data.string_to_insert_)) > 80)
        {
          eval_data.insert_string_len_ = 81;
          eval_data.string_to_insert_[80] = '\0';
        }
        eval_data.insert_pos_in_string_ = data.last_stop_in_code;
        insertString(&(data.data_segment_), &eval_data);
        data.eval_stop_point_ = data.last_stop_in_code + eval_data.insert_string_len_;
        
        return_value = runCode(&data);
        cutOutString(&data.data_segment_, &eval_data, &data.last_stop_in_code);
     
      }
      */
        
      //free the input
      //free(input.command_);
      input.command_ = NULL;
      int counter;
      for(counter = 0; counter < input.args_count_; counter++)
      {
        free(input.args_[counter]);
      }
    }
  }
  else
  {
    printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
    return FALSE_ARGUMENTS;
  }
  //Can not quit if no memory was allocated for brackets!!!!!!!!!!!
  if (data.data_loaded_ == TRUE)
  {
   // initData(&data);
  }

  return return_value;
}

int readCodeFromFile(Environment* data, char* name)
{
  //open file
  FILE* file;
  file = fopen(name, "r");
  if (file == NULL)
  {
    printf("[ERR] reading the file failed\n");
    return READING_FILE_FAIL;
  }
  
  data->code_segment_ = (char*)calloc(500, sizeof(char));
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
    {//because of \0 and eval string
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

/*
//--------------------------------------------------------------------------------------
// TODO: implement run function!
//-------------------------------------------------------------------------------------
*/
int runCode(Environment* data)
{
  unsigned char* program_counter = data->data_segment_ + data->program_counter_index_;
  int current_command_counter = data->last_stop_in_code;
    
  while ((data->code_segment_)[current_command_counter] != '\0' && (checkIfEqalWithBreakPoint(current_command_counter, &data->break_points_, &data->eval_stop_point_) == FALSE))
  { 
    if (checkSteps(&data->step_counter_) == TRUE)
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
        int index_of_current_command = getIndexOfBracket(data->bracket_index_, data->number_of_loops_, current_command_counter, OPEN);
        current_command_counter = data->bracket_index_[index_of_current_command][CLOSE];
      }
      break;
    case ']':
    {
      int index_of_current_command = getIndexOfBracket(data->bracket_index_, data->number_of_loops_, current_command_counter, CLOSE);
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

int checkIfEqalWithBreakPoint(int current_command_counter, int** break_points, int* eval_stop_point)
{
  int counter = 0;
  int current_break_point = 0;
  if(*eval_stop_point != NEUTRAL)
  {
    if(current_command_counter == *eval_stop_point)
    {
      *eval_stop_point = NEUTRAL;
      return TRUE;
    }
  }
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
  
      char** new_memory_for_strings = (char**)realloc(input->args_, input->args_count_ + 1);
      if (new_memory_for_strings == NULL)
      {
        printf("[ERR] out of memory\n");
        return OUT_OF_MEMORY;
      }
      input->args_ = new_memory_for_strings;
      allocated_size_for_string = 5;
      //most args are smaller than 4 + nullbyte
      input->args_[input->args_count_] = (char*) calloc(allocated_size_for_string, sizeof(char));
      if(input->args_[input->args_count_] == NULL)
      {
        printf("[ERR] out of memory\n");
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
      if((shell_count + 1) < allocated_size_for_string)
      {
        input->command_[shell_count] = current_char;
        ++shell_count;
        input->command_[shell_count] = '\0';
      }
      else
      {
        //allocated_size_for_string = last size of memoryblock
        allocated_size_for_string *= 2;
        char* new_memory_for_input = (char*)realloc(input->command_, allocated_size_for_string);
        if(new_memory_for_input == NULL)
        {
          printf("[ERR] out of memory\n");
          return OUT_OF_MEMORY;
        }
        input->command_ = new_memory_for_input;
        
        input->command_[shell_count] = current_char;
        ++shell_count;
        input->command_[shell_count] = '\0';
      }
    }
    else
    {
       if((shell_count + 1) < allocated_size_for_string)
      {
        input->args_[input->args_count_ - 1][shell_count] = current_char;
        ++shell_count;
        input->args_[input->args_count_ - 1][shell_count] = '\0';
      }
      else
      {
        //allocated_size_for_string double memory space
        allocated_size_for_string *= 2;
        char* new_memory_for_input = (char*)realloc(input->args_[input->args_count_ - 1], allocated_size_for_string);
        if(new_memory_for_input == NULL)
        {
          printf("[ERR] out of memory\n");
          return OUT_OF_MEMORY;
        }
        input->args_[input->args_count_ - 1] = new_memory_for_input;
        
        input->args_[input->args_count_ - 1][shell_count] = current_char;
        ++shell_count;
        input->args_[input->args_count_ - 1][shell_count] = '\0';
      }
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
    if(point_pos != 0)
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
    for( counter = 0; counter < *number_to_show; counter++)
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
  else if(*number_to_show == 0)
    return SUCCESS;
  else
    return FAILED;
}

int insertString(unsigned char** string, EvalData* eval_data)
{
  eval_data->insert_string_len_ = strlen(eval_data->string_to_insert_);

  unsigned char* string_copy = (unsigned char*)calloc(1, sizeof(*string));
  if (string_copy == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }
  
  strcpy(string_copy, *string);
  int counter;
  for (counter = 0; counter < eval_data->insert_string_len_; counter++)
  {
    (*string)[eval_data->insert_pos_in_string_ + counter] = eval_data->string_to_insert_[counter];
  }

  counter = 0;
  while (string_copy[eval_data->insert_pos_in_string_ + counter] != '\0')
  {
    (*string)[eval_data->insert_pos_in_string_ + eval_data->insert_string_len_ + counter] = string_copy[eval_data->insert_pos_in_string_ + counter];
    counter++;
  }

  (*string)[eval_data->insert_pos_in_string_ + eval_data->insert_string_len_ + counter] = '\0';

  return SUCCESS;
}

int cutOutString(unsigned char** string, EvalData* eval_data, int* last_stop_in_code)
{
  int counter = eval_data->insert_pos_in_string_;
  int old_string_len = strlen(*string) - eval_data->insert_string_len_;
  while (counter < old_string_len)
  {
    (*string)[counter] = (*string)[counter + eval_data->insert_string_len_];
    counter++;
  }
  string[counter] = '\0';
  
  *last_stop_in_code -= eval_data->insert_string_len_;
  
  //because the input args will be freed outside of the function
  eval_data->string_to_insert_ = NULL;
  eval_data->insert_pos_in_string_ = 0;
  eval_data->insert_string_len_ = 0;
  
  
  return SUCCESS;
}

int initData(Environment* data)
{
  if (data->break_points_ != NULL)
  {
    free(data->break_points_);
    data->break_points_ = NULL;
  }
  setBreakPoint(&data->break_points_, 0);
  data->code_segment_size_ = 0;
  data->data_segment_size_ = 1024;
  data->last_stop_in_code = 0;
  data->last_program_counter_stop_ = NULL;
  data->data_loaded_ = FALSE;
  data->end_reached_ = FALSE;
  data->step_counter_ = NEUTRAL;
  data->eval_stop_point_ = NEUTRAL;
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

