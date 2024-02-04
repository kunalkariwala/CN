#include<sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include<stdio.h>

#define PORT 4444

int main(int argc, char const *argv[]){
    int server_fd, new_sock;
    struct sockaddr_in address;

    char *hello = "'HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nHello World";

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(server_fd<0){
        printf("Error in opening socket\n");
        return 0;
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(server_fd, (struct sockaddr *) &address, sizeof(address)) == -1){
        printf("Cannot bind\n");
        return 0;
    }

    listen(server_fd, 7);

    int addrlen = sizeof(address); 
    new_sock = accept(server_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen);
    
    if(new_sock == -1){
        printf("Cannot accept\n");
        return 0;
    }

    send(new_sock, hello, strlen(hello), 0);
    printf("Hello message sent to browser\n");
    return 0;
}
