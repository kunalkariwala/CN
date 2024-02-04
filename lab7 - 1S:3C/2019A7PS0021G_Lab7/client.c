#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_SIZE 1024

// Taken from https://stackoverflow.com/questions/8534274/is-the-strrev-function-not-available-in-linux
char *strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

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

int main(int argc, char *argv[]){
    int client_fd;                                                              // Socket file descriptor for client
    struct sockaddr_in address;                                                 // We use AF_INET for IPv6    
    client_fd = socket(AF_INET, SOCK_STREAM, 0);                                // SOCK_STREAM = relaible and connection oriented
    if(client_fd<0){ printf("> Error in opening socket\n"); return 0; }         // Error Checking

    int PORT; 
    char message[MAX_SIZE]; resetString(message);     // Buffer used for communication.
    PORT = atoi(argv[1]);
    address.sin_family = AF_INET;                        // Need to specifiy address
    address.sin_addr.s_addr = inet_addr(argv[2]);        // and port number so that 
    address.sin_port = htons(PORT);                      // it can bind to it.

    resetString(message);                                                       // Reset the buffer.
    
    int retVal = connect(client_fd, (struct sockaddr*)&address, sizeof(address));    // need to bind the client_fd to an address 
    if(retVal<0){                                                                    // and a port as mentioned in
        printf("> Failed connecting to the server\n");                               // struct (sockaddr_in) (4444 in this case).
        return 0;                                                                    // Error checking.
    }

    retVal = read(client_fd, message, sizeof(message)); readError(retVal);           // Read server response that says if client is accepted or rejected.
    int accept = 0;                                                                  // Flag to indicate accept state.
    if(strstr(message, "REJECT")) {                                                  // If server rejects due to too many clients
        printf("> Server rejected due to too many clients\n");                       // then say so.
    }else{
        printf("> Connected to server IP = %s and PORT = %d\n", inet_ntoa(address.sin_addr), PORT); // Or else, print connection.
        accept = 1;                                                                                 // Change accept state to true
    }

    while(accept){
        printf("\n> Enter string : "); fflush(0); resetString(message);
        fgets(message, sizeof(message), stdin);                                           // Put string in buffer.
        for(int i=0; i<MAX_SIZE; i++){if(message[i]=='\n') message[i]='\0';}              // Remove newline from string.
        retVal = write(client_fd, message, strlen(message)); writeError(retVal);          // Send string to server.
        if(!strcmp(message, "exit")) break;
        resetString(message);                                                             // Reset the string.
        int retVal = read(client_fd, message, sizeof(message)); readError(retVal);        // Read server response.
        printf("> Server response : %s\n", strrev(message));                              // Print server response.
    }
    printf("> Closing client...\n");
    close(client_fd);           // Close client's file descriptor.
}
