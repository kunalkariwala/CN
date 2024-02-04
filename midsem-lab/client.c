#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

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
    for(int i=0; i<MAX_SIZE; i++) s[i] = '\0';                                       // Reset the string to null terminators.
}

int main(){
    int client_fd;                                                              // Socket file descriptor for client
    struct sockaddr_in address;                                                 // We use AF_INET for IPv6    
    client_fd = socket(AF_INET, SOCK_STREAM, 0);                                // SOCK_STREAM = relaible and connection oriented
    if(client_fd<0){ printf("> Error in opening socket\n"); return 0; }         // Error Checking

    int PORT; 
    char message[MAX_SIZE]; resetString(message);     // Buffer used for communication.

    printf("> Enter port number : "); fflush(0);
    fgets(message, sizeof(message), stdin);
    PORT = atoi(message);
    printf("> Enter server address : "); fflush(0);
    fgets(message, sizeof(message), stdin);

    address.sin_family = AF_INET;                        // Need to specifiy address
    address.sin_addr.s_addr = inet_addr(message);        // and port number so that 
    address.sin_port = htons(PORT);                      // it can bind to it.

    resetString(message);                                                       // Reset the buffer.
    
    int retVal = connect(client_fd, (struct sockaddr*)&address, sizeof(address));    // need to bind the client_fd to an address 
    if(retVal<0){                                                                    // and a port as mentioned in
        printf("> Failed connecting to the server\n");                               // struct (sockaddr_in) (4444 in this case).
        return 0;                                                                    // Error checking.
    }

    printf("> Connected to server IP = %s and PORT = %d\n", inet_ntoa(address.sin_addr), PORT);

    printf("\n> Enter campus id : "); fflush(0);
    char id[MAX_SIZE]; resetString(id);
    fgets(id, sizeof(id), stdin);                                           // Put name in buffer.
    for(int i=0; i<100; i++){if(id[i]=='\n') id[i]='\0';}                   // Remove newline from name.
    retVal = write(client_fd, id, strlen(id)); writeError(retVal);          // Send name to server.

    char y[MAX_SIZE];
    retVal = read(client_fd, y, sizeof(y)); readError(retVal);      // Read the result
    printf("\n> Server response: %s\n", y);

    char name[MAX_SIZE];
    retVal = read(client_fd, name, sizeof(name)); readError(retVal);
    printf("> Server response: %s\n", name); 

    printf("\n> %s %s %s\n\n", name, id, y);

    printf("> Closing client...\n");
    close(client_fd);           // Close client's file descriptor.
}
