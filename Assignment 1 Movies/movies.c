#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>


struct movie
{
    char *title;
    int year;
    char *language;
    double rating;
    struct movie *nextMovie;
};

struct movie *createMovie(char *currLine)
{
    
    struct movie *currMovie = malloc(sizeof(struct movie));
    
    // For use with strtok_r
    char *saveptr;

    // The title
    
    char *token = strtok_r(currLine, ",", &saveptr);
    // printf("title input = %s\n", token);
    currMovie->title = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->title, token);

    // The first token is the year
    char *display = strtok_r(NULL, ",", &saveptr);
    // printf("year input = %s\n", display);
    int tokenInt = atoi(display);
    currMovie->year = tokenInt;
    
    // movieLang
    char *line = strtok_r(NULL, ",", &saveptr);
    // printf("language input = %s\n", line);
    
    currMovie->language = calloc(strlen(line) + 1, sizeof(char));
    strcpy(currMovie->language, line);

    // The first token is the rating
    char *ptr;
    // throwaway variable for strtod
    char *test = strtok_r(NULL, " ", &saveptr);
    double tokenD = 0;
    tokenD = strtod(test, &ptr);
    currMovie->rating = tokenD;

    // Set the next node to NULL in the newly created student entry
    currMovie->nextMovie = NULL;
    
    return currMovie;
}

void printMovie(struct movie *out)
{//for debugging tbh
    printf("title = %s\n", out->title);
    printf("year = %d\n", out->year);
    printf("language = %s\n", out->language);
    printf("rating = %f\n", out->rating);
    if (out->nextMovie==NULL){
        printf("nextMovie = NULL\n\n");
    }
    else{
        printf("nextMovie = %s\n\n", out->nextMovie->title);
    }
}

int showMenu()
{
    char *menu;
    printf("\nPlease select the functionality you want to use:\n 1. Show movies released in specified year\n 2. Show highest rated movie in specified year\n 3. Show movies and their year of release in a specified language \n 4. Exit\n");
    int input;
    scanf("%d", &input);
    // printf("%d", input);
    return input;
}

void numberOne(struct movie *data)
{ // will have to change params to use variable arrays
    printf("Enter the year for which you want to see movies: ");
    int userInput;
    scanf("%d", &userInput);
    int counter = 0;
    struct movie *node = data;

    while (node != NULL)
    {
        // printf("%s for consideration\n", node->title);
        if (node->year == userInput)
        {
            printf("%s\n", node->title);
            counter++;
        }
        node = node->nextMovie;
    }
    if (counter == 0)
    {
        printf("No data about movies released in the year %d", userInput);
    }
}

void numberTwo(struct movie *data, int number){// show highest rated movie for each year
    int year[number];
    struct movie *movies[number];
    int list = 0;// number of filled items in year[] and movies[]
    struct movie *node = data;

    //clearing out the data
    for (int i = 0; i<number;i++){
        year[number] = 0;
        movies[number] = NULL;
    }

    
    for (int i = 0; i<number-1;i++){// 
        node = node->nextMovie;
        // printMovie(node);
        //check if it's in array
        int index = -1;
        for(int j=0; j<list;j++){
            if(node->year == year[j]){
                index = j;
                // printf("%d is in array\n", year[j]);
                break;
            }
        }
        //if it's in array
        if (index!=-1){
            
            if((movies[index]->rating)<node->rating){
                movies[index] = node;
            }
            else{
                // printf("%d %s is in array but not highest\n", year[index], node->title);
            }
        }
        //if not,
        else{
            // printf("%d %s new entry\n", node->year, node->title);
            year[list] = node->year;
            movies[list] = node;
            list++;
        }
    }

    //print
    for (int i = 0; i<list;i++){
        printf("%d %f %s\n",year[i], movies[i]->rating, movies[i]->title);
    }
    
}

void numberThree(struct movie *data, int number){ // show movies and their year of release for a specific language
    printf("please enter the language you want to look for\n");
    struct movie *node = data;
    char userInput[20];
    scanf("%s", userInput);
    for (int i = 0; i<number-1; i++){
        node = node->nextMovie;
        // char compare = NULL;
        char *compare = strstr(node->language,userInput);
        if (compare == NULL){
            
        }else{
            printf("%d %s\n",node->year, node->title );
        }
    }
    // strstr()
}

int main(int argc, char *argv[])
{
    // get file name from command line
    char *fileName = argv[1];
    // char *fileName = "movies_sample_1.csv";
    FILE *file_descriptor = fopen(fileName, "r");
    if (file_descriptor < 0)
    {
        fprintf(stderr, "Could not open %s\n", fileName);
        exit(1);
    }

    // variables that store the movies
    int numMovies = 0;

    struct movie *currMovie = NULL;
    struct movie *firstMovie = NULL;

    // getting the lines into the variables

    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;
    char *token;
    //reading lines from input file
    while ((nread = getline(&currLine, &len, file_descriptor)) != -1)
    {
        if (currLine[0] == '\0')
        {
            break;
        }
        struct movie *tempMovie = createMovie(currLine);
        numMovies++;
        if (firstMovie == NULL)
        {
            firstMovie = tempMovie;
            currMovie = tempMovie;
        }
        else
        {
            currMovie->nextMovie = tempMovie;
            currMovie = tempMovie;
        }
    }
    free(currLine);
    fclose(file_descriptor);
    
    //displays all received structs for debugging
    // struct movie *node = firstMovie;
    // while(node !=NULL){
    //     node = node->nextMovie;
    //     printMovie(node);
    // }

    // run program based on menu choice
    int type;
    int loop = 1;
    while (loop == 1)
    {
        type = showMenu();
        switch (type)
        {
        case 1: // show movies released in a specified year
            numberOne(firstMovie);
            break;
        case 2: // show highest rated movie for each year
            numberTwo(firstMovie,numMovies);
            break;
        case 3: // show movies and theyr year of release for a specific language
            numberThree(firstMovie, numMovies);
            break;
        case 4:
            return 1;
            break;
        default:
            printf("you have exited the program");
            exit(1);
        }
    }
}