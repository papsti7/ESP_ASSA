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




#define SUCCESS 0
#define FALSE_ARGUMENTS 1
#define OUT_OF_MEMORY 2
#define PARSE_FILE_ERROR 3
#define READING_FILE_FAIL 4

#define TRUE 0
#define FALSE 1
#define NEUTRAL -1

#define OPEN 0
#define CLOSE 1

typedef struct
{
  unsigned char* data_segment_;
  int* break_points_;
  int code_length_;
  int data_segment_size_;
  int number_of_loops_;
  int last_stop_in_code;
} Data;


/*------------HEADER NEED TO BE DONE-----------*/
int readCodeFromFile(Data* data, char* name);
int checkCodeCorrectness(unsigned char* data_segment, int* number_of_loops);
int checkCommandOrComment(unsigned char current_char);
int runCode(Data* data);
int parseCode(unsigned char* data_segment, int*** brackets, int number_of_loops);
int getIndexOfBracket(int** bracket_index, int size_of_array, int current_command_counter, int open_or_close);
int createBracketIndex(int*** bracket_index, int number_of_loops);
int checkIfEqalWithBreakPoint(int current_command_counter, int** break_points);


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
  Data data;
  data.data_segment_ = NULL;
  data.break_points_ = NULL;
  data.break_points_ = (int*) malloc(sizeof (int) * 3);
  data.break_points_[0] = 2;
  data.break_points_[1] = 40;
  data.break_points_[2] = -1;
  data.code_length_ = 0;
  data.data_segment_size_ = 1024;
  data.number_of_loops_ = 0;
  data.last_stop_in_code = 0;
  int return_value = SUCCESS;

  if (argc == 3)
  {
    if (!strcmp(argv[1], "-e"))
    {
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
  }
  else
  {
    printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
    return FALSE_ARGUMENTS;
  }
  free(data.data_segment_);
  free(data.break_points_);
  data.data_segment_ = NULL;
  data.break_points_ = NULL;

  

  //getchar();

  
  return return_value;
}

int readCodeFromFile(Data* data, char* name)
{
  data->data_segment_ = (unsigned char*) calloc(1024, sizeof (unsigned char));
  if (data->data_segment_ == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }
  //set end of string
  data->data_segment_[(int) (1024 * 0.8) + 1] = '\0';

  //open file
  FILE* file;
  //errno_t error_type;
  //error_type = fopen_s(&file, name, "r");
  file = fopen(name, "r");
  if (file == NULL)
  {
    printf("[ERR] reading the file failed\n");
    //printf("errno type: %s\n", strerror(error_type));
    return READING_FILE_FAIL;
  }

  int current_char;
  int counter = 0;
  while ((current_char = getc(file)) != EOF)
  {
    if (checkCommandOrComment(current_char) == TRUE)
    {
      if (counter < (int) (data->data_segment_size_ * 0.8))
      {
        (data->data_segment_)[counter] = current_char;
        (data->data_segment_)[counter + 1] = '\0';

      }
      else
      {
        unsigned char* new_data_segment = NULL;
        data->data_segment_size_ *= 2;

        new_data_segment = (unsigned char*) realloc(data->data_segment_, data->data_segment_size_);
        if (new_data_segment == NULL)
        {
          printf("[ERR] out of memory\n");
          return OUT_OF_MEMORY;
        }
        unsigned char* start = &(new_data_segment[data->data_segment_size_ / 2]);
        memset(start, 0, data->data_segment_size_ / 2);

        data->data_segment_ = new_data_segment;
        (data->data_segment_)[counter] = current_char;
        (data->data_segment_)[counter + 1] = '\0';
      }
      data->code_length_ = (++counter) + 1;
    }
  }
  //puts(*data_segment);

  fclose(file);

  return SUCCESS;
}

