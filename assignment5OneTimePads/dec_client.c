#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

// based on Exploration: Client-Server Communication Via Sockets
// https://canvas.oregonstate.edu/courses/1890464/pages/exploration-client-server-communication-via-sockets?module_item_id=22345212
/**
 * Client code
 * 1. Create a socket and connect to the server specified in the command arugments.
 * 2. Prompt the user for input and send that input as a message to the server.
 * 3. Print the message received from the server and exit the program.
 */

// Error function used for reporting issues
void error(const char *msg)
{
    // perror(msg);
    fprintf(stderr, "%s", msg);
    exit(0);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in *address,
                        int portNumber,
                        char *hostname)
{

    // Clear out the address struct
    memset((char *)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent *hostInfo = gethostbyname(hostname);
    if (hostInfo == NULL)
    {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char *)&address->sin_addr.s_addr,
           hostInfo->h_addr_list[0],
           hostInfo->h_length);
}

int main(int argc, char *argv[])
{ // call syntax: enc_client plaintext key port
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char buffer[70000];
    char key [70000];
    char message[70000];
    char messageLength[10];
    char accMsg[20];
    int messageSize;

    char *plaintextFile = argv[1];
    char *keyFile = argv[2];

    // Check usage & args
    if (argc < 4)
    {
        fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]);
        exit(0);
    }

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0)
    {
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    // Connect to server
    if (connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        error("CLIENT: ERROR connecting");
    }

    // send out identifying message
    charsWritten = send(socketFD, "dec_client", 10, 0);
    if (charsWritten < 0)
    {
        error("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(buffer))
    {
        printf("CLIENT: WARNING: Not all data written to socket!\n");
    }
    //receive the return message
    memset(buffer, '\0', sizeof(buffer));
    charsRead = recv(socketFD, buffer, 10, 0);
    if (charsRead < 0)
    {
        error("CLIENT: ERROR reading from socket");
    }

    //if server sends back an incorrect 
    if (strstr(buffer, "incorrect") != 0){
        error("incorrect app connecting to server");
    }
    
    // Clear out the buffer array
    memset(buffer, '\0', sizeof(buffer));
    // Get input from the plaintext file
    FILE *plaintextInput = fopen(plaintextFile, "r");
    fgets(buffer, 70000, plaintextInput);
    fclose(plaintextInput);
    // Remove the trailing \n that fgets adds
    buffer[strcspn(buffer, "\n")] = '\0';

    //get key 
    memset(key, '\0', sizeof(key));
    // Get input from the key file
    FILE *keyInput = fopen(keyFile, "r");
    fgets(key, 70000, keyInput);
    fclose(keyInput);
    // Remove the trailing \n that fgets adds
    key[strcspn(key, "\n")] = '\0';

    //error handling if the key and message size don't match
    if (strlen(buffer) > strlen(key)){
        error("CLIENT: ERROR key is not appropriate for message size");
    }

    // send message length
    messageSize = strlen(buffer);
    sprintf(messageLength, "%d", messageSize);
    charsWritten = send(socketFD, messageLength, strlen(messageLength), 0);
    if (charsWritten < 0)
    {
        error("CLIENT: ERROR writing message to socket");
    }
    if (charsWritten < strlen(messageLength))
    {
        printf("CLIENT: WARNING: Not all data written to socket!\n");
    }
    // printf("\nCLIENT: the message to be sent to has length %d\n", messageSize);
    // fflush(stdout);

    //*******************************************************************************
    //*******************************************************************************
    // Send message to server
    // Write to the server
    charsWritten = 0;
    int ack = 0;// holds ack number
    while (ack< messageSize)
    {// basically some RDT methods from computer networks
    // sends data in chunks and waits for ACK from server
        memset(message, '\0', 70000);
        for (int i = 0; i<20000; i++){
            message[i] = buffer[ack];
            if (buffer[ack] == '\0')
            {
                break;
            }
            ack++;
        }
        // printf("CLIENT: message to be sent to server: %s\n\n", message);
        fflush(stdout);
        charsWritten = send(socketFD, message, strlen(message), 0);
        if (charsWritten < 0)
        {
            error("CLIENT: ERROR writing message to socket");
        }
        if (charsWritten < strlen(message))
        {
            printf("CLIENT: chars sent = %d\n", charsWritten);
            printf("CLIENT: WARNING: Not all data written to socket!\n");
        }

        //receive ACK
        memset(accMsg, '\0', sizeof(accMsg));
        charsRead = recv(socketFD, accMsg, 20, 0);
        if (charsRead < 0)
        {
            error("CLIENT: ERROR reading from socket");
        }
        // printf("\nCLIENT: received ACK %s\n", accMsg);
        ack = atoi(accMsg);
    }


     // send key
    ack = 0;
    while (ack < messageSize)
    {   // basically some RDT methods from computer networks
        // sends data in chunks and waits for ACK from server
        memset(message, '\0', 70000);
        for (int i = 0; i < 20000; i++)
        {
            // message copies key from what the server last received
            message[i] = key[ack];
            if (key[ack] == '\0')
            {
                break;
            }
            ack++;
        }
        // printf("CLIENT: message to be sent to server: %s\n\n", message);
        fflush(stdout);
        charsWritten = send(socketFD, message, strlen(message), 0);
        if (charsWritten < 0)
        {
            error("CLIENT: ERROR writing message to socket");
        }
        if (charsWritten < strlen(message))
        {
            printf("CLIENT: chars sent = %d\n", charsWritten);
            printf("CLIENT: WARNING: Not all data written to socket!\n");
        }

        // receive ACK
        memset(accMsg, '\0', sizeof(accMsg));
        charsRead = recv(socketFD, accMsg, 20, 0);
        if (charsRead < 0)
        {
            error("CLIENT: ERROR reading from socket");
        }
        // printf("\nCLIENT: received ACK %s\n", accMsg);
        ack = atoi(accMsg);
    }

    // Get return message from server
    // Clear out the buffer again for reuse
    memset(buffer, '\0', sizeof(buffer));
    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, buffer, 70000, 0);
    if (charsRead < 0)
    {
        error("CLIENT: ERROR reading from socket");
    }
    printf("%s", buffer);

    // Close the socket
    close(socketFD);
    return 0;
}