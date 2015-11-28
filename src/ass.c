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
#define INPSIZE 5
#define COMMANDSIZE 100

typedef struct{
    char** inputString;
    int inputRange;
    int error;
}Input;
typedef struct{
    char* data_segment;
    int readFileError;
    int memoryError;
}Data;
Data readCodeFromFile(Data data_segment, char* name);
Input readStd();
void freeAll(char** input, int countInpMalloc);

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
  data.data_segment = NULL;
  data.memoryError = 0;
  data.readFileError = 0;
  int return_value = SUCCESS;

  if (argc == 3)
  {
    if (!strcmp(argv[1], "-e"))
    {
      data = readCodeFromFile(data, argv[2]);
      if(data.memoryError == OUT_OF_MEMORY)
      {
        free(data.data_segment);
        return OUT_OF_MEMORY;
      }
      if(data.readFileError == READING_FILE_FAIL)
      {
        free(data.data_segment);
        return READING_FILE_FAIL;
      }
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
    Input input;
    printf("esp> ");
    input = readStd();
    if(input.error == OUT_OF_MEMORY)
      return OUT_OF_MEMORY;
    if(input.inputRange < 1)
    {
      printf("[ERR] wrong parameter count\n");
      freeAll(input.inputString, input.inputRange);
      continue;
    }
    if(strcmp(input.inputString[0], "quit") == 0)
    {
      freeAll(input.inputString, input.inputRange);
      if(data.data_segment != NULL)
        free(data.data_segment);
      printf("Bye\n");
      return SUCCESS;
    }
    if(strcmp(input.inputString[0], "load") == 0)
    {
      if(data.data_segment != NULL)
      {
      	free(data.data_segment);
      }
      if(input.inputRange < 2)
      {
        printf("[ERR] wrong parameter count\n");
        freeAll(input.inputString, input.inputRange);
        continue;
      }
      data = readCodeFromFile(data, input.inputString[1]);
      if(data.memoryError == OUT_OF_MEMORY)
      {
        free(data.data_segment);
        freeAll(input.inputString, input.inputRange);
        return OUT_OF_MEMORY;
      }
      if(data.readFileError == READING_FILE_FAIL)
      {
        free(data.data_segment);
        freeAll(input.inputString, input.inputRange);
        return READING_FILE_FAIL;
      }
    }
    if(strcmp(input.inputString[0], "show") == 0)
      {
        int number = 0;
        int counter;
        if(data.data_segment == NULL)
        {
          printf("[ERR] no program loaded\n");
          freeAll(input.inputString, input.inputRange);
          return OUT_OF_MEMORY;
        }
        if(input.inputRange < 2)
        {
          for(counter = 0; counter < 10; counter++)
          {
            if(data.data_segment[counter] == '\0')
              break;
            printf("%c", data.data_segment[counter]);
          }
          printf("\n");
        }else{
          if((number = atoi(input.inputString[1])) != 0)
          {
            for(counter = 0; counter < number; counter++)
            {
              if(data.data_segment[counter] == '\0')
                break;
              printf("%c", data.data_segment[counter]);
            }
            printf("\n");
          }
        }
      }
      if(strcmp(input.inputString[0],"run") == 0)
      {
        printf("This is the run function!\n");    //do something
      }
    freeAll(input.inputString, input.inputRange);
  }
    return SUCCESS;
  }
  else
  {
    printf("[ERR] usage: ./assa [-e brainfuck_filnename]\n");
    return FALSE_ARGUMENTS;
  }

  puts(data.data_segment);
  getchar();
  free(data.data_segment);
  return return_value;
}

