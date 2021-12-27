// /*
//  *  Here is the starting point for your netster part.1 definitions. Add the 
//  *  appropriate comment header as defined in the code formatting guidelines
//  */

// /* Add function definitions */

// Below all lines of code were referenced from https://github.com/nikhilroxtomar/Chatroom-in-C
#include <fcntl.h> // for open

#include <unistd.h>

#include <stdlib.h>

#include <stdio.h>

#include <sys/types.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <string.h>

#include <arpa/inet.h>

#include <pthread.h>

#include <semaphore.h>

// structure definition for getting client details
struct ipc {
  char ip[15];
  long por;
  int cl_socket;
};

// client side socket connection
void chat_client(char * host, long port, int use_udp) {
  int clientSocket;
  int k;
  struct sockaddr_in serverAddr;
  char buffer[256];
  if (use_udp) {
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
  } else {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  }

  memset( & serverAddr, '\0', sizeof(serverAddr));
// construct server address  
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
// client socket connection with server
  k = connect(clientSocket, (struct sockaddr * ) & serverAddr, sizeof(serverAddr));
  if (k == -1) {
    printf("Error in connecting to server");
    exit(1);
  }
  while (1) {
    if (fgets(buffer, 256, stdin)) {
        // sending and recieving message from server
      if ((strncmp(buffer, "goodbye", (strlen(buffer)) - 1) == 0)) {
        k = send(clientSocket, buffer, 256, 0);
        if (k == -1) {
          printf("Error in sending message to server");
          exit(1);
        }
        k = recv(clientSocket, buffer, 256, 0);
        if (k == -1) {
          printf("Error in receiving message from server");
          exit(1);
        }
        printf("%s\n", buffer);
        exit(1);
      }
      if ((strncmp(buffer, "exit", (strlen(buffer)) - 1) == 0)) {
        k = send(clientSocket, buffer, 256, 0);
        if (k == -1) {
          printf("Error in sending message to server");
          exit(1);
        }
        k = recv(clientSocket, buffer, 256, 0);
        if (k == -1) {
          printf("Error in receiving message from server");
          exit(1);
        }
        printf("%s\n", buffer);
        exit(1);
      }
      k = send(clientSocket, buffer, 256, 0);
      if (k == -1) {
        printf("Error in sending message to server");
        exit(1);
      }
      k = recv(clientSocket, buffer, 256, 0);
      if (k == -1) {
        printf("Error in receiving message from server");
        exit(1);
      }
      printf("%s\n", buffer);

    }
  }
}

// TCP server side multithreading
void * socketThread(void * arg) {
  struct ipc * c = arg;
  int cli_socket = c -> cl_socket;
  char * ip = c -> ip;
  long po = c -> por;
  char buffer[256];
  int thread_sleep_time = 0, k;
  while (1) {
    //   recieving message from client and sending response from server
    k = recv(cli_socket, buffer, 256, 0);
    if (k == -1) {
      printf("Error in recieving message from client");
    }
    if (strncmp(buffer, "hello", (strlen(buffer)) - 1) == 0) {
      printf("got message from ('%s', %d)\n", ip, ntohs(po));
      strcpy(buffer, "world");
      send(cli_socket, buffer, strlen(buffer), 0);
    } else if ((strncmp(buffer, "goodbye", (strlen(buffer)) - 1)) == 0) {
      printf("got message from ('%s', %d)\n", ip, ntohs(po));
      strcpy(buffer, "farewell");
      send(cli_socket, buffer, 256, 0);

      close(cli_socket);
      pthread_exit(NULL);

    } else if ((strncmp(buffer, "exit", (strlen(buffer)) - 1)) == 0) {
      printf("got message from ('%s', %d)\n", ip, ntohs(po));
      strcpy(buffer, "ok");
      send(cli_socket, buffer, 256, 0);
      close(cli_socket);
      exit(1);
    } else {
      printf("got message from ('%s', %d)\n", ip, ntohs(po));
      send(cli_socket, buffer, 256, 0);
    }
    sleep(thread_sleep_time);

  }
  return NULL;
}

