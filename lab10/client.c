#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/select.h>

#define MAX_SIZE 1024

fd_set s_rd, s_wr, s_ex;

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

void *listenToServer(void *args){
    int client_fd = (int) args;
    char message[MAX_SIZE];
    while(1){
        resetString(message);
        int retVal = read(client_fd, message, sizeof(message)); readError(retVal);
        printf("\33[2K\r");
        printf("%s> Enter string : ", message); fflush(0);
    }
}

int end = 0;
void *tellToServer(void *args){
    int client_fd = (int) args;
    char message[MAX_SIZE];
    while(1){
        printf("> Enter string : "); fflush(0); resetString(message);
        select(fileno(stdin)+1, &s_rd, &s_wr, &s_ex, NULL);
        fgets(message, sizeof(message), stdin);                                               // Put string in buffer.
        printf("\033[A");
        printf("\33[2K\r");
        for(int i=0; i<MAX_SIZE; i++){if(message[i]=='\n') message[i]='\0';}                  // Remove newline from string.
        int retVal = write(client_fd, message, strlen(message)); writeError(retVal);          // Send string to server.
        if(!strcmp(message, "EXIT")) break;
    }
    end = 1;
}

int main(int argc, char *argv[]){

    FD_ZERO(&s_rd);
    FD_ZERO(&s_wr);
    FD_ZERO(&s_ex);
    FD_SET(fileno(stdin), &s_rd);

    int client_fd;                                                              // Socket file descriptor for client
    struct sockaddr_in address;                                                 // We use AF_INET for IPv6    
    client_fd = socket(AF_INET, SOCK_STREAM, 0);                                // SOCK_STREAM = relaible and connection oriented
    if(client_fd<0){ printf("> Error in opening socket\n"); return 0; }         // Error Checking

    int PORT; 
    char message[MAX_SIZE]; resetString(message);     // Buffer used for communication.
    PORT = 4444; //atoi(argv[1]);
    address.sin_family = AF_INET;                        // Need to specifiy address
    address.sin_addr.s_addr = inet_addr("10.20.9.99"); //inet_addr(argv[2]);        // and port number so that 
    address.sin_port = htons(PORT);                      // it can bind to it.

    resetString(message);                                                       // Reset the buffer.
    
    for(int i=0; i<MAX_SIZE; i++){if(message[i]=='\n') message[i]='\0';}

    int retVal = connect(client_fd, (struct sockaddr*)&address, sizeof(address));    // need to bind the client_fd to an address 
    if(retVal<0){                                                                    // and a port as mentioned in
        printf("> Failed connecting to the server\n");                               // struct (sockaddr_in) (4444 in this case).
        return 0;                                                                    // Error checking.
    }

    retVal = read(client_fd, message, sizeof(message)); readError(retVal);

    printf("> Connected to server IP = %s and PORT = %d\n", inet_ntoa(address.sin_addr), PORT); // Or else, print connection.
    
    pthread_t responseThread;
    pthread_create(&responseThread, NULL, listenToServer, (void *) client_fd);

    pthread_t tellThread;
    pthread_create(&tellThread, NULL, tellToServer, (void *) client_fd);
    
    while(!end);
    printf("> Closing client...\n");
    close(client_fd);           // Close client's file descriptor.
}
