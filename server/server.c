/*///////////////////////////////////////////////////////////
*
* FILE:		server.c
* AUTHOR:	Eric Zhou
* PROJECT:	CNT 4007 Project 2 - Professor Traynor
* DESCRIPTION:	UFmyMusic
*
*////////////////////////////////////////////////////////////

/*Included libraries*/

#include <stdio.h>	  /* for printf() and fprintf() */
#include <sys/socket.h>	  /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>	  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>	  /* supports all sorts of functionality */
#include <unistd.h>	  /* for close() */
#include <string.h>	  /* support any string ops */
#include <openssl/evp.h>  /* for OpenSSL EVP digest libraries/SHA256 */

#define RCVBUFSIZE 512		/* The receive buffer size */
#define SNDBUFSIZE 512		/* The send buffer size */
#define BUFSIZE 40		/* Your name can be as many as 40 chars*/
#define MAXPENDING 10

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

void *handle_client(void *client_socket) {
    int clientSock = *(int *)client_socket;
    free(client_socket);
    char recvBuf[RCVBUFSIZE];
    int num_bytes;

    // Handle client messages
    while ((num_bytes = recv(clientSock, recvBuf, RCVBUFSIZE - 1, 0)) > 0) {
        recvBuf[num_bytes] = '\0';

        // Identify the command
        if (strncmp(recvBuf, "LIST", 4) == 0) {
            // Implement LIST functionality here

        } else if (strncmp(recvBuf, "DIFF", 4) == 0) {
            // Implement DIFF functionality here

        } else if (strncmp(recvBuf, "PULL", 4) == 0) {
            // Implement PULL functionality here

        } else if (strncmp(recvBuf, "LEAVE", 5) == 0) {
            // Implement LEAVE functionality here
            break;
        }
    }

    close(clientSock);
    pthread_exit(NULL);
}


/* The main function */
int main(int argc, char *argv[])
{

    int serverSock;				/* Server Socket */
    int clientSock;				/* Client Socket */
    struct sockaddr_in changeServAddr;		/* Local address */
    struct sockaddr_in changeClntAddr;		/* Client address */
    unsigned short changeServPort;		/* Server port */
    unsigned int clntLen;			/* Length of address data struct */

    char nameBuf[BUFSIZE];			/* Buff to store name from client */
    unsigned char md_value[EVP_MAX_MD_SIZE];	/* Buff to store change result */
    EVP_MD_CTX *mdctx;				/* Digest data structure declaration */
    const EVP_MD *md;				/* Digest data structure declaration */
    int md_len;					/* Digest data structure size tracking */


    /* Create new TCP Socket for incoming requests*/
    /*	    FILL IN	*/
    if((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        fatal_error("socket() failed");
    }

    /* Construct local address structure*/
    /*	    FILL IN	*/
    memset(&changeServAddr, 0, sizeof(changeServAddr));
    changeServAddr.sin_family = AF_INET;
    changeServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    changeServAddr.sin_port = htons(31313);

    /* Bind to local address structure */
    /*	    FILL IN	*/
    if(bind(serverSock, (struct sockaddr *) &changeServAddr, sizeof(changeServAddr)) < 0) {
        fatal_error("bind() failed");
    }

    /* Listen for incoming connections */
    /*	    FILL IN	*/
    if(listen(serverSock, MAXPENDING) < 0) {
        fatal_error("listen() failed");
    }

    /* Loop server forever*/
    while(1)
    {

        /* Accept incoming connection */
        /*	FILL IN	    */
        int clnt_len = sizeof(changeClntAddr);
        if((clientSock = accept(serverSock, (struct sockaddr *) &changeClntAddr, &clnt_len)) < 0) {
            fatal_error("accept() failed");
        }

        /* Extract Your Name from the packet, store in nameBuf */
        /*	FILL IN	    */
        int num_bytes;
        if((num_bytes = recv(clientSock, nameBuf, BUFSIZE-1, 0)) < 0) {
            fatal_error("recv() failed");
        }
        nameBuf[num_bytes] = '\0';  // Null-terminate the received string

        /* Run this and return the final value in md_value to client */
        /* Takes the client name and changes it */
        /* Students should NOT touch this code */
        OpenSSL_add_all_digests();
        md = EVP_get_digestbyname("SHA256");
        mdctx = EVP_MD_CTX_create();
        EVP_DigestInit_ex(mdctx, md, NULL);
        EVP_DigestUpdate(mdctx, nameBuf, strlen(nameBuf));
        EVP_DigestFinal_ex(mdctx, md_value, &md_len);
        EVP_MD_CTX_destroy(mdctx);

        /* Return md_value to client */
        /*	FILL IN	    */
        if(send(clientSock, md_value, md_len, 0) != md_len) {
            fatal_error("send() failed");
        }

        close(clientSock);
    }

    close(serverSock);

    return 0;
}

