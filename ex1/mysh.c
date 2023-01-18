#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"
#include "stddef.h"

#define  MAX_INPUT_LENGTH 1024
#define  MAX_NUM_ARGS 100
#define  MAX_ARG_LENGTH 100



int specCharacter(char c){
  char* list = "><|";
  if (strchr(list, c) == NULL) {
    return -1;
  }
  else{
    return 1;
  }
}

int numOfSpec(char symbol, char** arguments){
  int counter = 0;
  int j = 0;
  char symbolString[2];
  symbolString[0] = symbol;
  symbolString[1] = '\0';

  while(arguments[j] != NULL) {
    if (strcmp(arguments[j], symbolString) == 0) {
      counter++;
    }
    j++;
  }
  return counter;
}

//Taken from techiedelight.com
char* strcat(char* destination, const char* source){
	char* ptr = destination + strlen(destination);
	while (*source != '\0')
		*ptr++ = *source++;
	*ptr = '\0';
	return destination;
}

int numofWords(char* string){
  int spaces = 0;
  for (int i = 0; i < strlen(string); i++) {
    if (string[i] == 32) {
      spaces++;
    }
  }
  return spaces+1;
}

int indexOfChar (char symbol, char** arguments){
  int j = 0;
  char symbolString[2];
  symbolString[0] = symbol;
  symbolString[1] = '\0';

  while(arguments[j] != NULL) {
    if (strcmp(arguments[j], symbolString) == 0) {
      return j;
    }
    j++;
  }
  return -1;
}

char** splitargs(char* buf){
  char** arguments =  (char**)malloc(sizeof(char*) * MAX_NUM_ARGS * MAX_ARG_LENGTH);
  char* currentArg = (char*)malloc(sizeof(char) * MAX_ARG_LENGTH);
  int inQuotes = 0;
  int numOfArgs = 0;
  char temp[2];
  temp[1] = '\0';

  for (int i = 0; i < strlen(buf); i++) {
    if (inQuotes == 0) {
      if (buf[i] == '\"') {
        inQuotes = 1;
      }
      else{
        if (buf[i] == ' ' || i == (strlen(buf) - 1)){
          //strcat(currentArg, "\0");9
          arguments[numOfArgs] = (char*)malloc(sizeof(char) * strlen(currentArg));
          strcpy(arguments[numOfArgs], currentArg);

          memset(currentArg, 0, sizeof(char) * MAX_ARG_LENGTH);
          numOfArgs++;
        }
        else{
          temp[0] = buf[i];
          strcat(currentArg, temp);
        }
      }
    }
    else{
      if (buf[i] == '\"') {
        inQuotes = 0;
      }
      else{
        temp[0] = buf[i];
        strcat(currentArg, temp);
      }
    }
  }
  return arguments;

  // //array of arguments that gets returned
  // char** arguments =  (char**)malloc(sizeof(char*) * MAX_NUM_ARGS * MAX_ARG_LENGTH);
  // int numOfArgs = 0;
  // int charNum = 0;
  //
  // char* currentArg = (char*)malloc(sizeof(char) * MAX_ARG_LENGTH);
  // printf("INSIDE SPLIT\n");
  // for (int i = 0; i < numofWords(buf); i++) {
  //   int j = 0;
  //   currentArg = "";
  //   while (buf[charNum] != 32 && (strlen(buf) - i )>1) {
  //     if (buf[charNum] != ';') {
  //       currentArg[j] = buf[charNum];
  //       j++;
  //     }
  //     charNum++;
  //   }
  //   currentArg[j] = '\0';
  //   charNum++;
  //
  //   if (strlen(currentArg) > 0) {
  //     printf("INSIDE SPLIT2\n");
  //     arguments[numOfArgs] = (char*)malloc(sizeof(char)*100);
  //     strcpy(arguments[numOfArgs], currentArg);
  //     printf("INSIDE SPLIT3\n");
  //   }
  //   numOfArgs++;
  //   memset(currentArg, 0, sizeof(char) * MAX_ARG_LENGTH);
  // }
  // printf("INSIDE SPLIT4\n");
  // arguments[numOfArgs] = (char*)malloc(sizeof(char)*100);
  // arguments[numOfArgs] = '\0';
  // printf("INSIDE SPLIT5\n");
  //
  //
  // return arguments;
}


int arraylen(char** a) {
  int i = 0;
  int count = 0;
  while(a[i] != NULL) {
    count++;
    i++;
  }
  return count;
}