int checkCodeCorrectness(unsigned char* data_segment, int* number_of_loops)
{

  //1 because in data_segment[0] is a value of how long the code part is. 
  int counter = 1;
  int open_brackets_count = 0;
  int close_brackets_count = 0;
  while (data_segment[counter] != '\0')
  {
    if (data_segment[counter] == '[')
      open_brackets_count++;
    else if (data_segment[counter] == ']')
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

int runCode(Data* data)
{

  if (checkCodeCorrectness(data->data_segment_, &(data->number_of_loops_)) != SUCCESS)
  {
    printf("[ERR] parsing of input failed\n");
    return PARSE_FILE_ERROR;
  }

  int** bracket_index = NULL;
  if (createBracketIndex(&bracket_index, data->number_of_loops_) != SUCCESS)
    return OUT_OF_MEMORY;


  parseCode(data->data_segment_, &bracket_index, data->number_of_loops_);


  /*
  //--------------------------------------------------------------------------------------
  // TODO: implement run function! -THERE IS A SEQ FAULT IN IT OR ANYTHING ELSE!!!
  //-------------------------------------------------------------------------------------
   */


  //data - part of segment
  unsigned char* start_address = (data->data_segment_) + data->code_length_;
  unsigned char* program_counter = start_address;


  int current_command_counter = data->last_stop_in_code;
  

  while ((data->data_segment_)[current_command_counter] != '\0' || checkIfEqalWithBreakPoint(current_command_counter, &data->break_points_))
  {
    if (program_counter >= ((data->data_segment_) + data->data_segment_size_ - 1))
    {
      printf("out of memory now\n");
      data->data_segment_size_ *= 2;

      unsigned char* new_data_segment = (unsigned char*) realloc(data->data_segment_, data->data_segment_size_);
      if (new_data_segment == NULL)
      {
        printf("[ERR] out of memory\n");
        //Do THIS IN A FUNCTION!!!!_______________
        int i;
        for (i = 0; i < data->number_of_loops_; i++)
        {
          free(bracket_index[i]);
          bracket_index[i] = NULL;
        }
        free(bracket_index);
        bracket_index = NULL;
        return OUT_OF_MEMORY;
      }

      unsigned char* start = &(new_data_segment[data->data_segment_size_ / 2]);
      memset(start, 0, data->data_segment_size_ / 2);
      data->data_segment_ = new_data_segment;
      printf("new memory done\n");
    }



    switch ((data->data_segment_)[current_command_counter])
    {
    case '<':
      //should fix the out of field bug

      if (program_counter > start_address)
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
        int index_of_current_command = getIndexOfBracket(bracket_index, data->number_of_loops_, current_command_counter, OPEN);
        current_command_counter = bracket_index[index_of_current_command][1];
      }
      break;
    case ']':
    {
      int index_of_current_command = getIndexOfBracket(bracket_index, data->number_of_loops_, current_command_counter, CLOSE);
      current_command_counter = bracket_index[(index_of_current_command)][0];
      current_command_counter--;
      break;
    }

    default:
      break;
    }
   
    current_command_counter++;
   
  }
  
  data->last_stop_in_code = current_command_counter;
  
  //DO THIS IN A FUNCTION!!!!
  int i;
  for (i = 0; i < data->number_of_loops_; i++)
  {
    free(bracket_index[i]);
    bracket_index[i] = NULL;
  }

  free(bracket_index);
  bracket_index = NULL;
  return SUCCESS;
}

int parseCode(unsigned char* data_segment, int*** brackets, int number_of_loops)
{
  int counter = 1;
  int latest_open_bracket = 0;
  int bracket_queue_counter = 0;
  int* bracket_queue = (int*) calloc(number_of_loops, sizeof (int));
  if (bracket_queue == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }

  while (data_segment[counter] != '\0')
  {
    if (number_of_loops == 0)
    {
      free(bracket_queue);
      return SUCCESS;
    }
    if (data_segment[counter] == '[')
    {

      (*brackets)[latest_open_bracket][0] = counter;
      *(bracket_queue + bracket_queue_counter++) = latest_open_bracket++;
    }
    else if (data_segment[counter] == ']')
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
    (*bracket_index)[col_count] = (int*) calloc(2, sizeof ((**bracket_index)[col_count]));
    if ((*bracket_index)[col_count] == NULL)
    {
      printf("[ERR] out of memory\n");
      col_count--;
      while (col_count >= 0)
      {
        free((*bracket_index)[col_count]);
        (*bracket_index)[col_count--] = NULL;
      }
      return OUT_OF_MEMORY;
    }
  }
  return SUCCESS;
}

int checkIfEqalWithBreakPoint(int current_command_counter, int** break_points)
{
  int counter = 0;
  int current_break_point = 0;
  while((current_break_point = (*break_points)[counter]) != -1)
  {
    if(current_break_point == 0)
      continue;
    
    if(current_command_counter == current_break_point)
    {
      (*break_points)[counter] = 0;
      return TRUE;
    }
      
    counter++;
  }
  return FALSE;
}