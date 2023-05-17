#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

const char *getFileNameExt(const char *filename)
{
   // acquired from ThiefMaster from StackOverflow https://stackoverflow.com/questions/5309471/getting-file-extension-in-c
   // written on August 12, 2014
   const char *dot = strrchr(filename, '.');
   if (!dot || dot == filename)
      return "";
   return dot + 1;
}

int main()
{
   // char randomInfo[50] = "this is test";
   // printf(randomInfo);
   // sprintf(randomInfo, "will");
   // printf(randomInfo);
   // fflush(stdout);

   FILE *inputFile = fopen("random.txt", "w");

   if (inputFile ==NULL){
      printf("file doesn't exist");
      fflush(stdout);
   } else {
      printf("file exists");
      fflush(stdout);
   }
   // fprintf(inputFile, "this is just a test\n");
   // fprintf(inputFile, "this is another test\n");

   fclose(inputFile);

   // char* testStr = "Year";
   // int testInt = (int)testStr;
   // printf("%d",testInt);

   return 1;
}