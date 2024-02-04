#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PORT 4444

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

int main(){
    int client_fd;                                                              // Socket file descriptor for client
    struct sockaddr_in address;                                                 // We use AF_INET for IPv6    
    client_fd = socket(AF_INET, SOCK_STREAM, 0);                                // SOCK_STREAM = relaible and connection oriented
    if(client_fd<0){ printf("> Error in opening socket\n"); return 0; }         // Error Checking

    address.sin_family = AF_INET;                // Need to specifiy address
    address.sin_addr.s_addr = INADDR_ANY;        // and port number so that 
    address.sin_port = htons(PORT);              // it can bind to it.

    int retVal = connect(client_fd, (struct sockaddr*)&address, sizeof(address));    // need to bind the client_fd to an address 
    if(retVal<0){                                                                    // and a port as mentioned in
        printf("> Failed connecting to the server\n");                               // struct (sockaddr_in) (4444 in this case).
        return 0;                                                                    // Error checking.
    }

    printf("> Connected to server\n");

    char message[100]; resetString(message);            // Buffer used for communication.

    retVal = read(client_fd, message, sizeof(message)); readError(retVal);      // Get the Hello Response from the server.
    printf("> Server response: %s", message); fflush(0);
    resetString(message);                                                       // Reset the buffer.
    printf("> Enter your name: "); fflush(0);
    fgets(message, sizeof(message), stdin);                                     // Put name in buffer.
    for(int i=0; i<100; i++){if(message[i]=='\n') message[i]='\0';}             // Remove newline from name.

    retVal = write(client_fd, message, strlen(message)); writeError(retVal);    // Send name to server.
    resetString(message);                                                       // Reset the buffer.
    retVal = read(client_fd, message, sizeof(message)); readError(retVal);      // Read the status code sent by the server.
    
    printf("\n");
    printf("> Server response: %s\n", message); 

    if(strstr(message, "500")){
        printf("> Server responded with 500 ERROR code\n> Terminating connection...\n\n");  // If status code is 500, terminate connection.
        retVal = write(client_fd, ".", sizeof(".")); writeError(retVal);                    // Send termination signal to server.
    }else{
        while(1){                                                                           // If status code is 200:
            resetString(message);                                                           // Reset buffer for new message.
            printf("> Send a message : ");
            fgets(message, sizeof(message), stdin);                                         // Get message from user
            retVal = write(client_fd, message, strlen(message)); writeError(retVal);        // Send message to server
            
            if(message[0] == '.' && message[1]=='\n'){                                      // If user enters '.', terminate connection.
                resetString(message);                                                       // Reset buffer to read server's response
                retVal = read(client_fd, message, sizeof(message)); readError(retVal);      // Read server's goodbye message.
                printf("\n> Server response: %s\n", message);
                break;
            }
        }
    }

    printf("> Closing client...\n");
    close(client_fd);           // Close client's file descriptor.
}
