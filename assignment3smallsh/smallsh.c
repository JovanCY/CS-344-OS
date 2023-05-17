#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

int backgroundSet = 0;
int statusCode = 0;
int background = 0;

struct variables
{
   char mainLine[2048]; // to hold the input line
   char *arguments[512];
   char *command;
   char *inputName;
   char *outputName;
   int argumentCount;
   int hasInput;
   int hasOutput;
};

void ZFunc(int sig)
{
   if (backgroundSet == 0)
   {
      char *message = "\nEntering foreground-only mode (& is now ignored)\n";
      write(STDOUT_FILENO, message, 50);

      backgroundSet = 1;
   }
   else if (backgroundSet == 1)
   {
      char *message = "\nExiting foreground-only mode\n";
      write(STDOUT_FILENO, message, 30);

      backgroundSet = 0;
   }
}

// Function to replace a string with another
// string from GeeksForGeeks on: https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/
char *replaceWord(const char *s, const char *oldW, const char *newW)
{
   char *result;
   int i, cnt = 0;
   int newWlen = strlen(newW);
   int oldWlen = strlen(oldW);

   // Counting the number of times old word
   // occur in the string
   for (i = 0; s[i] != '\0'; i++)
   {
      if (strstr(&s[i], oldW) == &s[i])
      {
         cnt++;

         // Jumping to index after the old word.
         i += oldWlen - 1;
      }
   }
   // Making new string of enough length
   result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);

   i = 0;
   while (*s)
   {
      // compare the substring with the result
      if (strstr(s, oldW) == s)
      {
         strcpy(&result[i], newW);
         i += newWlen;
         s += oldWlen;
      }
      else
         result[i++] = *s++;
   }

   result[i] = '\0';
   return result;
}