Data readCodeFromFile(Data data, char* name)
{
  data.data_segment = (char*) malloc(sizeof(char) * 1024);
  data.memoryError = 0;
  data.readFileError = 0;
  if (data.data_segment == NULL)
  {
    printf("[ERR] out of memory\n");
    data.memoryError = OUT_OF_MEMORY;
    return data;
  }

  FILE* file =  fopen(name, "r");
//  errno_t error_type;
//  error_type = fopen_s(&file, name, "r");
  if (file == NULL)
  {
    printf("[ERR] reading the file failed\n");
//    printf("errno type: %s\n", strerror(error_type));
    free(data.data_segment);
    memset(&data.data_segment, 0, sizeof(data.data_segment));
    data.readFileError = READING_FILE_FAIL;
    return data;
  }

  char current_char = 0;
  int data_segment_size = 1024;
  int counter = 0;
  while ((current_char = fgetc(file)) != EOF)
  {
    if (counter < (int)(data_segment_size * 0.8))
    {
      data.data_segment[counter] = current_char;
    }
    else
    {
      char* new_datasegment = NULL;
      data_segment_size *= 2;

      new_datasegment = (char*)realloc(data.data_segment, sizeof(char) * data_segment_size);
      if (new_datasegment == NULL)
      {
        printf("[ERR] out of memory\n");
        free(data.data_segment);
        data.memoryError = OUT_OF_MEMORY;
        return data;
      }
      data.data_segment = new_datasegment;
      data.data_segment[counter] = current_char;
    }
    counter++;
  }
  data.data_segment[counter] = '\0';
  fclose(file);
  return data;
}
Input readStd()
{
  int size = 0;
  int i;
  int increased = 1;
  int marked = 0;
  int counter = 0;
  char string3[COMMANDSIZE];
  memset(string3, 0, COMMANDSIZE);
  Input input;
  input.error = 0;
  input.inputRange = 0;
  input.inputString = NULL;
  input.inputString = (char**) malloc(sizeof(char*) * INPSIZE);
  if(input.inputString == NULL)
  {
    printf("[ERR] out of memory\n");
    input.error = OUT_OF_MEMORY;
    return input;
  }
  
  char* string = (char*) malloc(sizeof(char) * COMMANDSIZE);
  
  if(string == NULL)
  {
    printf("[ERR] out of memory\n");
    free(input.inputString);
    input.error = OUT_OF_MEMORY;
    return input;
  }
  char* string2 = (char*) malloc(sizeof(char) * COMMANDSIZE);
  if(string2 == NULL)
  {
    printf("[ERR] out of memory\n");
    freeAll(input.inputString, input.inputRange);
    free(string);
    input.error = OUT_OF_MEMORY;
    return input;
  }
  while(fgets(string, COMMANDSIZE, stdin) != NULL)
  {
    if(marked == 0)
    {
      strcpy(string2, string);
      marked = 1;
    }
    size += (int)strlen(string2);
    
    if(size >= COMMANDSIZE * increased)
    {
      char* save = NULL;
      increased++;
      save = (char*) realloc(string2, sizeof(char) * COMMANDSIZE * increased);
      if(save == NULL)
      {
        printf("[ERR] out of memory\n");
        freeAll(input.inputString, input.inputRange);
        input.error = OUT_OF_MEMORY;
        free(string2);
        free(string);
        return input;
      }else{
        string2 = save;
        strcat(string2, string);
      }
    }
    if(string[strlen(string) - 1] == '\n')
    {
      break;
    }
  }
  free(string);
  
  increased = 1;
  while(string2[counter] != '\n')
  {
    i = 0;
    while(string2[counter] == ' ')
      counter++;
    
    while(string2[counter] != ' ' && string2[counter] != '\n')
    {
      if(input.inputRange >= INPSIZE * increased)
      {
        increased++;
        char** save2 = NULL;
        save2 = (char**) realloc(input.inputString, sizeof(char*) * INPSIZE * increased);
        if(save2 == NULL)
        {
          printf("[ERR] out of memory\n");
          freeAll(input.inputString, input.inputRange);
          input.error = OUT_OF_MEMORY;
          free(string2);
          return input;
        }else{
          input.inputString = save2;
        }
      } 
      string3[i] = string2[counter];
      i++;
      counter++;
    }
    string3[strlen(string3)] = '\0';
    input.inputString[input.inputRange] = (char*) malloc(sizeof(char) * COMMANDSIZE);
      if(input.inputString[input.inputRange] == NULL)
      {
        printf("[ERR] out of memory\n");
        freeAll(input.inputString, input.inputRange);
        free(string2);
        input.error = OUT_OF_MEMORY;
        return input;
      }
    strcpy(input.inputString[input.inputRange], string3);
    memset(string3, 0, COMMANDSIZE);
    input.inputRange++;
  }
  free(string2);
  return input;
  
}
void freeAll(char** input, int countInpMalloc)
{
  int countFree;
  for(countFree = 0; countFree < countInpMalloc; countFree++)
  {
    free(input[countFree]);
  }
  free(input);
}

