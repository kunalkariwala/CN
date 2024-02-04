#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_L 1024
#define NUMTHREAD 3


//global arrays to store the sockets, to keep a track of how many active connections there are 
//and how many have sent a message already
// int CLIENT_TRACKER[NUMTHREAD];

int CLIENT_SOCKETS[NUMTHREAD];
pthread_t tid[NUMTHREAD];
char * usernames[NUMTHREAD];

int sentPOLL[NUMTHREAD];
int gonext = 0;
int atleastone = 0;

struct headerTemp {
    int found;
    long bytes;
};

//defining the struct that will be passed to every thread
typedef struct threadstruct{
  int index;
  int newSocket;
  struct sockaddr_in clientAddr;
} threadstruct;

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

void * pollingThread(void * args){
    printf("Polling thread is active\n");
    char pbuffer[MAX_L] = {0};
    while (1)
    {
        int i = 0;
        for(i = 0 ; i < NUMTHREAD;i++){
            if(CLIENT_SOCKETS[i] == -1){
                continue;
            }
            if(sentPOLL[i] == -1){break;}
        }
        if(gonext == 0 && i < NUMTHREAD){
            sleep(2);
            printf("Server sends a POLLING request to client with username as %s\n",usernames[i]);
            bzero(pbuffer,sizeof(pbuffer));
            strcpy(pbuffer,"POLL");
            send(CLIENT_SOCKETS[i], pbuffer, sizeof(pbuffer), 0);
            sentPOLL[i] = 1;
            gonext = 1;
        }
        int all = 0;
        for(int i = 0; i < NUMTHREAD;i++){
            if(sentPOLL[i] == -1){
                all = 1;
            }
        }
        if(all == 0){
            memset(sentPOLL,-1,sizeof(sentPOLL));
        }
    }
    
}

//thread function to be attached to every time a new client is accepted
void * socketThread(void * args){
  threadstruct * t = (threadstruct *)args;
  int newsock = t->newSocket;
  int port = ntohs(t->clientAddr.sin_port);
  char * ip = inet_ntoa(t->clientAddr.sin_addr);
  int index = t->index;
  FILE *filePointer;
  //printing acceptance message
  printf("[+]Connection accepted from [%s:%d] having username as %s\n", ip, port,usernames[index]);
  char globbuff[MAX_L] = {0};
  send(newsock, "YES", sizeof("YES"), 0);
  while (1) {
      bzero(globbuff,sizeof(globbuff));
      recv(CLIENT_SOCKETS[index],globbuff,sizeof(globbuff),0);

      if(strcasecmp(globbuff,"NONE") == 0){
          gonext = 0;
      }
      if(strcasecmp(globbuff,"EXIT") == 0){
            gonext = 0;
            printf("[+]Client with ip:port as %s:%d has safely exited\n", ip,port);
            break;
      }
      char str_buf[2048];
      //printf("%s %s %s\n",usernames[0],usernames[1],usernames[2]);
      if(strcmp(globbuff, "LIST") == 0) {
        char buff[MAX_L];
        bzero(buff, sizeof(buff));
        printf("THIS IS WORKING\n");
        for(int i = 0; i < NUMTHREAD;i++){
            bzero(buff, sizeof(buff));
            if(CLIENT_SOCKETS[i] == -1){
                continue;
            }
            strcpy(buff,usernames[i]);
            printf("Active user = %s\n",buff);
        }
        gonext = 1;
      }
      
        // sending str_buff to client
        if(send(CLIENT_SOCKETS[index], str_buf, strlen(str_buf), 0) == -1) {
            perror("Failed to send message to client");
            return -1;
        }
        continue;
  }
  //once a client exits, we create space for a new connection to happen
  //send(CLIENT_SOCKETS[index], "QUIT", sizeof("QUIT"), 0);
  CLIENT_SOCKETS[index] = -1;
}


int main(int argc, char const *argv[]){
  //create sockets for the connection
  int sock1, sock2; // Socket file descriptor for server.
  char * username;
  //initialise the arrays with -1
  memset(CLIENT_SOCKETS,-1,sizeof(CLIENT_SOCKETS));
  // memset(DID_SEND_MESSAGE,-1,sizeof(DID_SEND_MESSAGE));
  memset(sentPOLL,-1,sizeof(sentPOLL));
  //get the port num from the command line arg
  int portNum = atoi(argv[1]);

  //initiliase address structs for sockets
  struct sockaddr_in addr;
  struct sockaddr_in clientAddr;
  socklen_t clientAddr_Size;


  //initialising the buffer with 0
  char buffer[MAX_L] = {0};

  // Creating a new socket in the TCP mode and adding the details to the addr 
  sock1 = socket(AF_INET, SOCK_STREAM, 0); // We use AF_INET for IPv6 and SOCK_STREAM = relaible and connection oriented.
  if(sock1<0){ printf("> Error in opening socket\n"); return 0; }  

  strcpy(buffer, "0.0.0.0");

  addr.sin_family = AF_INET;
  addr.sin_port = htons(portNum);
  addr.sin_addr.s_addr = inet_addr(buffer);

  int sizeAddr = sizeof(addr);

  // Binding the address to the socket with error handling
  if(bind(sock1, (struct sockaddr *)&addr, sizeAddr) < 0){
    perror("[-]error in binding!!!!");
    return 0;
  }
  
  //printing success message for server binding
  printf("[+]Server is active on [%s : %d]\n",inet_ntoa(addr.sin_addr),portNum);
  pthread_t tid1;
  pthread_create(&tid1, NULL,pollingThread,NULL);

  listen(sock1, 1); // This is needed so that the socket waits for client to approach. The number 1 is the maximum queue size.  

  int i;
  for(;;){
    //server accepts all requests
    sock2 = accept(sock1, (struct sockaddr *) &clientAddr, &clientAddr_Size);
    if(sock2 < 0){
      //if error in accepting
      continue;
    }
    //function to find an empty spot(check whether number of active connections is less than 4)
    for(i = 0 ; i < NUMTHREAD;i++){
      if(CLIENT_SOCKETS[i] == -1){
        break;
      }
    }

    //if there are 5 already, then deny the connection to go any further and close the connection
    if(i == NUMTHREAD){
      strcpy(buffer, "DENIED");
      send(sock2, buffer, strlen(buffer), 0);
      bzero(buffer, sizeof(buffer));
    }
    else{
      //found a place, make a new thread
      strcpy(buffer, "ACCEPTED");
      send(sock2, buffer, strlen(buffer), 0);
      bzero(buffer, sizeof(buffer));

      bzero(buffer, sizeof(buffer));
      recv(sock2, buffer, MAX_L, 0);
    //   strcpy(username, buffer);

      //store the socket in the socket array for broadcast and mark its presence
      // CLIENT_TRACKER[i] = 1;
      CLIENT_SOCKETS[i] = sock2;
      usernames[i] = buffer; 

      threadstruct * temp = malloc (sizeof (threadstruct));
      temp->index = i;
      temp->newSocket = sock2;
      temp->clientAddr = clientAddr;

      //create the thread and start the chatting for every client
      pthread_create(&tid[i], NULL,socketThread, (void *)temp);
      

    }

  }
  return 0;
}