void run(char** arguments) {
    char *file = malloc(sizeof(char));
    file[0] = '/';

    int arrayLength = arraylen(arguments);




    if (numOfSpec('|', arguments) > 0) {
      int p[2];
      pipe(p);
      int k = 0, j = 0;
      int pipeSeen = 0;


      char** toRun = (char**)malloc(sizeof(char)* MAX_ARG_LENGTH * MAX_NUM_ARGS);
      char** passOn = (char**)malloc(sizeof(char)* MAX_ARG_LENGTH * MAX_NUM_ARGS);

      //this for loop splits the argument array into two new arguemnt arrays, one to be run and the other to be passed on.
      for (int i = 0; i < arrayLength; i++) {
        if (pipeSeen == 0 && strcmp("|" , arguments[i]) == 0) {
          pipeSeen = 1;
          continue;
        }
        //needs to be a new if statement because otherwise the pipe character won't be included in the arguments array to be passed on
        if (pipeSeen == 0) {
          //DO I NEED THIS??
          toRun[k] = (char*)malloc(sizeof(char)*MAX_ARG_LENGTH);
          strcpy(toRun[k], arguments[i]);
          k++;
        }
        else{
          passOn[j] = (char*)malloc(sizeof(char)*MAX_ARG_LENGTH);
          strcpy(passOn[j], arguments[i]);
          j++;
        }

      }

      if (fork()==0) {
        close(1);
        dup(p[1]);
        close(p[0]);
        close(p[1]);
        run(toRun);
        exit(0);
      }
      else if (numOfSpec('|',passOn) != 0 || numOfSpec('>', passOn) != 0 || numOfSpec('<', passOn) == 0) {
        if (fork() == 0) {
          close(p[1]);
          close(0);
          dup(p[0]);
          close(p[0]);
          run(passOn);
          exit(0);
        }
      }
      else{
        if(fork() == 0) {
          run(passOn);
          exit(0);
        }
      }
      close(p[0]);
      close(p[1]);
      wait(0);
      wait(0);
    }
    else if (numOfSpec('<', arguments) > 0 || numOfSpec('>', arguments) > 0) {
      char** toRun = (char**)malloc(sizeof(char)* MAX_ARG_LENGTH * MAX_NUM_ARGS);
      int counter = 0;

      for (int i = 0; i < arrayLength; i++) {
        if (strcmp("<", arguments[i]) == 0) {
          close(0);
          open(arguments[i+1], O_RDWR);
          exec(toRun[0], toRun);
        }
        else if (strcmp(">", arguments[i]) == 0) {
          close(1);
          open(arguments[i+1], O_TRUNC | O_CREATE | O_RDWR);
          exec(toRun[0], toRun);
        }
        else {
          toRun[counter] = (char*)malloc(sizeof(char)* MAX_ARG_LENGTH);
          strcpy(toRun[counter], arguments[i]);
          if (i == 0) {
            if (toRun[0][0] != '/') {
             strcat(file,toRun[0]);
             strcpy(toRun[0], file);
            }
          }
          counter++;
        }
      }
      memset(toRun, 0, sizeof(char)*MAX_ARG_LENGTH * MAX_NUM_ARGS);
    }
    else{
      //DO THIS BEFORE EVERY EXECUTION
      if (arguments[0][0] != '/') {
        strcat(file,arguments[0]);
        strcpy(arguments[0], file);
      }
      exec(arguments[0], arguments);
      printf("the exec has failed\n");
      exit(0);
    }
}

void splitcommand(char ** arguments){
  if (numOfSpec(';',arguments) > 0) {
    char** command = (char**)malloc(sizeof(char*) * MAX_NUM_ARGS * MAX_ARG_LENGTH);
    int i = 0, com = 0;

    while (arguments[i] != NULL) {
      if (strcmp(arguments[i], ";") == 0) {
        if (fork()==0) {
          run(command);
          exit(0);
        }
      }
      //last argument
      else if (arguments[i+1] == NULL) {
        command[com] = (char*)malloc(sizeof(arguments[i]));
        strcpy(command[com], arguments[i]);
        if (fork()==0) {
          run(command);
          exit(0);
        }
      }
      else{
        command[com] = (char*)malloc(sizeof(arguments[i]));
        strcpy(command[com], arguments[i]);
        com++;
      }
      wait(0);
      i++;
    }
  }
  else{
    run(arguments);
  }
}




void main(int argc, char const *argv[]) {

  while (1) {
    //input string
    char* buf = malloc(sizeof(char)*MAX_INPUT_LENGTH);
    //array of arguments
    char** arguments = (char**)malloc(sizeof(char*) * MAX_NUM_ARGS * MAX_ARG_LENGTH);

    printf(">>> ");
    read(0, buf, MAX_INPUT_LENGTH);
    arguments = splitargs(buf);


    if (strcmp(arguments[0], "cd") == 0) {
      chdir(arguments[1]);
      }
    else if (strcmp(arguments[0], "exit") == 0) {
      exit(0);
      }
    else if(fork()==0){
      splitcommand(arguments);
      }
    wait(0);
    memset(buf,0,strlen(buf));
    memset(arguments,0, sizeof(char*) * MAX_NUM_ARGS * MAX_ARG_LENGTH);
  }
}