void executeCommand(struct variables input)
{
   pid_t spawnpid;
   // from Exploration: Process API - Executing a new Program
   // https://canvas.oregonstate.edu/courses/1890464/pages/exploration-process-api-creating-and-terminating-processes?module_item_id=22345179
   int childStatus;
   int devIn = 0;
   int devOut = 0;

   // Fork a new process
   pid_t spawnPid = fork();
   int inputStat;
   int outStat;

   switch (spawnPid)
   {
   case -1:
      perror("fork() error\n");
      exit(1);
      break;
   case 0:

      // In the child process
      // printf("CHILD(%d) running ls command\n", getpid());
      if (input.hasInput == 1)
      {
         inputStat = open(input.inputName, O_RDONLY);
         if (inputStat == -1)
         {
            printf("cannot open %s for input\n", input.inputName);
            fflush(stdout);
            exit(1);
         }
         statusCode = dup2(inputStat, 0);
         close(inputStat);
         if (statusCode == -1)
         {
            perror("source dup2()\n");
            exit(1);
         }
      }

      if (input.hasOutput == 1)
      {
         outStat = open(input.outputName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
         if (outStat == -1)
         {
            printf("cannot open %s for output\n", input.outputName);
            fflush(stdout);

            exit(1);
         }
         // redirects output to opened file
         statusCode = dup2(outStat, 1);
         close(outStat);
         if (statusCode == -1)
         {
            perror("source dup2()\n");

            exit(1);
         }
      }

      statusCode = execvp(input.arguments[0], input.arguments);
      if (statusCode < 0)
      {
         printf("%s: command not found\n", input.arguments[0]);
         fflush(stdout);
         exit(1);
      }

      // exec only returns if there is an error
      perror("execvp");
      exit(2);
      break;
   default:
      // In the parent process
      // Wait for child's termination
      spawnPid = waitpid(spawnPid, &childStatus, 0);
      // printf("PARENT(%d): child(%d) terminated. Exiting\n", getpid(), spawnPid);
      exit(0);
      break;

   }
}
int main(int argc, char *argv[])
{

   struct variables input; // to hold the input variables
   input.argumentCount = 0;
   input.hasInput = 0;
   input.hasOutput = 0;
   int shell = 1; // for shell loop exit
   int pid = getpid();
   char strPid[6];             // to hold string version of pid
   sprintf(strPid, "%d", pid); // moves pid to strpid
   int childProcess = 0;       // to hold variable whether & is called or not

   // ignore SIGINT (^C)
   struct sigaction ignSigInt = {0};
   ignSigInt.sa_handler = SIG_IGN;
   sigfillset(&ignSigInt.sa_mask);
   ignSigInt.sa_flags = 0;

   sigaction(SIGINT, &ignSigInt, NULL);

   // ignore SIGSTP
   struct sigaction ignSigtstp = {0};
   ignSigtstp.sa_handler = ZFunc;
   sigfillset(&ignSigtstp.sa_mask);
   ignSigtstp.sa_flags = 0;
   sigaction(SIGTSTP, &ignSigtstp, NULL);

   while (shell == 1)
   {
      // receiving input
      printf(": ");
      fflush(stdout);
      // scanf("%s", &input.mainLine);
      fgets(input.mainLine, 2048, stdin); // string always ends with endl

      char command[512];

      // check if empty or is a comment
      if (input.mainLine[0] == '#' || input.mainLine[0] == '\n' || input.mainLine[0] == '\0')
      {
         continue;
      }
      if (input.mainLine[0] == ' ')
      { // spam spaces will not be detected above
         int empty = 1;
         for (int i = 0; i < 2048; i++)
         {
            if (input.mainLine[i] == '\n' || input.mainLine[i] == '\0') // endl meaning string has ended
            {
               break;
            }
            else if (!(input.mainLine[i] == ' ')) // not a space nor endl
            {
               empty = 0;
               break;
            }
         }
         if (empty == 1)
         {
            continue;
         }
      }

      // cleaning endl from input.mainLine
      if (strstr(input.mainLine, "\n") != 0)
      {
         for (int i = 0; i < 2048; i++)
         {
            if (input.mainLine[i] == '\n')
            {
               input.mainLine[i] = '\0';
               break;
            }
         }
      }

      // replace $$ to pid in case $$ pops up multiple times
      while (strstr(input.mainLine, "$$") != 0)
      {
         // find $$
         for (int i = 0; i < 2048; i++)
         {
            if (input.mainLine[i] == '$' && input.mainLine[i + 1] == '$')
            { // delete $$ from the string
               input.mainLine[i] = '\0';
               input.mainLine[i + 1] = '\0';
               break;
            }
         }
         strcat(input.mainLine, strPid);
      }

      char *saveptr;
      input.command = strtok_r(input.mainLine, " ", &saveptr); // command
      strcpy( input.arguments[0],input.command);
      input.argumentCount++;

      int argsLoop = 1; // boolean
      while (argsLoop == 1)
      {
         // check if it's input, output, or child Process
         char *token = strtok_r(NULL, " ", &saveptr);
         if (token) // prevent segfault if token is NULL
         {
            if (strcmp(token, "<") == 0) // if it's input, put into input
            {
               input.inputName = strtok_r(NULL, " ", &saveptr);
               input.hasInput = 1;
            }
            else if (strcmp(token, ">") == 0)
            {
               input.outputName = strtok_r(NULL, " ", &saveptr);
               input.hasOutput = 1;
            }
            else if (strcmp(token, "&") == 0)
            {
               childProcess = 1;
            }
            else
            {
               // strcpy(input.arguments[input.argumentCount], token);
               input.arguments[input.argumentCount] = token;
               input.argumentCount++;
            }
         }
         else
         { // if token is NULL
            argsLoop = 0;
         }
      }

      // command cases

      if (strcmp(input.command, "exit") == 0)
      {
         // maybe shut off zombies
         shell = 0;
         break;
      }
      else if (strcmp(input.command, "cd") == 0)
      {
         if (!input.arguments[1]) // if there are no provided arguments
         {
            chdir(getenv("HOME"));
         }
         else
         { // if arguments are provided //MAKE IT SO THAT THEY CAN USE RELATIVE PATH OR SOLID PATH

            if (chdir(input.arguments[1]) != 0)
            { // if dir doesn't exists
               printf("directory not found\n");
               fflush(stdout);
            }
            else
            { // if dir exists
               chdir(input.arguments[1]);
            }
         }
         // exec();
      }
      else if (strcmp(input.command, "status") == 0)
      {
         if (WIFEXITED(statusCode) != 0)
         {
            int exitStatus = WEXITSTATUS(statusCode);

            printf("exit value %d\n", exitStatus);
            fflush(stdout);
         }
         else if (WIFSIGNALED(statusCode) != 0)
         {
            int termSignal = WTERMSIG(statusCode);

            printf("terminated by signal %d\n", termSignal);
            fflush(stdout);
         }
      }
      else // execute other commands
      {
         executeCommand(input);
      }

      childProcess = 0;
      background = 0;
   }

   return 1;
}