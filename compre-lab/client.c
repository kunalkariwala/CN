#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_L 1024
int sock1,ret;
char buffer[MAX_L];
int quit = 0;


struct headerTemp {
    int found;
    long bytes;
};

void handle_signal_sigint(int sig){
    bzero(buffer,sizeof(buffer));
    strcpy(buffer, "quit");
    send(sock1,"quit",sizeof("quit"),0);
    quit = 1;
}

int main(int argc, char const* argv[]){
  signal(SIGINT, handle_signal_sigint);
  int portNum = atoi(argv[2]);
  char * servAdd = argv[1];
  char * username = argv[3];

  struct sockaddr_in addr;
  char revstr[MAX_L] = {0};
  char filename[MAX_L];
  char* textEnt;


  // Creating a new socket in the TCP mode and adding the details to the addr 
  sock1 = socket(AF_INET, SOCK_STREAM, 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(portNum);
  addr.sin_addr.s_addr = inet_addr(servAdd);

  int sizeAddr = sizeof(addr);
  int ret2 = connect(sock1, (struct sockaddr*)&addr, sizeAddr);

  // Connecting with error handling
  if(ret2 < 0){
    perror("[-]ERROR IN CONNECTING TO THE SERVER. Server may be unreachable or more than 4 clients\n");
    exit(0);
  }

  //recieves a message from the server stating whether it is accepted or not
  bzero(buffer, sizeof(buffer));
  recv(sock1, buffer, MAX_L, 0);

  //denied cause no space, close the connection
  if(strcmp(buffer,"DENIED") == 0){
    printf("[-]Conncection refused! Server is full\n");
    close(sock1);
    exit(0);
  }
  else{
    //connection accepted
    printf("[+]Connection Accepted!\n");
  }

  bzero(buffer,sizeof(buffer));
  strcpy(buffer,username);
  send(sock1,buffer,sizeof(buffer),0);

  char test[4];
  recv(sock1, &test, sizeof(test), 0);
  while (strcmp(test, "YES") == 0) {
    bzero(buffer,sizeof(buffer));
    recv(sock1, buffer, MAX_L, 0);
    printf("Message recieved = %s\n",buffer);

    if(strcasecmp(buffer,"POLL") == 0){
        printf("Recieved POLLING request, enter message\n");
        bzero(buffer,sizeof(buffer));
        scanf("%s",buffer);
    }
    send(sock1,buffer,sizeof(buffer),0);

    if(strcasecmp(buffer,"LIST") == 0){
        bzero(buffer,sizeof(buffer));
        recv(sock1, buffer, MAX_L, 0);
        printf("Message recieved = %s\n",buffer);   
    } 
  }
  ret = close(sock1);
  printf("[-] Connection closed!\n");
  return 0;
}
