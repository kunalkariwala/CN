#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PORT 4444
#define OK_STATUS "HTTP/1.1 200 OK\n"
#define ERROR_STATUS "HTTP/1.1 500 ERROR\n"

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

void chatWithClient(char *name, int new_sock){
    char message[100];                                                             // Buffer used for communication.
    while(1){                                                                      // Keep reading client's messages.
        printf("> Client's message: "); fflush(0);
        resetString(message);                                                      // Reset buffer for new message from client.
        int retVal = read(new_sock, message, sizeof(message)); readError(retVal);  // Get client's message.
        printf("%s", message);

        if(message[0]=='.' && (message[1]=='\n' || message[1]=='\0')){             // If message == "." , close connection.
            resetString(message);                                                  // Reset buffer for
            sprintf(message, "Thank you %s\n", name);                              // Write thank you message in buffer.
            retVal = write(new_sock, message, sizeof(message));writeError(retVal); // Send thank you message to client.
            break;                                                                 // Stop chatting with the client.
        }
    }

    printf("\n> Closing connection...\n");
    close(new_sock);                                                               // Close client connection.
}

int main(){

    int server_fd, new_sock;                                                    // Socket file descriptor for server.
    struct sockaddr_in address;                                                 // We use AF_INET for IPv6.
    server_fd = socket(AF_INET, SOCK_STREAM, 0);                                // SOCK_STREAM = relaible and connection oriented.
    if(server_fd<0){ printf("> Error in opening socket\n"); return 0; }         // Error Checking.

    address.sin_family = AF_INET;                // Need to specifiy address
    address.sin_addr.s_addr = INADDR_ANY;        // and port number so that 
    address.sin_port = htons(PORT);              // it can bind to it.

    int retVal = bind(server_fd, (struct sockaddr *) &address, sizeof(address));    // Need to bind the server_fd to an address 
    if(retVal<0){                                                                   // and a port as mentioned in
        printf("> Cannot bind %d\n", retVal);    //Error checking.                  // struct (sockaddr_in) (4444 in this case).
        return 0;
    }

    
    listen(server_fd, 1);               // This is needed so that the socket waits for client to approach. The number 1 is the maximum queue size.  

    while(1){
        int addrlen = sizeof(address);                                                    // Had to initialize this variable in order to pass it as a pointer in a later function.
        new_sock = accept(server_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen); // It accepts the first connection request from the listening socket queue 
        printf("> New connection\n");                                                     // and creates a new connected socket and returns a file descriptor.
        if(new_sock == -1){ printf("> Cannot accept socket\n"); return 0;}                // Error checking.


        char *hello = "Hello, what is your name?\n"; fflush(0);                           // Set the hello message.
        retVal = write(new_sock , hello , strlen(hello)); writeError(retVal);             // Send the hello message.
        printf("> Hello message sent to client\n");
        
        char name[100]; resetString(name);                                                // Name string to store the name.
        printf("> Name recieved from client: "); fflush(0);
        retVal = read(new_sock, name, sizeof(name)); readError(retVal);                   // Get name from client.
        printf("%s\n", name);

        printf("\n> Status sent to client : "); fflush(0);
        if('A'<=name[0] && name[0]<='Z'){
            retVal = write(new_sock, OK_STATUS, sizeof(OK_STATUS)); writeError(retVal);   // Send OK to client.
            printf(OK_STATUS); printf("\n");
        }else{
            retVal = write(new_sock, ERROR_STATUS, sizeof(ERROR_STATUS)); writeError(retVal); // Send ERROR to client.
            printf(ERROR_STATUS); printf("\n");
        }

        chatWithClient(name, new_sock);         // Initiate chat with client.
        break;                                  // Break since the lab specified server can only accept one client and no further.
    }

    printf("\n> Closing server...\n");
    close(server_fd);                           // Close server's file descriptor.
    return 0;
}
