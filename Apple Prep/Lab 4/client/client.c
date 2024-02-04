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
    for(int i=0; i<100; i++) s[i] = '\0';                                       // Reset the string to null terminators.
}

int main(int argc, char *argv[]){
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

    int retVal = connect(client_fd, (struct sockaddr*)&address, sizeof(address));    // need to bind the client_fd to an address 
    if(retVal<0){                                                                    // and a port as mentioned in
        printf("> Failed connecting to the server\n");                               // struct (sockaddr_in) (4444 in this case).
        return 0;                                                                    // Error checking.
    }

    printf("> Connected to server IP = %s and PORT = %d\n", inet_ntoa(address.sin_addr), PORT);

    printf("\n> Enter file name: "); fflush(0);
    char name[MAX_SIZE]; resetString(name);
    fgets(name, sizeof(name), stdin);                                           // Put name in buffer.
    for(int i=0; i<100; i++){if(name[i]=='\n') name[i]='\0';}                   // Remove newline from name.

    retVal = write(client_fd, name, strlen(name)); writeError(retVal);          // Send name to server.

    resetString(message);                                                       // Reset the buffer.
    retVal = read(client_fd, message, sizeof(message)); readError(retVal);      // Read the bytes sent by the server.
    
    printf("> Server response: %s\n", message); 

    printf("\n> Creating file : %s\n\n", name);
    FILE *fdesc = fopen(name, "w");
    retVal = fputs(message, fdesc); if(retVal<0) printf("> Error creating and writing to file\n");

    printf("> Closing client...\n");
    close(client_fd);           // Close client's file descriptor.
}
