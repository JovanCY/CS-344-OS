#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <process.h>

int main()
{
   char testStr[20];

   fgets(testStr, 20, stdin);

   int i = 1;
   char *saveptr;
   char *token = strtok_r(testStr, " ", &saveptr);
   for (int i = 0; i < 5; i++)
   {
      token = strtok_r(NULL, " ", &saveptr);
      // if (strstr(testStr, "\n") == 0){
      //    printf("endl is in string");
      // }
      if (token)
      {
         printf("%s", token);
         if (strcmp(token, "\n") == 0)
         {
            printf("%s", token);
            printf("endl is in\n");
         }
      }
   }
}