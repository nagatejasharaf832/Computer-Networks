// Below lines of code are taken from https://github.com/nikhilroxtomar/Stop-and-Wait-Protocol-Implemented-in-UDP-C

// All include statements
#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <netdb.h>

#include <sys/socket.h>

#include <signal.h>

#define TIMEOUT_MS 30

/*
 *  Here is the starting point for your netster part.3 definitions. Add the 
 *  appropriate comment header as defined in the code formatting guidelines
 */

/* Add function definitions */

// packet definition
typedef struct packet
{
  char data[240];
} Packet;

// frame initialization
typedef struct frame
{
  int frame_kind; //ACK:0, SEQ:1 FIN:2
  int sq_no;
  int ack;
  int packetSize;
  Packet packet;
} Frame;

// file size
int actual_file_size(Frame f)
{
  int actual_file_size_here = 0;
  int f_size = sizeof(f);
  printf("frame size %d\n", f_size);
  int frame_kind_size = sizeof(f.frame_kind);
  printf("frame kind %d\n", frame_kind_size);

  int frame_sq_no_size = sizeof(f.sq_no);
  printf("frame packet %d\n", frame_sq_no_size);
  int frame_ack_size = sizeof(f.ack);
  printf("frame packet %d\n", frame_ack_size);
  actual_file_size_here = f_size - frame_kind_size - frame_sq_no_size - frame_ack_size;
  return actual_file_size_here;
}

// Returns host information corresponding to host name
void checkHostEntry(struct hostent *hostentry)
{
  if (hostentry == NULL)
  {
    perror("gethostbyname");
    exit(1);
  }
}

// Converts space-delimited IPv4 addresses
// to dotted-decimal format
void checkIPbuffer(char *IPbuffer)
{
  if (NULL == IPbuffer)
  {
    perror("inet_ntoa");
    exit(1);
  }
}

// server side stop and wait implementation
void stopandwait_server(char *iface, long port, FILE *fp)
{

  struct addrinfo hints;
  struct addrinfo *result;
  int socket_fd;
  struct sockaddr_in server_address;
  char buffer_for_data[240];
  char buffer_for_data1[240];
  char str_port[240];
  int interface;
  int frame_id = 0;
  Frame frame_recv;
  Frame frame_send;
  bzero(buffer_for_data, 240);
  bzero(buffer_for_data1, 240);

  memset(&hints, 0, sizeof(struct addrinfo));

  memset(&server_address, 0, sizeof(server_address));
  // settings for server side connection
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
  hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
  hints.ai_protocol = 0;          /* Any protocol */
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  sprintf(str_port, "%ld", port);
  interface = getaddrinfo(NULL, str_port, &hints, &result);
  if (interface != 0)
  {
    perror("Error!!!!\n");
  }
  // socket creation for server side connection
  socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (socket_fd == -1)
  {
    printf("Socket Error\n");
    exit(0);
  }
  // bind socket to host
  if (bind(socket_fd, result->ai_addr, result->ai_addrlen) < 0)
  {
    printf("Bind Error\n");
    exit(0);
  }

  socklen_t addr_size;
  addr_size = sizeof(server_address);

  while (1)
  {
    int frame_size_here = sizeof(frame_recv);
    printf("Frame size %d\n", frame_size_here);
    // recieve packets from client
    int f_recv_size = recvfrom(socket_fd, &frame_recv, sizeof(Frame), 0, (struct sockaddr *)&server_address, &addr_size);
    printf("[+]Frame Received Seq: %d\n", frame_recv.sq_no);

    if (f_recv_size > 0 && frame_recv.frame_kind == 1)
    {
      if (frame_recv.sq_no == frame_id)
      {
        frame_send.ack = frame_recv.sq_no + 1;
        frame_id++;
        memcpy(buffer_for_data, frame_recv.packet.data, frame_recv.packetSize);
        // write the packet received to file
        fwrite(buffer_for_data, sizeof(unsigned char), frame_recv.packetSize, fp);
        fflush(fp);
      }
      else
      {
        frame_send.ack = frame_recv.sq_no;
      }

      printf("[+]ACK Frame: %d\n", frame_send.ack);

      frame_send.sq_no = 0;
      frame_send.frame_kind = 0;
      // send ack to client
      sendto(socket_fd, &frame_send, sizeof(frame_send), 0, (struct sockaddr *)&server_address, addr_size);
      printf("[+]Ack Send\n");
    }
    if (frame_recv.frame_kind == 2)
    {
      printf("File done\n");
      fflush(fp);
      break;
    }

    bzero(buffer_for_data, 240);
  }

  close(socket_fd);
}

