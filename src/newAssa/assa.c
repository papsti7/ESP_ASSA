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
// Latest Changes: 21.11.2015 (by Stefan Papst)
//------------------------------------------------------------------------------
//
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//for error message in fopen()
#include <asm/errno.h>




#define SUCCESS 0
#define FALSE_ARGUMENTS 1
#define OUT_OF_MEMORY 2
#define COULD_NOT_PARSE_FILE 3
#define READING_FILE_FAIL 4



int readCodeFromFile(char** data_segment, char* name);
char** readStd(void);
void freeAll(char** freeIt);

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
      if((return_value = readCodeFromFile(&data_segment, argv[2]) != 0))
        return return_value;
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
    while(1)
    {
      char** input = readStd();
      if(input == NULL)
      {
        freeAll(input);
        return OUT_OF_MEMORY;
      }
      if(strcmp(*input, "quit") == 0)
      {
        freeAll(input);
        printf("Bye\n");
        return SUCCESS;
      }
      if(strcmp(*input, "load") == 0)
      {
        if((return_value = readCodeFromFile(&data_segment, input[1])) != 0)
          return return_value;
        if(data_segment[strlen(data_segment) - 1] == '\n')
          data_segment[strlen(data_segment) - 1] = '\0';
      }
      if(strcmp(*input, "show") == 0)
      {
        int number = 0;
        int counter;
        if(data_segment == NULL)
        {
          printf("[ERR] no program loaded\n");
          return OUT_OF_MEMORY;
        }
        if(input[1] != NULL && (number = atoi(input[1])) != 0)
        {
          for(counter = 0; counter < number; counter++)
            printf("%c", data_segment[counter]);
          printf("\n");
        }else{
          printf("%s\n", data_segment);
        }
      }
      if(strcmp(*input,"run") == 0)
      {
        printf("This is the run function!\n");    //do something
      }
      freeAll(input);
    }
    return SUCCESS;
  }
  else
  {
    printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
    return FALSE_ARGUMENTS;
  }

  puts(data_segment);
  getchar();
  free(data_segment);
  return return_value;
}

int readCodeFromFile(char** data_segment, char* name)
{
  *data_segment = (char*) calloc(sizeof(char) * 1024, 0);
  if (*data_segment == NULL)
  {
    printf("[ERR] out of memory\n");
    return OUT_OF_MEMORY;
  }

  FILE* file =  fopen(name, "r");
//  errno_t error_type;
//  error_type = fopen_s(&file, name, "r");
  if (file == NULL)
  {
    printf("[ERR] reading the file failed\n");
//    printf("errno type: %s\n", strerror(error_type));
    return READING_FILE_FAIL;
  }

  int current_char;
  int data_segment_size = 1024;
  int counter = 0;
  while ((current_char = getc(file)) != EOF)
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
    counter++;
  }
  fclose(file);
  return SUCCESS;
}

char** readStd(void)
{
  char** inputString = (char**) calloc(sizeof(char*), 10);
  char* stringBuffer = NULL;
  char whitespace = 0;
  int count_params = 0;
  int counter_char = 0;
  char buffer = 0;
  int sizeAfter = 0;
  if(inputString == NULL)
  {
    printf("[ERR] out of memory\n");
    return NULL;
  }
  stringBuffer = (char*) calloc(sizeof(char), 100);
  if(stringBuffer == NULL)
  {
    printf("[ERR] out of memory\n");
    return NULL;
  }
  printf("esp> ");
  while((buffer = fgetc(stdin)) != EOF)
  {
    stringBuffer[counter_char] = buffer;
    if(buffer == '\n')
    {
      *(inputString + count_params) = (char*) calloc(sizeof(char), 100);
      if(*(inputString + count_params) == NULL)
      {
        printf("[ERR] out of memory\n");
        return NULL;
      }
      stringBuffer[counter_char] = '\0';
      strncpy(*(inputString + count_params), stringBuffer + sizeAfter, strlen(stringBuffer) + 1 - sizeAfter);
      break;
    }
    if(buffer == ' ' && whitespace != ' ')
    {
      *(inputString + count_params) = (char*) calloc(sizeof(char), 100);
      if(*(inputString + count_params) == NULL)
      {
        printf("[ERR] out of memory\n");
        return NULL;
      }
      strncpy(*(inputString + count_params), stringBuffer + sizeAfter, strlen(stringBuffer) + 1- sizeAfter);
      inputString[count_params][counter_char - sizeAfter] = '\0';
      if(*inputString[0] == '\0')
      {
        free(*inputString);
        count_params = 0;
        sizeAfter = 0;
        continue;
      }
      sizeAfter = counter_char;
      count_params++;
    }
    whitespace = buffer;
    if(whitespace == ' ')
    {
      sizeAfter++;
    }
    counter_char++;
  }
  free(stringBuffer);
  return inputString;
}
void freeAll(char** freeIt)
{
  int count = 0;
  while(*(freeIt + count) != NULL)
  {
    free(*(freeIt + count));
    count++;
  }
  free(freeIt);
  return;
}
