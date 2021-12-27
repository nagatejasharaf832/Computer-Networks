// all include statements
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#define MSG_SIZE 200
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <string.h>

// frame definition
typedef struct frame
{
    int frame_def;
    int sequence_nnumber;
    char data[MSG_SIZE];
    int data_size;
} Frame;

// server side logic for go back n protocol
void gbn_server(char *iface, long port, FILE *fp)
{
    Frame frame_ack;
    Frame frame_receive;
    int sockfd;
    struct sockaddr_in server_address;

    //settings for server side connection
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    int n;
    // create socket for server
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    // bind socket to host
    bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int sequence_number = 0;

    while (1)
    {
        // recieve data from client
        n = recvfrom(sockfd, &frame_receive, sizeof(frame_receive), 0, (struct sockaddr *)&client_addr, &len);
        //frames recieved or correct or not

        if (n > 0 && frame_receive.frame_def == 1)
        {
            if (sequence_number == frame_receive.sequence_nnumber)
            {

                // write frames recieved to file
                fwrite(frame_receive.data, sizeof(char), frame_receive.data_size, fp);
                frame_ack.frame_def = 0;
                frame_ack.sequence_nnumber = sequence_number;
                sequence_number++;

                printf("client ack");
                // send ack to client
                sendto(sockfd, &frame_ack, sizeof(frame_ack), 0, (struct sockaddr *)&client_addr, len);

                if (frame_receive.data_size < MSG_SIZE - 1)
                {
                    printf("Exit once end of message reached");
                    exit(0);
                }
            }
            else
            {
                frame_ack.frame_def = 0;
                frame_ack.sequence_nnumber = sequence_number - 1;
                // decrement sequence number and send ack to client
                sendto(sockfd, &frame_ack, sizeof(frame_ack), 0, (struct sockaddr *)&client_addr, len);
            }
        }
    }
}

// client side logic for go back n protocol
void gbn_client(char *host, long port, FILE *fp)
{
    int sockfd;
    struct sockaddr_in server_address;

    //check if localhost and assign the loopback addresss to it
    if (strncmp("localhost", host, strlen("localhost")) == 0)
    {
        host = "127.0.0.1";
    }

    //settings for client side socket connection
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(host);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // Initialze the time
    struct timeval t_val;
    t_val.tv_sec = 0;
    t_val.tv_usec = 150000;

    //set time out
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_val, sizeof(struct timeval));

    size_t bytes_data;
    char cli_message_format[MSG_SIZE];
    bzero(cli_message_format, sizeof(cli_message_format));

    char sv_msg[MSG_SIZE];
    bzero(sv_msg, sizeof(sv_msg));
    int count = 0;

    // read data of the file as bytes and send it as chunks
    while ((bytes_data = fread(sv_msg, sizeof(char), MSG_SIZE - 1, fp)) > 0)
    {
        count++;
    }

    //set file position to the given offset that is the beginning of the file
    fseek(fp, 0, SEEK_SET);

    Frame frame_array[count];
    int seq_no = 0;
    int ct = 0;

    while ((bytes_data = fread(cli_message_format, sizeof(char), MSG_SIZE - 1, fp)) > 0)
    {
        //set frame details
        Frame frame;
        frame.data_size = bytes_data;
        frame.frame_def = 1;
        frame.sequence_nnumber = seq_no;

        memcpy(frame.data, cli_message_format, bytes_data);
        memcpy(&frame_array[ct], &frame, sizeof(Frame));
        ct++;
        seq_no++;
    }

    int frame_count = seq_no;

    short window_start = 0;
    short window_size = 5;
    Frame ack_frame;
    int sequence_number = 0;
    while (1)
    {
        if (window_start >= frame_count)
        {
            break;
        }

        // Send all the data in current frame
        while (sequence_number < window_start + window_size)
        {

            printf("Sequence number of the frame sent is: %d\n", sequence_number);
            // sending data to server
            sendto(sockfd, &frame_array[sequence_number], sizeof(Frame), 0, (struct sockaddr *)&server_address, sizeof(server_address));

            sequence_number = sequence_number + 1;
        }

        // Receive Ack from server
        int rec_ack = recvfrom(sockfd, &ack_frame, sizeof(Frame), 0, NULL, NULL);
        if (rec_ack > 0)
        {
            printf("The acknowledgement received\n");

            // When the final acknowledgement is received then end the program
            if (ack_frame.sequence_nnumber + 1 == frame_count)
            {
                printf("Inside break\n");
                exit(0);
            }
            window_start = ack_frame.sequence_nnumber + 1;

            //increment window size if no congestion
            window_size = window_size + 1;
        }
        else
        {
            printf("The timeout occured now resend \n");
            sequence_number = window_start;
            //we try to reduce the window size by half every time congestion occurs
            window_size = window_size / 3;
            if (window_size < 1)
            {
                window_size = 1;
            }
        }
    }
}