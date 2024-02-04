#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define OK_STATUS "HTTP/1.1 200 OK\n"
#define ERROR_STATUS "HTTP/1.1 500 ERROR\n"
#define MAX_SIZE 1024

void readError(int retVal){
    if(retVal < 0){
        printf("> Error in reading message, return value = %d\n", retVal);     // Helper function to check errors.
        exit(0);
    }
}

void writeError(int retVal){
    if(retVal < 0){
        printf("> Error in writing message, return value = %d\n", retVal);     // Helper function to check errors.
        exit(0);
    }
}

void resetString(char *s){
    for(int i=0; i<100; i++) s[i] = '\0';                                       // Reset the string to null terminators.
}

void serviceClient(int numberOfBytes, int new_sock){
    char message[MAX_SIZE];                                                        // Buffer used for communication.

    printf("\n> Client's requested file: "); fflush(0);
    resetString(message);                                                      // Reset buffer for new message from client.
    int retVal = read(new_sock, message, sizeof(message)); readError(retVal);  // Get client's requested file.
    printf("%s\n", message);

    FILE *fdesc = fopen(message, "r");

    resetString(message);        
    if(fdesc == NULL){
        int retVal = write(new_sock, message, sizeof(message)); writeError(retVal);
    }else{
        fgets(message, numberOfBytes+1, fdesc);
        fclose(fdesc);
        int retVal = write(new_sock, message, sizeof(message)); writeError(retVal);
    }
    
    printf("> Data sent to the client : %s\n", message);

    printf("\n> Closing connection...\n");
    close(new_sock);                                                               // Close client connection.
}

int main(){

    int server_fd, new_sock;                                                    // Socket file descriptor for server.
    struct sockaddr_in address;                                                 // We use AF_INET for IPv6.
    server_fd = socket(AF_INET, SOCK_STREAM, 0);                                // SOCK_STREAM = relaible and connection oriented.
    if(server_fd<0){ printf("> Error in opening socket\n"); return 0; }         // Error Checking.

    int PORT;
    printf("> Enter port number : "); fflush(0);
    scanf("%d", &PORT);

    address.sin_family = AF_INET;                       // Need to specifiy address
    address.sin_addr.s_addr = inet_addr("127.0.0.1");   // and port number so that 
    address.sin_port = htons(PORT);                     // it can bind to it.

    int retVal = bind(server_fd, (struct sockaddr *) &address, sizeof(address));    // Need to bind the server_fd to an address 
    if(retVal<0){                                                                   // and a port as mentioned in
        printf("> Cannot bind %d, socket probably in TIME_WAIT state, please try again later\n", retVal);    //Error checking.
        return 0;
    }

    printf("> Server bound to IP : %s and PORT : %d\n", inet_ntoa(address.sin_addr), PORT);
    
    listen(server_fd, 1);               // This is needed so that the socket waits for client to approach. The number 1 is the maximum queue size.  

    while(1){
        int addrlen = sizeof(address);                                                    // Had to initialize this variable in order to pass it as a pointer in a later function.
        new_sock = accept(server_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen); // It accepts the first connection request from the listening socket queue 
        printf("> New connection\n");                                                     // and creates a new connected socket and returns a file descriptor.
        if(new_sock == -1){ printf("> Cannot accept socket\n"); return 0;}                // Error checking.
    
        serviceClient(10, new_sock);                // Initiate service with client.
        break;                                  // Break since the lab specified server can only accept one client and no further.
    }

    printf("\n> Closing server...\n");
    shutdown(server_fd, 0);                           // Close server's file descriptor.
    return 0;
}