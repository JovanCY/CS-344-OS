/***************************************************************
 * Filename otp.c
 * Author: Brent Irwin
 * Date: 5 June 2017
 * Description: A main function for otp_enc and otp_dec
***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "otp.h"

void error(const char *msg, int value) { perror(msg); exit(value); } // Error function used for reporting issues

/***************************************************************
 * 			otp(char*, char*, char*, char*)
 *	
 *	Runs the bulk of main for otp_enc.c and otp_dec.c
 *
 *	INPUT:
 *		char* plaintext	The name of the input text file
 *		char* key			The name of the key text file
 *		char* port_arg		The port number as a string
 *		char* enc_dec		Indicates whether this function encrypts or decrypts
***************************************************************/
int otp(char* plaintext, char* key, char* port_arg, char* enc_dec)
{	
	int i, j;
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[512];
    
	// Save port argument to better variable
	int port = atoi(port_arg);

	// Check that the the key is not shorter than the plaintext
	// Get text from plaintext file
	FILE* textFile = fopen(plaintext, "r");
	char text_plaintext[80000];
	fgets(text_plaintext, 80000, textFile);
	fclose(textFile);
	text_plaintext[strcspn(text_plaintext, "\n")] = '\0';

	// Get text from key file
	FILE* keyFile = fopen(key, "r");
	char text_key[80000];
	fgets(text_key, 80000, keyFile);
	fclose(keyFile);
	text_key[strcspn(text_key, "\n")] = '\0';
	
	// Check string length
	int textLength = strlen(text_plaintext);
	int keyLength = strlen(text_key);

	if (keyLength < textLength) {
		fprintf(stderr, "ERROR The key is shorter than the plaintext.\n");
		exit(1);
	}
	
	// Check that the message and key have valid characters only
	// Loop through code to find a match.
	// If j reaches 28, it hasn't found one, and there's an error.
	
	// Plaintext
	for (i=0; i<textLength; i++) {
		for (j=0; j<28; j++) {
			if (j == 27) {
				fprintf(stderr, "ERROR Plaintext contains invalid characters.\n");
				exit(1);
			}
			if (text_plaintext[i] == code[j]) {
				break;
			}
		}
	}

	// Key
	for (i=0; i<keyLength; i++) {
		for (j=0; j<28; j++) {
			if (j == 27) {
				fprintf(stderr, "Key contains invalid characters.\n");
			}
			if (text_key[i] == code[j]) {
				break;
			}
		}
	}

	// Set up the server address struct
	// Clear out the address struct
		memset((char*)&serverAddress, '\0', sizeof(serverAddress));
	// Get the port number, convert to an integer from a string
		portNumber = port;
	// Create a network-capable socket
		serverAddress.sin_family = AF_INET;
	// Store the port number
		serverAddress.sin_port = htons(portNumber);
	// Convert the machine name into a special form of address
		serverHostInfo = gethostbyname("localhost");
		if (serverHostInfo == NULL) {
			fprintf(stderr, "CLIENT: ERROR, no such host\n");
			exit(0);
		}
	// Copy in the address
		memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket", 1);
	
	// Connect to server
	// Connect socket to address
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
		error("CLIENT: ERROR connecting", 1);

	// Splice the message together
	// It will be separated by a newline 
	memset(buffer, '\0', sizeof(buffer));
	sprintf(buffer, "%s\n%s\n%s", plaintext, key, enc_dec);

	// Send the message over
	// Write to the server
	charsWritten = send(socketFD, buffer, strlen(buffer), 0);
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket", 1);
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNIGN: Not all data written to socket!\n");

	// Get return message from server
	// Clear out the buffere agin for reuse
	memset(buffer, '\0', sizeof(buffer));
	// Read data from the socket, leaving \0 at end
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
	if (charsRead < 0) error("CLIENT: ERROR reading from socket", 1);

	// Open file returned and print it out!
	FILE* recvFile = fopen(buffer, "r");
	char output[80000];
	fgets(output, 80000, recvFile);
	fclose(recvFile);

	// Delete the file
	remove(buffer);

	// Print it out and wrap 'er up.
	printf("%s\n", output);
	close(socketFD);
	return 0;
}
