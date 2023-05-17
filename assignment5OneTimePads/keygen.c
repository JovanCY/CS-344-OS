#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]){
    int keyLength = atoi(argv[1]);
    // int keyLength = 1000;
    const char ALPHABET[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    char key[keyLength+1];
    srand(time(0));

    for (int i =0; i<keyLength; i++){
        int num = rand() %27;
        // key[i] = ALPHABET[num];
        
        key[i] = (char)(num+65);
    }
    key[keyLength] = '\n';

    printf("%s", key);
    return 0;
}