/***************************************************************
 * Filename: keygen.c
 * Author: Brent Irwin
 * Date: 5 June 2017
 * Description: Creates a key file of specified length
***************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "otp.h"

int main (int argc, char* argv[]) {
	
	int i;
	int length = atoi(argv[1]);
	char key[length+1];
	srand(time(0));

	// For each char in key, assign a random value
	for (i=0; i<length; i++) {
		int letter = rand() % 27;
		key[i] = code[letter];
	}

	// Without extending length 1 longer, my keys kept having "cs="
	// appended to them for some reason, so I'll use \0 to cut them off.
	key[length] = '\0';
	
	printf("%s\n", key);

	return 0;
}
