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
    for(int i=0; i<MAX_SIZE; i++) s[i] = '\0';                                       // Reset the string to null terminators.
}

long long int getResult(int campusId){
    int y = (campusId%((campusId%599)+(campusId%599))/3)+98;

    FILE *f = fopen("math.txt", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);
    // printf("%s\n", string);
    
    int p=0, temp=y;
    while(temp--){
        while(string[p]!=';') p++;
        p++;
    }
    char equation[MAX_SIZE];

    int x=0;
    while(string[p]!=';'){
        equation[x] = string[p];
        x++; p++;
    }

    int num1, num2;
    char op;

    sscanf(equation, "%d %c %d", &num1, &op, &num2);
    printf("\n> The equation corresponding to y = %d is %d%c%d\n", y, num1, op, num2);

    long long result;
    if(op == '+'){
        result = num1+num2;
    }
    if(op == '-'){
        result = num1-num2;
    }
    if(op == '*'){
        result = num1*num2;
    }
    if(op == '/'){
        result = num1/num2;
    }

    return result;
}

void serviceClient(int new_sock){
    char message[MAX_SIZE];                                                        // Buffer used for communication.

    printf("\n> Client's campus id: "); fflush(0);
    resetString(message);                                                      // Reset buffer for new message from client.
    int retVal = read(new_sock, message, sizeof(message)); readError(retVal);  // Get client's requested file.
    printf("%s\n", message);
    long long int campusId = atoi(message);

    long long int result = getResult(campusId);
    resetString(message);
    sprintf(message, "%lld", result);
    retVal = write(new_sock, message, strlen(message)); writeError(retVal);
    
    while(1){
        int flag = 1;
        printf("> Enter name in lowercase : "); fflush(0);
        resetString(message);
        fgets(message, sizeof(message), stdin);
        for(int i=0; i<MAX_SIZE; i++){
            if(message[i] == '\n') message[i] = '\0';
            if(message[i] == '\0') break;
            if(message[i]<'a' || 'z'<message[i]) flag = 0;
        }
        if(flag) break;
    }

    retVal = write(new_sock, message, sizeof(message)); writeError(retVal);

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
    char message[MAX_SIZE];
    fgets(message, sizeof(message), stdin);
    PORT = atoi(message); 

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
    
        serviceClient(new_sock);                                                          // Initiate service with client.
        printf("\n> ======= Waiting for a new client... =======\n\n");
    }

    printf("\n> Closing server...\n");
    shutdown(server_fd, 0);                           // Close server's file descriptor.
    return 0;
}