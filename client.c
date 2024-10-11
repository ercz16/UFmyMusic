#include <stdio.h>		    /* for printf() and fprintf() */
#include <sys/socket.h>		    /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>		    /* for sockaddr_in and inet_addr() */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <openssl/evp.h>	    /* for OpenSSL EVP digest libraries/SHA256 */

/* Constants */
#define RCVBUFSIZE 512		    /* The receive buffer size */
#define SNDBUFSIZE 512		    /* The send buffer size */
#define MDLEN 32

typedef struct {
    int messageType; // 1 for LIST, 2 for DIFF, 3 for PULL, 4 for LEAVE
    char payload[256]; // Additional data (e.g., file names for PULL, DIFF)
} Message;

void fatal_error(const char *msg) {
    perror(msg);
    exit(1);
}

/* Function to display the menu */
void display_menu() {
    printf("Menu Options:\n");
    printf("1. List Files\n");
    printf("2. Diff\n");
    printf("3. Pull Files\n");
    printf("4. Leave\n");
    printf("Enter your choice: ");
}

/* The main function */
int main(int argc, char *argv[])
{
    int clientSock;		    /* socket descriptor */
    struct sockaddr_in serv_addr;   /* The server address */
    char rcvBuf[RCVBUFSIZE];	    /* Receive Buffer */

    /* Create a new TCP socket*/
    if((clientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        fatal_error("socket() failed");
    }

    /* Construct the server address structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(31313);

    /* Establish connection to the server */
    if(connect(clientSock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fatal_error("connect() failed");
    }

    int choice;
    Message msg;
    while (1) {
        display_menu();
        scanf("%d", &choice);

        memset(&msg, 0, sizeof(Message)); // Clear the message struct
        msg.messageType = choice;

        switch (choice) {
            case 1: // LIST
                printf("Requesting file list from server...\n");
                break;
            case 2: // DIFF
                printf("Sending DIFF request...\n");
                // Example: msg.payload could be a directory path if needed
                strncpy(msg.payload, "some/path", sizeof(msg.payload) - 1);
                break;
            case 3: // PULL
                printf("Enter file name to pull: ");
                scanf("%s", msg.payload);
                break;
            case 4: // LEAVE
                printf("Closing connection...\n");
                send(clientSock, &msg, sizeof(Message), 0);
                close(clientSock);
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
                continue;
        }

        /* Send the message to the server */
        if(send(clientSock, &msg, sizeof(Message), 0) != sizeof(Message)) {
            fatal_error("send() sent unexpected number of bytes");
        }

        /* Receive and print response from the server */
        int num_bytes;
        if ((num_bytes = recv(clientSock, rcvBuf, RCVBUFSIZE-1, 0)) < 0) {
            fatal_error("recv() failed");
        }
        rcvBuf[num_bytes] = '\0';

        printf("Server response: %s\n", rcvBuf);
    }

    close(clientSock);
    return 0;
}