void * socketThread1(void * arg) {

  struct ipc * c = arg;
  struct sockaddr_in newAddr;
  socklen_t addr_size;
  addr_size = sizeof(newAddr);
  int cli_socket = c -> cl_socket;
  char buffer[256];
  int thread_sleep_time = 0, k;
  while (1) {
      //   recieving message from client and sending response from server
    k = recvfrom(cli_socket, buffer, 256, 0, (struct sockaddr * ) & newAddr, & addr_size);
    if (k == -1) {
      printf("Error in recieving message from client");
    }
    if (strncmp(buffer, "hello", (strlen(buffer)) - 1) == 0) {
      printf("got message from ('%s', %d)\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
      strcpy(buffer, "world");
      sendto(cli_socket, buffer, 256, 0, (struct sockaddr * ) & newAddr, addr_size);
    } else if ((strncmp(buffer, "goodbye", (strlen(buffer)) - 1)) == 0) {
      printf("got message from ('%s', %d)\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
      strcpy(buffer, "farewell");
      sendto(cli_socket, buffer, 256, 0, (struct sockaddr * ) & newAddr, addr_size);
    } else if ((strncmp(buffer, "exit", (strlen(buffer)) - 1)) == 0) {
      printf("got message from ('%s', %d)\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
      strcpy(buffer, "ok");
      sendto(cli_socket, buffer, 256, 0, (struct sockaddr * ) & newAddr, addr_size);
      close(cli_socket);
      exit(1);
    } else {
      printf("got message from ('%s', %d)\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
      sendto(cli_socket, buffer, 256, 0, (struct sockaddr * ) & newAddr, addr_size);
      pthread_exit(NULL);
    }
    sleep(thread_sleep_time);

  }
  close(cli_socket);
  return NULL;
}
void chat_server(char * iface, long port, int use_udp)

{
  if (use_udp) {
    int sockfd;
    struct sockaddr_in serverAddr;
    //Create the socket.
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset( & serverAddr, '\0', sizeof(serverAddr));
    // Configure settings of the server address struct
    // Address family = Internet 
    serverAddr.sin_family = AF_INET;
    //Set port number, using htons function to use proper byte order
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(sockfd, (struct sockaddr * ) & serverAddr, sizeof(serverAddr));
    pthread_t tid[60];
    int i = 0;
    while (1) {
        // Fetching connected client details from server
      struct ipc ip_details;
      ip_details.cl_socket = sockfd;
      ip_details.por = port;
      strcpy(ip_details.ip, inet_ntoa(serverAddr.sin_addr));
      if (pthread_create( & tid[i++], NULL, socketThread1, & ip_details) != 0)
        printf("Failed to create thread\n");

      if (i >= 50) {
        i = 0;
        while (i < 50) {
          pthread_join(tid[i++], NULL);
        }
        i = 0;
      }
    }
    close(sockfd);
  } else {
    int serverSocket, newSocket, cou = 0;
    struct sockaddr_in serverAddr;
    struct sockaddr_in cli_addr;
    socklen_t addr_size;

    //Create the socket. 
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Configure settings of the server address struct
    // Address family = Internet 
    serverAddr.sin_family = AF_INET;

    //Set port number, using htons function to use proper byte order 
    serverAddr.sin_port = htons(port);

    //Set IP address to localhost 
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    //Set all bits of the padding field to 0 
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    //Bind the address struct to the socket 
    bind(serverSocket, (struct sockaddr * ) & serverAddr, sizeof(serverAddr));

    //Listen on the socket, with 40 max connection requests queued 
    if (listen(serverSocket, 50) == -1) {
      printf("Error\n");
    }
    pthread_t tid[60];
    int i = 0;
    while (1) {
      //Accept call creates a new socket for the incoming connection

      addr_size = sizeof cli_addr;
      newSocket = accept(serverSocket, (struct sockaddr * ) & cli_addr, & addr_size);
      printf("connection %d from ('%s', %d)\n", cou, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
      cou = cou + 1;
    //   Fetching client details from server
      struct ipc ip_details;
      ip_details.cl_socket = newSocket;
      ip_details.por = (cli_addr.sin_port);
      strcpy(ip_details.ip, inet_ntoa(cli_addr.sin_addr));
      //for each client request creates a thread and assign the client request to it to process
      //so the main thread can entertain next request
      if (pthread_create( & tid[i++], NULL, socketThread, & ip_details) != 0)
        printf("Failed to create thread\n");

      if (i >= 50) {
        i = 0;
        while (i < 50) {
          pthread_join(tid[i++], NULL);
        }
        i = 0;
      }
    }
    close(serverSocket);
  }
}