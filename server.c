#include <stdio.h>	  /* for printf() and fprintf() */
#include <sys/socket.h>	  /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>	  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>	  /* supports all sorts of functionality */
#include <unistd.h>	  /* for close() */
#include <string.h>	  /* support any string ops */
#include <openssl/evp.h>  /* for OpenSSL EVP digest libraries/SHA256 */
#include <pthread.h>      /* for multithreading */
#include <signal.h>     /* for signal handling like control-c */

#define RCVBUFSIZE 512		/* The receive buffer size */
#define SNDBUFSIZE 512		/* The send buffer size */
#define BUFSIZE 40		/* Your name can be as many as 40 chars*/
#define MAXPENDING 10

int running = 1;

typedef struct {
    int messageType; // 1 for LIST, 2 for DIFF, 3 for PULL, 4 for LEAVE
    char payload[256]; // Additional data (e.g., file names for PULL, DIFF)
} Message;

typedef struct {
    char filename[256];
    unsigned long filesize;
    char hash[64]; // A hash to check file contents without sending the entire file
} FileInfo;

typedef struct {
    FileInfo files[100]; // Array to store information about up to 100 files
    int fileCount; // Number of files currently in the array
} FileList;

void fatal_error(const char *msg) {
    perror(msg);
    exit(1);
}

// void handle_signal(int sig) {
//     if (sig == SIGINT) {
//         printf("\nShutting down server...\n");
//         running = 0;
//     }
// }

/* Function to handle LIST request */
void handle_list(int clientSock) {
    char response[] = "List of files: ..."; // Replace with actual file list logic
    send(clientSock, response, strlen(response), 0);
}

/* Function to handle DIFF request */
void handle_diff(int clientSock, char *payload) {
    char response[] = "Differences calculated"; // Replace with actual diff logic
    send(clientSock, response, strlen(response), 0);
}

/* Function to handle PULL request */
void handle_pull(int clientSock, char *filename) {
    char response[] = "Sending file..."; // Replace with actual file transfer logic
    send(clientSock, response, strlen(response), 0);
}

/* Function to handle client connections */
void *handle_client(void *client_socket) {
    int clientSock = *(int *)client_socket;
    free(client_socket);
    
    Message msg;
    int num_bytes;
    
    // Handle client messages
    while ((num_bytes = recv(clientSock, &msg, sizeof(Message), 0)) > 0) {
        printf("Received messageType: %d\n", msg.messageType);
        
        switch (msg.messageType) {
            case 1: // LIST
                handle_list(clientSock);
                break;
            case 2: // DIFF
                handle_diff(clientSock, msg.payload);
                break;
            case 3: // PULL
                handle_pull(clientSock, msg.payload);
                break;
            case 4: // LEAVE
                printf("Client requested to leave.\n");
                close(clientSock);
                pthread_exit(NULL);
            default:
                printf("Unknown message type received.\n");
        }
    }
    
    if (num_bytes < 0) {
        perror("recv() failed");
    }

    printf("Closing client socket\n");
    close(clientSock);
    pthread_exit(NULL);
}

/* The main function */
int main(int argc, char *argv[])
{
    int serverSock;				/* Server Socket */
    struct sockaddr_in servAddr;		/* Local address */
    struct sockaddr_in clntAddr;		/* Client address */
    unsigned short servPort = 31313;		/* Server port */
    unsigned int clntLen;			/* Length of address data struct */

    /* Create new TCP Socket for incoming requests */
    if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        fatal_error("socket() failed");
    }

    /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(servPort);

    /* Bind to local address structure */
    if (bind(serverSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        fatal_error("bind() failed");
    }

    /* Listen for incoming connections */
    if (listen(serverSock, MAXPENDING) < 0) {
        fatal_error("listen() failed");
    }

    printf("Server is running and waiting for connections...\n");
    // signal(SIGINT, handle_signal); // Wait for control-c

    /* Loop server forever */
    while (running) {
        clntLen = sizeof(clntAddr);
        int *clientSock = malloc(sizeof(int));
        if ((*clientSock = accept(serverSock, (struct sockaddr *) &clntAddr, &clntLen)) < 0) {
            fatal_error("accept() failed");
        }

        printf("Handling client...\n");

        pthread_t threadID;
        if (pthread_create(&threadID, NULL, handle_client, (void *)clientSock) != 0) {
            fatal_error("pthread_create() failed");
        }
        
        pthread_detach(threadID); // Detach the thread so it cleans up after itself
    }

    close(serverSock);
    return 0;
}
