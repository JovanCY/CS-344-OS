/***************************************************************
 * Filename: otp_d.c
 * Author: Brent Irwin
 * Date: 5 June 2017
 * Description: A main funciton for otp_enc_d and otp_dec_d
 ***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "otp.h"

// Error function used for reporting issues
void error(const char *msg)
{
	perror(msg);
	exit(1);
}

/***************************************************************
 *				otp_d(char*, char*)
 *
 *	Runs the bulk of main for otp_enc_d.c and otp_dec_d.c
 *
 *	INPUT:
 *		char* port_arg		The port number as a string
 *		char* enc_dec		Indicates whether this function encrypts or decrypts
 *
 ***************************************************************/
int otp_d(char *port_arg, char *enc_dec)
{
	int wrongFile = 0;
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, status;
	socklen_t sizeOfClientInfo;
	char buffer[512];
	char inputFileName[256];
	char keyFileName[256];
	char inputText[80000];
	char key[80000];
	char outputText[80000];
	struct sockaddr_in serverAddress, clientAddress;
	pid_t pid, sid;

	// Set up the address struct for this process (the server)
	// Clear out the address struct
	memset((char *)&serverAddress, '\0', sizeof(serverAddress));
	// Get the port number, convert to an integer from a string
	portNumber = atoi(port_arg);
	// Create a network-capable socket
	serverAddress.sin_family = AF_INET;
	// Store the port number
	serverAddress.sin_port = htons(portNumber);
	// Any address is allowed for connection to this process
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0)
		error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	while (1)
	{
		// Get the size of teh address for the client that will connect
		sizeOfClientInfo = sizeof(clientAddress);
		// Accept
		//		establishedConnectionFD = accept(listenSocketFD, NULL, NULL);
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
		if (establishedConnectionFD < 0)
			error("ERROR on accept");

		// Time to fork
		pid = fork();
		switch (pid)
		{

		// Error
		case -1:;
			error("ERROR creating fork");
			exit(0);
			break;

		// Success
		case 0:;
			memset(buffer, '\0', sizeof(buffer));
			// Read the client's message from the socket
			if (charsRead < 0)
				error("ERROR reading from socket");

			// Convert buffer into message and key
			const char newline[2] = {'\n', '\0'};

			// Plaintext
			char *token = strtok(buffer, newline);
			strcpy(inputFileName, token);

			// Key
			token = strtok(NULL, newline);
			strcpy(keyFileName, token);

			// Make sure the right program is connecting
			token = strtok(NULL, newline);
			if (strcmp(enc_dec, token))
			{
				fprintf(stderr, "ERROR %s cannot use %s_d.\n", token, enc_dec);
				wrongFile = 1;
			}

			if (!wrongFile)
			{
				// Open the input and key and put them into strings
				// Remove newlines at end
				FILE *inputFile = fopen(inputFileName, "r");
				fgets(inputText, 80000, inputFile);
				fclose(inputFile);
				inputText[strcspn(inputText, "\n")] = '\0';

				FILE *keyFile = fopen(keyFileName, "r");
				fgets(key, 80000, keyFile);
				fclose(keyFile);
				key[strcspn(key, "\n")] = '\0';

				// Encode or decode that shit
				if (!strcmp(enc_dec, "otp_enc"))
				{
					encode(key, inputText, outputText, strlen(inputText));
				}
				else if (!strcmp(enc_dec, "otp_dec"))
				{
					decode(key, inputText, outputText, strlen(inputText));
				}
			}
			// Write outputText to unique file
			// Create a unique file name
			int uniquePid = getpid();
			char uniqueFile[32];
			sprintf(uniqueFile, "%s_f.%d", enc_dec, uniquePid);

			// Open it
			// Write the output text to it
			// Write nothing if it is the wrong program connecting
			FILE *uniqueFD = fopen(uniqueFile, "w+");
			if (wrongFile)
			{
				fprintf(uniqueFD, "");
			}
			else
			{
				fprintf(uniqueFD, "%s", outputText);
			}
			fclose(uniqueFD);

			// Send it!
			charsRead = send(establishedConnectionFD, uniqueFile, strlen(uniqueFile), 0);
			if (charsRead < 0)
				error("ERROR writing to socket");

			// Close the connection
			close(establishedConnectionFD);
			establishedConnectionFD = -1;
			exit(0);

			break;

		default:;
		}
		// wait for zombie programs and kill them
		close(establishedConnectionFD);
		establishedConnectionFD = -1;
		wait(NULL);
	}

	// We're all done here!
	close(listenSocketFD);
	return 0;
}
