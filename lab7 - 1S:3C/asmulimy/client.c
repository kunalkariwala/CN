#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define buffer_size 1024
// Taken from https://stackoverflow.com/questions/8534274/is-the-strrev-function-not-available-in-linux
char *strrev(char *str)
{
    char *p1, *p2;

    if (!str || !*str)
        return str;
    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
    {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
    return str;
}
int main(int argc, char const *argv[])
{
    int port = atoi(argv[1]);
    char IPaddr[buffer_size];
    int server_fd, new_sock;
    struct sockaddr_in address;
    //this is address to bind socket to


    //creating socket
    //server_fd contains the file descriptor of the socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("Webclient not created successfully");
        return 1;
    }
    //client created successfully

    //this variable stores the size of address
    int addrlen = sizeof(address);

    //Address struct gets info added to it
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(argv[2]);
    //sin_family is always set to AF_INET, sin_port contains the port in network byte order.

    //Incoming connections are accepted via this statement
    if (connect(server_fd, (struct sockaddr *)&address, (socklen_t)sizeof(address)) == -1)
    {
        perror("Error in connection, unable to reach server");
        exit(0);
    }
    //connection has been established at this stage

    char buffer1[buffer_size];  //for received messages
    char buffer2[buffer_size];  //for sending messages
    
    memset(buffer1, '\0', sizeof(buffer1));
    if (read(server_fd, buffer1, sizeof(buffer1)) == -1)
    {
        perror("Error in receiving in client!");
        exit(0);
    }
    printf("S: %s\n", buffer1);
    if (strcmp(buffer1, "rejected!") == 0)
    {
        //Closing connection
        close(server_fd);

        return 0;
    }

    printf("Please enter the string \n");

    int flag = 1;
    //if we reach here it means no error message
    do
    {
        //client sends message
        memset(buffer2, '\0', sizeof(buffer2));
        printf("C: ");
        fgets(buffer2, sizeof(buffer2), stdin);
        for(int i=0; i<1024; i++){if(buffer2[i]=='\n') buffer2[i]='\0';}

        write(server_fd, buffer2, sizeof(buffer2));
        if (strcmp(buffer2, "exit") == 0) //so the input is .
        {
            flag = 0;
        }
        memset(buffer1, '\0', sizeof(buffer1));

        int ret = read(server_fd, buffer1, sizeof(buffer1));
        if (ret == -1)
        {
            perror("Error in receiving in client!");
            exit(0);
        }
        printf("S: %s[Read %d bytes]\n", strrev(buffer1), ret);

    } while (flag);


    //Closing connection
    close(server_fd);

    return 0;
}
