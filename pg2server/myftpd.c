/*
 * CSE30264: Programming Assignment 2
 * Chris Foley (cfoley)
 * Catherine Markley (cmarkley)
 * Bailey Blum (bblum1)
 *
 * myftpd.c (Server)
 *
 * TODO: CODE DESCRIPTION
*/

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>


#define MAX_BUFFER_SIZE 4096

void list_handler(int, struct sockaddr_in);

int send_short(short value, int socket) {
    int len;
    short temp = htonl(value);
    if ((len = write(socket, &temp, sizeof(temp))) == -1) {
        perror("ERROR: Client Send");
        exit(1);
    }
}

int receive_short(int s) {
  int buffer;
  int len;
  if ((len = read(s, &buffer, sizeof(buffer))) == -1) {
      perror("ERROR: Client Receive");
      exit(1);
  }

  short temp = ntohl(buffer);
  return temp;
}

int send_buffer(int s, char* buffer, int size) {
    int len;
    if ((len = write(s, buffer, size)) == -1) {
        perror("ERROR: Client Send\n");
        exit(1);
    }
    return len;
}

int receive_buffer(int s, char* buffer, int size) {
    int len;
    if ((len = read(s, buffer, size)) == -1) {
        perror("ERROR: Receive Error!");
        exit(1);
    }
    return len;
}

int send_int(int value, int socket) {
	  int len;
    int temp = htonl(value);
    if ((len = write(socket, &temp, sizeof(temp))) == -1) {
        perror("ERROR: Client Send");
        exit(1);
    }
}

int receive_int(int s) {
    int buffer;
    int len;
    if ((len = read(s, &buffer, sizeof(buffer))) == -1) {
        perror("ERROR: Client Receive");
        exit(1);
    }

    int temp = ntohl(buffer);
    return temp;
}

int main(int argc, char* argv[]) {


    // signal (SIGINT, my_handler);

    int port = 41030;
    // int port = 41015;


    /* Initialize variables */
    int sockfd, clientAddrLen;
    struct sockaddr_in serverAddr, clientAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    memset(&clientAddr, 0, sizeof(clientAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    /* Establish the server's own ip address */
    char hostBuffer[256];

    if (gethostname(hostBuffer, sizeof(hostBuffer)) < 0) {
        fprintf(stderr, "%s: Failed to get current host name\n", argv[0]);
        exit(-1);
    };

    char* IPbuffer = inet_ntoa(*((struct in_addr*) gethostbyname(hostBuffer)->h_addr_list[0]));

    serverAddr.sin_addr.s_addr = inet_addr(IPbuffer);

    /* Initialize server to accept incoming connections */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        fprintf(stderr, "%s: Failed to call socket\n", argv[0]);
        exit(-1);
    }

    printf("Created socket\n");

    if (bind(sockfd, (const struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0 ) {
        fprintf(stderr, "%s: Failed to bind socket: %s\n", argv[0], strerror(errno));
        exit(-1);
    }

    printf("Binded socket\n");

    if((listen(sockfd, SOMAXCONN)) < 0){
      fprintf(stderr, "%s: Failed to listen: %s\n", argv[0], strerror(errno));
      exit(-1);
    }
    // TESTING CHDIR
    printf("changing directory!\n");
    execvp("/bin/ls", "ls");
    printf("changed directory\n");

    printf("Listened\n");
    printf("awaiting connection\n");
    int clientSocket;
    if((clientSocket = accept(sockfd, (struct sockaddr *) &clientAddr, &clientAddrLen)) < 0){
      fprintf(stderr, "%s: Failed to accept: %s\n", argv[0], strerror(errno));
      exit(-1);
    }
    printf("accepted a connection\n");

    // Receive first client operation
    printf("Receiving client operation\n");
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    socklen_t addr_len = sizeof(clientAddr);
    int n_received = receive_buffer(clientSocket, buf, 4096);
    printf("Received client operation: %d, %s\n", n_received, buf);

    int count = 0;
    while (strcmp(buf, "QUIT") != 0 && n_received >= 0 && count++ < 5) {

      // DNLD: Download
      if (strcmp(buf, "DNLD") == 0) {
        printf("Received DNLD command\n");

      }
      // UPLD: Upload
      else if (strcmp(buf, "UPLD") == 0) {
        printf("Received UPLD command\n");
      }
      // LIST: List
      else if (strcmp(buf, "LIST") == 0) {
        printf("Received LIST command\n");
        list_handler(clientSocket, clientAddr);

      }
      // MKDR: Make Directory
      else if (strcmp(buf, "MKDR") == 0) {
        printf("Received MKDIR command\n");

      }
      // RMDR: Remove Directory
      else if (strcmp(buf, "RMDR") == 0) {
        printf("Received RMDR command\n");

      }
      // CHDR: Change Directory
      else if (strcmp(buf, "CHDR") == 0) {
        printf("Received CHDR command\n");

      }
      // CRFL: Create File
      else if (strcmp(buf, "CRFL") == 0) {
        printf("Received CRFL command\n");



      }
      // RMFL: Remove File
      else if (strcmp(buf, "RMFL") == 0) {
        printf("Received RMFL command\n");

      }
      // Default case
      else {
        printf("Recieved unknown command\n");
        // TODO: Handle this
      }

      memset(buf, 0, sizeof(buf));
      int n_received = receive_buffer(clientSocket, buf, 4096);
      printf("Received client operation: %d, %s\n", n_received, buf);

    }

    // Close sockets
    printf("Shutting down!\n");
    close(sockfd);
    close(clientSocket);

}

/*
 * List()
 *
 * Obtains listing of directory and sends back to client
*/
void list_handler(int clientSocket, struct sockaddr_in clientAddr){
  char buffer[MAX_BUFFER_SIZE];
  char* listing;
  int totalBytesRead = 0;
  int bytesRead;
  int cnt = 0;
  FILE* fp;

  // Obtain list of directory
  fp = popen("ls -l", "r");

  // Determine size of directoy
  while((bytesRead = fread(buffer, sizeof(char), sizeof(buffer)-1, fp)) > 0) {
    totalBytesRead += bytesRead;
  }
  printf("Total bytes of directory: %d\n", totalBytesRead);

  // Send size of directory
  send_int(totalBytesRead, clientSocket);

  // Read directoy listing
  char directoryListing[totalBytesRead];
  fp = popen("ls -l", "r");
  //printf("Did fseek work?: %d\n\t %s\n", val, strerror(errno));
  bytesRead = fread(directoryListing, sizeof(char), sizeof(directoryListing), fp);
  int len;

  printf("directory listing:\n %s\n", directoryListing);
  len = send_buffer(clientSocket, directoryListing, sizeof(directoryListing));

  printf("\n");

  printf("total bytes: %d\n", totalBytesRead);


  pclose(fp);

  return;

}
