#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// based on Exploration: Client-Server Communication Via Sockets
// https://canvas.oregonstate.edu/courses/1890464/pages/exploration-client-server-communication-via-sockets?module_item_id=22345212

// Error function used for reporting issues
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in *address,
                        int portNumber)
{

    // Clear out the address struct
    memset((char *)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[])
{
    int childStatus;
    int connectionSocket, charsRead;
    char buffer[70000]; // to hold the original plaintext
    char key[70000];    // the key as string
    char cipher[70000]; // to hold the final ciphertext
    char accMsg[10];
    char message[70000];
    int messageLength =0;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    pid_t pid, sid;

    // Check usage & args
    if (argc < 2)
    {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0)
    {
        error("ERROR opening socket");
    }

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket,
             (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
    {
        error("ERROR on binding");
    }

    // printf("SERVER: enc_server now listening on port: ", atoi(argv[1]));

    // Start listening for connections. Allow up to 5 connections to queue up
    listen(listenSocket, 5);

    // Accept a connection, blocking if one is not available until one connects
    int childCount = 0;
    while (childCount < 5)
    {
        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket,
                                  (struct sockaddr *)&clientAddress,
                                  &sizeOfClientInfo);
        if (connectionSocket < 0)
        {
            error("ERROR on accept");
        }

        childCount++;
        pid = fork(); // so that you can run multiple encryptions at the same time on the same server
        switch (pid)
        {
        case -1:
            error("fork() error\n");
            exit(1);
            break;
        case 0:
            // printf("SERVER: Connected to client running at host %d port %d\n",
            //        ntohs(clientAddress.sin_addr.s_addr),
            //        ntohs(clientAddress.sin_port));

            // get identifying message
            memset(accMsg, '\0', 10);
            recv(connectionSocket, accMsg, 10, 0);

            // if accMsg is not dec_client, that means another app connected    
            if (strstr(accMsg, "dec_client") == 0)
            {
                // printf("SERVER: incorrect client connected");
                charsRead = send(connectionSocket,
                                 "incorrect", 10, 0);
                if (charsRead < 0)
                {
                    error("ERROR writing to socket");
                }
                error("ERROR incorrect program connecting to socket");
            }
            else
            {
                // printf("SERVER: correct client connected");
                charsRead = send(connectionSocket,
                                 "correct!!", 10, 0);
                if (charsRead < 0)
                {
                    error("ERROR writing to socket");
                }
            }

            // get messageLength
            memset(buffer, '\0', 70000);
            // Read the client's message from the socket
            charsRead = recv(connectionSocket, buffer, 70000, 0);
            if (charsRead < 0)
            {
                error("ERROR reading plaintext from socket");
            }
            messageLength = atoi(buffer);


            //get plaintext
            memset(buffer, '\0', 70000);
            char accMsg[20];
            memset(accMsg, '\0', 20);
            while (strlen(buffer)<messageLength)
            {
                // Get the message from the client and display it
                memset(message, '\0', 70000);
                // Read the client's message from the socket
                charsRead = recv(connectionSocket, message, 20000, 0);
                if (charsRead < 0)
                {
                    error("ERROR reading plaintext from socket");
                }
                // printf("SERVER: I received this from the client: \"%s\"\n", message);
                strcat(buffer, message);
                // printf("SERVER: the length of the message is %d charsRead = %d\n", strlen(buffer), charsRead);
                fflush(stdout);


                // send back chars read
                sprintf(accMsg, "%d", strlen(buffer));//copy size of already sent message to accMsg
                charsRead = send(connectionSocket,
                                 accMsg, strlen(accMsg), 0);
                if (charsRead < 0)
                {
                    error("ERROR writing accMsg to socket");
                }
            }
            // Get the key from the client and display it
            memset(key, '\0', 70000);
            while (strlen(key)<messageLength)
            {
                // Get the message from the client and display it
                memset(message, '\0', 70000);
                // Read the client's message from the socket
                charsRead = recv(connectionSocket, message, 20000, 0);
                if (charsRead < 0)
                {
                    error("ERROR reading plaintext from socket");
                }
                // printf("SERVER: I received this from the client: \"%s\"\n", message);
                strcat(key, message);
                // printf("SERVER: the length of the message is %d charsRead = %d\n", strlen(key), charsRead);
                fflush(stdout);


                // send back chars read
                sprintf(accMsg, "%d", strlen(key));//copy size of already sent message to accMsg
                charsRead = send(connectionSocket,
                                 accMsg, strlen(accMsg), 0);
                if (charsRead < 0)
                {
                    error("ERROR writing accMsg to socket");
                }
            }

            // decode the key
            for (int i = 0; i < strlen(buffer); i++)
            {
                int plain;
                int keyInt;
                // get value of message
                if (buffer[i] == ' ')
                {
                    plain = 26;
                }
                else
                {
                    plain = (int)(buffer[i]) - 65; // A will be 0
                }
                // get value of key (key uses [, which is ASCII 91, as space)
                keyInt = (int)(key[i]) - 65;

                // add them up
                int cipherInt = (plain-keyInt) % 27;
                if (cipherInt<0){// if it goes below 0, it loops around to the top,space = 26
                    cipherInt +=27;
                }

                // convert the new value to ciphertext
                if (cipherInt == 26)
                {
                    cipher[i] = ' ';
                }
                else
                {
                    cipher[i] = (char)(cipherInt + 65);
                }
            }
            cipher[strlen(buffer)] = '\n';
            cipher[strlen(buffer)+1] = '\0';

            // printf("SERVER: This is what I'm sending as ciphertext: %s", cipher);

            // Send a Success message back to the client
            charsRead = send(connectionSocket,
                             cipher, strlen(key), 0);
            if (charsRead < 0)
            {
                error("ERROR writing to socket");
            }

        default:;
            // pid = waitpid(pid, &childStatus, 0);
            // printf("PARENT(%d): child(%d) terminated. Exiting\n", getpid(), spawnPid);
            // wait(NULL);
        }
        // Close the connection socket for this client
        close(connectionSocket);
        childCount--;
    }
    // Close the listening socket
    close(listenSocket);
    return 0;
}