#include<sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include<stdio.h>
#include <unistd.h>

#define PORT 4444


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

    char message[100]; resetString(message);

    retVal = read(client_fd, message, sizeof(message)); readError(retVal);
    printf("> Server response: %s", message);
    
    printf("> Enter your name: ");
    char name[100]; resetString(name);
    fgets(name, sizeof(name), stdin);
    for(int i=0; i<100; i++){if(name[i]=='\n') name[i]='\0';}

    retVal = write(client_fd, name, strlen(name)); writeError(retVal);
    resetString(message);
    retVal = read(client_fd, message, sizeof(message)); readError(retVal);
    
    printf("\n");
    printf("> Server response: %s\n", message);

    if(strstr(message, "200")){
        while(1){
            resetString(message);
            printf("> Send a message : ");
            fgets(message, sizeof(message), stdin);
            retVal = write(client_fd, message, strlen(message)); writeError(retVal);
            
            if(message[0] == '.' || message[0] == '!'){
                for(int i=0; i<100; i++) message[i] = '\0';
                retVal = read(client_fd, message, sizeof(message)); readError(retVal);
                printf("\n> Server response: %s\n", message);
                break;
            }
        }
    }else{
        printf("> Server refused to connect\n");
    }

    printf("> Closing client...\n");
    close(client_fd);
}
