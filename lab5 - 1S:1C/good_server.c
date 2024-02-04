#include<sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include<stdio.h>
#include <unistd.h>

#define PORT 4444
#define OK_STATUS "HTTP/1.1 200 OK\n"
#define ERROR_STATUS "HTTP/1.1 500 ERROR\n"

void readError(int retVal){
    if(retVal < 0){
        printf("> Error in reading message, return value = %d\n", retVal);
        exit(0);
    }
}

void writeError(int retVal){
    if(retVal < 0){
        printf("> Error in writing message, return value = %d\n", retVal);
        exit(0);
    }
}

void resetString(char *s){
    for(int i=0; i<100; i++) s[i] = '\0';
}

int chatWithClient(char *name, int new_sock){
    char message[100];
    int flag = 0;
    while(1){
        printf("> Client's message: "); fflush(0);
        resetString(message);
        int retVal = read(new_sock, message, sizeof(message)); readError(retVal);
        printf("%s", message);

        if(message[0]=='.'){
            char s[100]; resetString(s);
            sprintf(s, "Thank you %s\n", name);
            retVal = write(new_sock,s, sizeof(s)); writeError(retVal);
            break;
        }

        if(message[0]=='!'){
            char s[100]; resetString(s);
            sprintf(s, "Thank you %s\n", name);
            retVal = write(new_sock,s, sizeof(s)); writeError(retVal);
            flag = 1;
            break;
        }
    }

    printf("\n> Closing connection...\n");
    close(new_sock);
    
    return flag;
}

int main(){

    int server_fd, new_sock;                                                    // Socket file descriptor for server
    struct sockaddr_in address;                                                 // We use AF_INET for IPv6    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);                                // SOCK_STREAM = relaible and connection oriented
    if(server_fd<0){ printf("> Error in opening socket\n"); return 0; }         // Error Checking

    address.sin_family = AF_INET;                // Need to specifiy address
    address.sin_addr.s_addr = INADDR_ANY;        // and port number so that 
    address.sin_port = htons(PORT);              // it can bind to it.

    int retVal = bind(server_fd, (struct sockaddr *) &address, sizeof(address));    // need to bind the server_fd to an address 
    if(retVal<0){                                                                   // and a port as mentioned in
        printf("> Cannot bind %d\n", retVal);    //Error checking.                  // struct (sockaddr_in) (4444 in this case).
        return 0;
    }

    
    listen(server_fd, 1);               // this is needed so that the socket waits for client to approach. The number 7 is the maximum queue size.
    // check if 1 is working!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    while(1){
        int addrlen = sizeof(address);                                                    // had to initialize this variable in order to pass it as a pointer in a later function.
        new_sock = accept(server_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen); // it accepts the first connection request from the listening socket queue 
        printf("> New connection\n");                                                     // and creates a new connected socket and returns a file descriptor.
        if(new_sock == -1){ printf("> Cannot accept socket\n"); return 0;}                // Error checking.

        char *hello = "Hello, what is your name?\n"; fflush(0);
        retVal = write(new_sock , hello , strlen(hello)); writeError(retVal);
        printf("> Hello message sent to client\n");
        
        char name[100]; resetString(name);
        printf("> Name recieved from client: "); fflush(0);
        retVal = read(new_sock, name, sizeof(name)); readError(retVal);
        printf("%s\n", name);

        int flag = 0;

        printf("\n> Status sent to client : "); fflush(0);
        if('A'<=name[0] && name[0]<='Z'){
            retVal = write(new_sock, OK_STATUS, sizeof(OK_STATUS)); writeError(retVal);
            printf(OK_STATUS); printf("\n");
            if(chatWithClient(name, new_sock)) break;
        }else{
            retVal = write(new_sock, ERROR_STATUS, sizeof(ERROR_STATUS));  writeError(retVal);
            printf(ERROR_STATUS); printf("\n");
            printf("> Bad Client\n");
        }

        printf("\n> ======= Waiting for a new client... =======\n\n");
    }

    printf("\n");
    printf("> Closing server...\n");
    close(server_fd);
    return 0;
}
