
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

// server side code
void file_server(char *iface, long port, int use_udp, FILE *fp)
{
  // UDP file trasfer code
  if (use_udp)
  {
    int sockfd;
    int SIZE = 256;
    char buffer[SIZE];
    struct sockaddr_in servaddr, cliaddr;
    // creating Socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror("socket creation failed");
      exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    // configure settings for server
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    // bind the server addr
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
      perror("bind failed");
      exit(EXIT_FAILURE);
    }

    int n;
    bzero(buffer, SIZE);
    int len = sizeof(cliaddr);
    // receive data from client
    while (1)
    {
      // use recvfrom while using udp protocal
      n = recvfrom(sockfd, buffer, SIZE, 0, (struct sockaddr *)&cliaddr, (unsigned int *)&len);
      if (strcmp(buffer, "END") == 0)
      {
        break;
        return;
      }
      if (n <= 0)
      {
        break;
      }
      // file writing
      fwrite(buffer, 1, n, fp);
      bzero(buffer, SIZE);
    }
  }
  // TCP file transfer code
  else
  {
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_in cli_addr;
    socklen_t addr_size;
    int n;
    char buffer[256];

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
    // memset(serverAddr.sin_zero, 0, sizeof serverAddr.sin_zero);

    //Bind the address struct to the socket
    bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    //Listen on the socket, with 40 max connection requests queued
    if (listen(serverSocket, 50) == -1)
    {
      printf("Error\n");
    }
    addr_size = sizeof cli_addr;
    newSocket = accept(serverSocket, (struct sockaddr *)&cli_addr, &addr_size);
    bzero(buffer, 256);
    // loop all the lines in the file until END
    while (1)
    {
      n = recv(newSocket, buffer, 256, 0);
      if (n)
      {
        fwrite(buffer, 1, n, fp);
        bzero(buffer, 256);
      }
      else
      {
        break;
        return;
      }
    }
  }
}

// Client file transfer to server address
void send_file(FILE *fp, int sockfd, int use_udp, struct sockaddr_in addr)
{
  char data[256] = {0};
  // UDP client sending file code
  if (use_udp)
  {
    int temp_size;
    // first step to read file size without this server cannot read the data correctly
    while ((temp_size = fread(data, 1, 256, fp)) > 0)
    {
      // loop and sending the data to server using sendto
      if (sendto(sockfd, data, temp_size, 0, (const struct sockaddr *)&addr, sizeof(addr)) == -1)
      {
        perror("[-]Error in sending file.");
      }
    }
    // ADD end string at the end of file to recognize its ending point
    strcpy(data, "END\n");
    if (sendto(sockfd, data, temp_size, 0, (const struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
      perror("[-]Error in sending file.");
    }
    bzero(data, 256);
  }
  // TCP file sending code
  else
  {
    int temp_size;
    // first step to read file size without this server cannot read the data correctly
    while ((temp_size = fread(data, 1, 256, fp)) > 0)
    {
      if (send(sockfd, data, temp_size, 0) == -1)
      {
        perror("[-]Error in sending file.");
        exit(1);
      }
    }
    bzero(data, 256);
    printf("[+]Closing the connection.\n");
    close(sockfd);
  }
}

// Client side file transfer code
void file_client(char *host, long port, int use_udp, FILE *fp)
{

  int clientSocket;
  int k;
  struct sockaddr_in serverAddr;
  if (use_udp)
  {
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
  }
  else
  {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  }

  memset(&serverAddr, 0, sizeof(serverAddr));
  // construct server address
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  // client socket connection with server
  k = connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
  if (k == -1)
  {
    printf("Error in connecting to server");
    exit(1);
  }
  if (fp == NULL)
  {
    perror("[-]Error in reading file.");
    exit(1);
  }

  send_file(fp, clientSocket, use_udp, serverAddr);

  printf("[+]File data sent successfully.\n");

  printf("[+]Closing the connection.\n");
}
