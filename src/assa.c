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
#include <errno.h>




#define SUCCESS 0
#define FALSE_ARGUMENTS 1
#define OUT_OF_MEMORY 2
#define PARSE_FILE_ERROR 3
#define READING_FILE_FAIL 4

#define TRUE 0
#define FALSE 1
#define NEUTRAL -1


/*------------HEADER NEED TO BE DONE-----------*/
int readCodeFromFile(char** data_segment, char* name, int* code_length);
int checkCodeCorrectness(char* data_segment, int* number_of_loops);
int checkCommandOrComment(char current_char);
int runCode(char* data_segment, int code_length);
int parseCode(char* data_segment, int*** brackets, int number_of_loops);


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
  char* data_segment = NULL;
  int return_value = SUCCESS;

  if (argc == 3)
  {
    if (!strcmp(argv[1], "-e"))
    {
      int code_length = 0;
      if((return_value = readCodeFromFile(&data_segment, argv[2], &code_length) != 0))
        return return_value;
      runCode(data_segment, code_length);
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
  
  
  //puts(data_segment);
  
  getchar();
  
  //---------This free is failing with visual studio, I'll test it on unix and fix it later. --------//
  //free(data_segment);
  return return_value;
}

int readCodeFromFile(char** data_segment, char* name, int* code_length)
{
  *data_segment = (char*) calloc(1024, sizeof(char));
  if (*data_segment == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }
  //set end of string
  (*data_segment)[(int)(1024 * 0.8 + 1)] = '\0';

  //open file
  FILE* file;
  errno_t error_type;
  error_type = fopen_s(&file, name, "r");
  if (file == NULL)
  {
    printf("[ERR] reading the file failed\n");
    printf("errno type: %s\n", strerror(error_type));
    return READING_FILE_FAIL;
  }
  
  int current_char;
  int data_segment_size = 1024;
  int counter = 1;
  while ((current_char = getc(file)) != EOF)
  {
    if (checkCommandOrComment(current_char) == TRUE)
    {
      if (counter < (int)(data_segment_size * 0.8))
      {
        (*data_segment)[counter] = current_char;
        (*data_segment)[counter + 1] = '\0';

      }
      else
      {
        char* new_datasegment = NULL;
        data_segment_size *= 2;

        new_datasegment = (char*)realloc(*data_segment, data_segment_size);
        if (new_datasegment == NULL)
        {
          printf("[ERR] out of memory\n");
          free(*data_segment);
          return OUT_OF_MEMORY;
        }

        *data_segment = new_datasegment;
        (*data_segment)[counter] = current_char;
        (*data_segment)[counter + 1] = '\0';
      }
      *code_length = (++counter) + 1;
    }
    (*data_segment)[0] = '\0';
  }
  //puts(*data_segment);

  fclose(file);
  return SUCCESS;
}

int checkCodeCorrectness(char* data_segment, int* number_of_loops)
{
 
  //1 because in data_segment[0] is a value of how long the code part is. 
  int counter = 1;
  int open_brackets_count = 0;
  int close_brackets_count = 0;
  while (data_segment[counter] != '\0')
  {
    if(data_segment[counter] == '[')
      open_brackets_count++;
    else if(data_segment[counter] == ']')
      close_brackets_count++;
    counter++;
  }
  *number_of_loops = open_brackets_count;
  return open_brackets_count - close_brackets_count;
}

int checkCommandOrComment(char current_char)
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

int runCode(char* data_segment, int code_length)
{
  int number_of_loops = 0;
  if (checkCodeCorrectness(data_segment, &number_of_loops) != SUCCESS)
  {
    printf("[ERR] parsing of input failed\n");
    //---------This free is failing with visual studio, I'll test it on unix and fix it later. --------//
    //free(data_segment);
    return PARSE_FILE_ERROR;
  }

  int** bracket_index = (int**) calloc(number_of_loops, sizeof(int*));
  if (bracket_index == NULL)
  {
    printf("[ERR] out of memory\n");
    //free(data_segment);
    return OUT_OF_MEMORY;
  }
  int col_count;
  for (col_count = 0; col_count < number_of_loops; col_count++)
  {
    bracket_index[col_count] = (int*) calloc(2, sizeof(*bracket_index[col_count]));
    if (bracket_index[col_count] == NULL)
    {
      printf("[ERR] out of memory\n");
      //free(data_segment);
      //free all parts with where allocated till now !!!!
      return OUT_OF_MEMORY;
    }
  }
  
  
  parseCode(data_segment, &bracket_index, number_of_loops);

  // first(0-te) element is the length of the code
  
  int current_command_counter = 1;
  
  int* bracket_queue = (int*)calloc(number_of_loops, sizeof(int));
  if (bracket_queue == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }
  int latest_open_bracket = 0;
  int first_opened_bracket = 0;

/*
//--------------------------------------------------------------------------------------
// TODO: implement run function , maybe a runFromTO() function run the code between brackets
//-------------------------------------------------------------------------------------
*/


  //data - part of segment
  char* program_counter = data_segment + code_length;
  int bracket_counter = 0;

  while (data_segment[current_command_counter] != '\0')
  {
    switch (data_segment[current_command_counter])
    {
      case '<':
        --program_counter;
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
          /*----ERR: latest_open_bracket is out of bounds, FIX IT!----*/
          current_command_counter = bracket_index[(--latest_open_bracket)][1];
          latest_open_bracket = bracket_queue[latest_open_bracket - 1];
        }
          
        else if(latest_open_bracket > 0)
        {
          if ((latest_open_bracket - 1) != bracket_queue[(latest_open_bracket - 1)])
          {
            bracket_queue[latest_open_bracket++] = latest_open_bracket;
          }
        }
        else if (latest_open_bracket == 0)
        {
          bracket_queue[latest_open_bracket++] = latest_open_bracket;
        }
        break;
      case ']':
        current_command_counter = bracket_index[(latest_open_bracket - 1)][0];
        current_command_counter--;
        break;
      default:
        break;
    }
    //puts("next command\n");
    current_command_counter++;
  }
  //free(bracket_queue);
  return SUCCESS;
}

int parseCode(char* data_segment, int*** brackets, int number_of_loops)
{
  int counter = 1;
  int latest_open_bracket = 0;
  int bracket_queue_counter = 0;
  int* bracket_queue = (int*) calloc(number_of_loops, sizeof(int));
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
      //error!!!
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
  return PARSE_FILE_ERROR;
}