#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

const char *getFileNameExt(const char *filename)
{
    // acquired from ThiefMaster from StackOverflow https://stackoverflow.com/questions/5309471/getting-file-extension-in-c
    // written on August 12, 2014
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "";
    return dot + 1;
}

int showMainMenu()
{
    char *menu;
    printf("\n");
    printf("1. Select a file to process\n");
    printf("2. Exit the Program\n");

    int input;
    scanf("%d", &input);
    fflush(stdout);
    return input;
}

int showSecondaryMenu()
{
    printf("\n");
    printf("Which file do you want to process?\n");
    printf("Enter 1 to pick the largest file\n");
    printf("Enter 2 to pick the smallest file\n");
    printf("Enter 3 to specify the name of a file\n");
    printf("Enter a choice from 1 to 3: ");
    fflush(stdout);

    int input;
    scanf("%d", &input);
    return input;
}

void processFile(const char *fileName)
{

    // open input file
    FILE *file_descriptor = fopen(fileName, "r");
    if (file_descriptor < 0)
    {
        fprintf(stderr, "Could not open %s\n", fileName);
        exit(1);
    }
    else
    {
        printf("Now processing the chosen file named %s\n", fileName);
        fflush(stdout);
    }

    char dirName[50];
    sprintf(dirName, "youngjov.movies.%d", rand() % 100000);
    mkdir(dirName, 0750);
    printf("created file with name %s\n", dirName);
    fflush(stdout);

    // make the files in the directory
    DIR *currDir = opendir(dirName);

    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;
    char *token;
    // reading lines from input file
    while ((nread = getline(&currLine, &len, file_descriptor)) != -1)
    {
        if (currLine[0] == '\0')
        {
            break;
        }
        char *tokenCopy = currLine;
        char *saveptr;

        char *token = strtok_r(input.mainLine, ",", &saveptr); // title



        token = strtok_r(NULL, ",", &saveptr); // year

        char filePath[50];
        if (strcmp(token, "Year") != 0)
        {
            sprintf(filePath, "%s/%s.txt", dirName, token);
            FILE *inputFile = fopen(filePath, "a");
            chmod(filePath,0640);
            fprintf(inputFile, "%s\n", currLine);
            fclose(inputFile);
        }
    }
    free(currLine);
    fclose(file_descriptor);
    closedir(currDir);
}

void largestFile()
{
    // get files in directory from 1.6 Slides "Everything is a file"
    // modified for the purposes of the assignment
    int bigDirSize = -1;                  // Modified timestamp of newest subdir examined
    char targetDirPrefix[32] = "movies_"; // Prefix we're looking for
    char largestDirName[256];             // Holds the name of the newest dir that contains prefix
    memset(largestDirName, '\0', sizeof(largestDirName));

    DIR *dirToCheck;           // Holds the directory we're starting in
    struct dirent *fileInDir;  // Holds the current subdir of the starting dir
    struct stat dirAttributes; // Holds information we've gained about subdir
    dirToCheck = opendir("."); // Open up the directory this program was run in
    if (dirToCheck > 0)
    { // Make sure the current directory could be opened

        while ((fileInDir = readdir(dirToCheck)) != NULL)
        { // Check each entry in dir
            char *dirName = fileInDir->d_name;
            const char *fileExt = getFileNameExt(dirName);
            if (strstr(dirName, targetDirPrefix) != NULL && strcmp(fileExt, "csv") == 0)
            { // If entry has prefix & ends with csv

                // printf("Found the prefix: %s\n", fileInDir->d_name);

                stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

                if ((int)dirAttributes.st_size > bigDirSize)
                { // If new file is bigger
                    bigDirSize = (int)dirAttributes.st_size;
                    memset(largestDirName, '\0', sizeof(largestDirName));
                    strcpy(largestDirName, fileInDir->d_name);
                    // printf("Bigger subdir: %s, new size: %d\n", fileInDir->d_name, bigDirSize);
                }
            }
        }
    }
    closedir(dirToCheck);
    fflush(stdout);

    processFile(largestDirName);
}

void smallestFile()
{
    // get files in directory from 1.6 Slides "Everything is a file"
    // modified for the purposes of the assignment
    int smallDirSize = 1000000;           // Updated size of the smallest file
    char targetDirPrefix[32] = "movies_"; // Prefix we're looking for
    char smallestDirName[256];            // Holds the name of the newest dir that contains prefix
    memset(smallestDirName, '\0', sizeof(smallestDirName));

    DIR *dirToCheck;           // Holds the directory we're starting in
    struct dirent *fileInDir;  // Holds the current subdir of the starting dir
    struct stat dirAttributes; // Holds information we've gained about subdir
    dirToCheck = opendir("."); // Open up the directory this program was run in
    if (dirToCheck > 0)
    { // Make sure the current directory could be opened

        while ((fileInDir = readdir(dirToCheck)) != NULL)
        { // Check each entry in dir

            char *dirName = fileInDir->d_name;
            const char *fileExt = getFileNameExt(dirName);
            if (strstr(dirName, targetDirPrefix) != NULL && strcmp(fileExt, "csv") == 0)
            { // If entry has prefix

                // printf("Found the prefix: %s\n", fileInDir->d_name);
                stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

                if ((int)dirAttributes.st_size < smallDirSize)
                { // If new file is bigger
                    smallDirSize = (int)dirAttributes.st_size;
                    memset(smallestDirName, '\0', sizeof(smallestDirName));
                    strcpy(smallestDirName, fileInDir->d_name);
                    // printf("Smaller subdir: %s, new size: %d\n", fileInDir->d_name, smallDirSize);
                }
            }
        }
    }
    closedir(dirToCheck);
    fflush(stdout);

    processFile(smallestDirName);
}

void specifiedFile()
{
    char fileName[50];
    printf("Enter the complete file name:\n");
    fflush(stdout);
    scanf("%s", &fileName);

    // test to see if fil exists
    FILE *test = fopen(fileName, "r");
    if (test == NULL)
    {
        printf("the file %s was not found. Try again\n", fileName);
        fflush(stdout);
        return;
    }

    processFile(fileName);
}

void selectFile()
{
    int userChoice = 0;
    int a = 1;
    while (a == 1)
    {
        userChoice = showSecondaryMenu();
        switch (userChoice)
        {
        case 1: // largest file
            largestFile();
            a = 0;
            break;
        case 2: // smallest file
            smallestFile();
            a = 0;
            break;
        case 3: // specify the name of file
            specifiedFile();
            a = 0;
            break;
        default:
            printf("Please enter a valid input");
            fflush(stdout);
        }
    }
}

int main(int argc, char *argv[])
{

    // variables that store the movies
    int numMovies = 0;

    struct movie *currMovie = NULL;
    struct movie *firstMovie = NULL;

    // getting the lines into the variables

    // run program based on menu choice
    int type;
    int loop = 1;
    while (loop == 1)
    {
        type = showMainMenu();
        switch (type)
        {
        case 1: // select file to process
            selectFile();
            break;
        case 2: // exit
            return 1;
            break;
        default:
            printf("Please enter a valid input");
            fflush(stdout);
        }
    }
}