// client side connection
void stopandwait_client(char *host, long port, FILE *fp)
{

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = TIMEOUT_MS * 1000;

  char *IPbuffer;
  struct hostent *host_entry;

  // To retrieve host information
  host_entry = gethostbyname(host);
  checkHostEntry(host_entry);

  IPbuffer = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));

  struct addrinfo hints;
  struct addrinfo *result;
  int sock, b;
  char buffer[240];
  char str_port[240];
  int interface;
  struct sockaddr_in addr;
  socklen_t length_of_server_address;

  int frame_id = 0;
  Frame frame_send;
  Frame frame_recv;
  int ack_recv = 1;
  //int ack_received_or_not;

  memset(&hints, 0, sizeof(struct addrinfo));
  // client side settings for socket connection
  hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
  hints.ai_protocol = 0;       /* Any protocol */
  hints.ai_flags = 0;
  sprintf(str_port, "%ld", port);
  interface = getaddrinfo(host, str_port, &hints, &result);
  if (interface != 0)
  {
    perror("Error!!!!\n");
  }
  // create socket

  sock = socket(result->ai_family, result->ai_socktype,
                result->ai_protocol);
  if (sock < -1)
  {
    printf("Error here\n");
  }
  // connect the socket to host address
  if (connect(sock, result->ai_addr, result->ai_addrlen) == -1)
  {
    perror("[-]Socket error........");
    exit(1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = result->ai_addr->sa_family;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(IPbuffer);
  length_of_server_address = sizeof(addr);

  while (1)
  {
    if (ack_recv == 0)
    {
      printf("Acknowledgement not received. Sending again\n");
      frame_send.sq_no = frame_id;
      frame_send.frame_kind = 1;
      frame_send.ack = 0;
      frame_send.packetSize = b;
      memcpy(frame_send.packet.data, buffer, 240);
      sendto(sock, &frame_send, sizeof(Frame), 0, (struct sockaddr *)&addr, sizeof(addr));
    }

    if (ack_recv == 1)
    {
      // read data from file
      b = fread(buffer, 1, sizeof(buffer), fp);
      if (b <= 0)
      {
        break;
      }

      frame_send.sq_no = frame_id;
      frame_send.frame_kind = 1;
      frame_send.ack = 0;
      frame_send.packetSize = b;
      printf("Data from file\n%s", buffer);
      memcpy(frame_send.packet.data, buffer, 240);
      int frame_size_here = sizeof(frame_send);
      printf("Frame size %d\n", frame_size_here);
      // send packets of data to server
      sendto(sock, &frame_send, sizeof(Frame), 0, (struct sockaddr *)&addr, sizeof(addr));
      printf("[+]Frame Send\n");
    }
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
    {
      perror("setsockopt failed\n");
    }
    // recieve ack from server
    int f_recv_size = recvfrom(sock, &frame_recv, sizeof(frame_recv), 0, (struct sockaddr *)&addr, &length_of_server_address);

    if (f_recv_size > 0 && frame_recv.sq_no == 0 && frame_recv.ack == frame_id + 1)
    {
      printf("\n[+]Ack Received\n");
      ack_recv = 1;
      frame_id++;
    }
    else
    {
      ack_recv = 0;
    }
  }
  frame_send.sq_no = frame_id;
  frame_send.frame_kind = 2;
  frame_send.ack = 0;
  bzero(buffer, 240);
  strcpy(frame_send.packet.data, buffer);
  sendto(sock, &frame_send, sizeof(Frame), 0, (struct sockaddr *)&addr, sizeof(addr));
  printf("\nFile done.\n");
  close(sock);
}