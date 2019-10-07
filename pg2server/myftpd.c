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

// TODO: spell check this whole bitch

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#define MAX_BUFFER_SIZE 4096//4096
#define EMPTY_DIRECTORY_SIZE 6
#define EMPTY_FILE_SIZE 0
#define MD5SUM_LENGTH 32
#define MIN(a,b) (((a)<(b))?(a):(b))

void list_handler(int);
void mkdr_handler(int);
void rmdr_handler(int);
void chdr_handler(int);
void crfl_handler(int);
void rmfl_handler(int);
void dnld_handler(int);
void upld_handler(int);

// int send_short(short value, int socket) {
//     int len;
//     short temp = htons(value);
//     if ((len = write(socket, &temp, sizeof(temp))) == -1) {
//         perror("ERROR: Client Send");
//         exit(1);
//     }
//
//     return len;
// }

static long getMicrotime() {
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

int receive_short(int s) {
  printf("IN SHORT FUNCTION\n");
  char buffer[MAX_BUFFER_SIZE];
  int len;
  bzero(buffer, sizeof(buffer));
  printf("Size we are waiting to receive %d\n", sizeof(short));
  if ((len = read(s, buffer, sizeof(short))) == -1) {
      perror("ERROR: Client Receive");
      exit(1);
  }

  short temp = ntohs(*buffer);
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
    bzero(buffer, sizeof(buffer));
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

    return len;
}

int receive_int(int s) {
    printf("IN FUNCTION\n");
    int buffer;
    int len;
    // bzero(&buffer, sizeof(buffer));
    if ((len = read(s, &buffer, sizeof(buffer))) == -1) {
        perror("ERROR: Client Receive");
        exit(1);
    }

    int temp = ntohl(buffer);
    printf("temp val: %d\n", temp);
    return temp;
}

int main(int argc, char* argv[]) {


    // signal (SIGINT, my_handler);

    int port = 41030;
    // int port = 41015;
    // int port = 41002;


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

    printf("Listened\n");
    printf("Awaiting connection\n");
    int clientSocket;
    if((clientSocket = accept(sockfd, (struct sockaddr *) &clientAddr, &clientAddrLen)) < 0){
      fprintf(stderr, "%s: Failed to accept: %s\n", argv[0], strerror(errno));
      exit(-1);
    }
    printf("accepted a connection\n");

    // Receive first client operation
    printf("Receiving client operation (oustside loop)\n");
    char buf[MAX_BUFFER_SIZE];
    memset(buf, 0, sizeof(buf));
    socklen_t addr_len = sizeof(clientAddr);
    int n_received = receive_buffer(clientSocket, buf, 4);
    printf("Received client operation: %d, %s\n", n_received, buf);

    while (strcmp(buf, "QUIT") != 0 && n_received >= 0) {
        printf("beginning processing while loop\n");

      // DNLD: Download
      if (strcmp(buf, "DNLD") == 0) {
        printf("Received DNLD command\n");
        dnld_handler(clientSocket);
      }
      // UPLD: Upload
      else if (strcmp(buf, "UPLD") == 0) {
        printf("Received UPLD command\n");
        upld_handler(clientSocket);
      }
      // LIST: List
      else if (strcmp(buf, "LIST") == 0) {
        printf("Received LIST command\n");
        list_handler(clientSocket);
      }
      // MKDR: Make Directory
      else if (strcmp(buf, "MKDR") == 0) {
        printf("Received MKDR command\n");
        mkdr_handler(clientSocket);
      }
      // RMDR: Remove Directory
      else if (strcmp(buf, "RMDR") == 0) {
        printf("Received RMDR command\n");
        rmdr_handler(clientSocket);
      }
      // CHDR: Change Directory
      else if (strcmp(buf, "CHDR") == 0) {
        printf("Received CHDR command\n");
        chdr_handler(clientSocket);
      }
      // CRFL: Create File
      else if (strcmp(buf, "CRFL") == 0) {
        printf("Received CRFL command\n");
        crfl_handler(clientSocket);
      }
      // RMFL: Remove File
      else if (strcmp(buf, "RMFL") == 0) {
        printf("Received RMFL command\n");
        rmfl_handler(clientSocket);
      }
      // Default case
      else {
        printf("Received unknown command\n");
        // TODO: Handle this
      }

      printf("Finished processing command, waiting for another operation\n");
      memset(buf, 0, sizeof(buf));
      int n_received = receive_buffer(clientSocket, buf, 4);
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
void list_handler(int clientSocket){
  char buffer[MAX_BUFFER_SIZE];
  char* listing;
  int totalBytesRead = 0;
  int bytesRead;
  int cnt = 0;
  FILE* fp;

  bzero(buffer, sizeof(buffer));

  // Obtain list of directory
  fp = popen("ls -l", "r");

  // Determine size of directory listing
  while((bytesRead = fread(buffer, sizeof(char), sizeof(buffer)-1, fp)) > 0) {
    totalBytesRead += bytesRead;
    bzero(buffer, sizeof(buffer));
  }
  bzero(buffer, sizeof(buffer));


  // Send size of directory
  send_int(totalBytesRead, clientSocket);

  // Read directory listing
  char directoryListing[totalBytesRead];
  fp = popen("ls -l", "r");
  bytesRead = fread(directoryListing, sizeof(char), sizeof(directoryListing), fp);
  int len;

  len = send_buffer(clientSocket, directoryListing, sizeof(directoryListing));

  pclose(fp);

  return;

}

void mkdr_handler(int clientSocket) {
    // Prepare to recieve directory
    char dirBuffer[MAX_BUFFER_SIZE];
    bzero((char*)&dirBuffer, sizeof(dirBuffer));

    // Recieve size and directory name
    int directory_size = receive_int(clientSocket);
    int bytesReceived = receive_buffer(clientSocket, dirBuffer, directory_size);

    struct stat st;

    // Check if directory already exists
    if (stat(dirBuffer, &st) == 0 && S_ISDIR(st.st_mode)) {
        int sent = send_int(-2, clientSocket);
    }
    else {
        // Create directory
        if (mkdir(dirBuffer, 0777) == -1) {
            int sent = send_int(-1, clientSocket);
        }
        else {
            int sent = send_int(1, clientSocket);
        }
    }
}

void rmdr_handler(int clientSocket) {
    // Prepare to recieve directory
    char dirBuffer[MAX_BUFFER_SIZE];
    bzero((char*)&dirBuffer, sizeof(dirBuffer));

    // Recieve size and directory name
    int directory_size = receive_int(clientSocket);
    int bytesReceived = receive_buffer(clientSocket, dirBuffer, directory_size);

    struct stat st;

    // Check if directory already exists
    if (stat(dirBuffer, &st) == 0 && S_ISDIR(st.st_mode)) {
        if (st.st_size == EMPTY_DIRECTORY_SIZE) {
            int sent = send_int(1, clientSocket);

            char confBuffer[5];
            bzero((char*)&confBuffer, sizeof(confBuffer));
            int received = receive_buffer(clientSocket, confBuffer, sizeof(confBuffer));
            if (strcmp(confBuffer, "Yes") == 0) {
                int status = rmdir(dirBuffer);
                if (status == 0) {
                  sent = send_int(1, clientSocket);
                }
                else {
                  sent = send_int(-1, clientSocket);
                }
            }
        } else {
          int sent = send_int(-2, clientSocket);
        }
    }
    else {
      printf("Directory DNE\n");
        // server sends negative confirmation back to the client
        int sent = send_int(-1, clientSocket);
    }
}

void chdr_handler(int clientSocket) {
    int size = receive_int(clientSocket);
    char nameBuffer[MAX_BUFFER_SIZE];
    bzero(nameBuffer, sizeof(nameBuffer));
    int bytesReceived = receive_buffer(clientSocket, nameBuffer, size);

    int status = chdir(nameBuffer);
    if (status < 0) {
      if (errno == ENOENT) {
        int sent = send_int(-2, clientSocket);
      }
      else {
        int sent = send_int(-1, clientSocket);
      }
    } else {
      int sent = send_int(1, clientSocket);
    }

}

void crfl_handler(int clientSocket){
  // Prepare to recieve file
  char fileBuffer[MAX_BUFFER_SIZE];
  bzero((char*)&fileBuffer, sizeof(fileBuffer));

  // Recieve size and file name
  int file_size = receive_int(clientSocket);
  int bytesReceived = receive_buffer(clientSocket, fileBuffer, file_size);

  struct stat st;

  // Check if file already exists
  if (stat(fileBuffer, &st) != 0) { // File DNE
      // Create file
      FILE* fp;
      fp = fopen(fileBuffer, "w");
      fclose(fp);
      int sent = send_int(1, clientSocket);
  }
  else { // File exists
    int sent = send_int(-1, clientSocket);
  }
}

void rmfl_handler(int clientSocket){
  // Prepare to recieve file
  char fileBuffer[MAX_BUFFER_SIZE];
  bzero((char*)&fileBuffer, sizeof(fileBuffer));

  // Recieve size and file name
  int file_size = receive_int(clientSocket);
  int bytesReceived = receive_buffer(clientSocket, fileBuffer, file_size);

  struct stat st;

  // Check if file already exists
  if (stat(fileBuffer, &st) == 0 && S_ISREG(st.st_mode)) {
    int sent = send_int(1, clientSocket);
    char confBuffer[5];
    bzero((char*)&confBuffer, sizeof(confBuffer));
    int received = receive_buffer(clientSocket, confBuffer, sizeof(confBuffer));
    if (strcmp(confBuffer, "Yes") == 0) {
        int status = remove(fileBuffer);
        if (status == 0) {
          sent = send_int(1, clientSocket);
        }
        else {
          sent = send_int(-1, clientSocket);
        }
    }
  }
  else {
      // server sends negative confirmation back to the client
    int sent = send_int(-1, clientSocket);
  }
}

void dnld_handler(int clientSocket){
  // Prepare to receive file name and file name size
  char fileBuffer[MAX_BUFFER_SIZE + 1];
  bzero((char*)&fileBuffer, sizeof(fileBuffer));

  // Recieve file name size and file name
  int file_name_size = receive_int(clientSocket);
  int bytesReceived = receive_buffer(clientSocket, fileBuffer, file_name_size);

  struct stat st;

  // Check if file already exists
  if (stat(fileBuffer, &st) == 0 && S_ISREG(st.st_mode)) {
    // send file size
    int file_size = st.st_size;
    int sent_size = send_int(file_size, clientSocket);

    // send md5sum of file
    char sum[MD5SUM_LENGTH + 1]; // buffer to hold actual sum
    char cmd[7 + strlen(fileBuffer) + 1]; // buffer to hold command for linux command
    sprintf(cmd, "md5sum %s", fileBuffer); // put the actual command string in its buffer
    FILE *p = popen(cmd, "r");
    if (p == NULL){
      printf("ERROR: Error calculating md5sum\n");
      // TODO: how should we return from here? maybe we just dont error check
    }
    // fetch the results of the command
    int i, ch;
    for (i = 0; i < 32 && isxdigit(ch = fgetc(p)); i++) {
        sum[i] = ch;
    }
    sum[MD5SUM_LENGTH] = '\0';
    pclose(p);

    // send the actual md5sum
    int sent_md5sum = send_buffer(clientSocket, sum, MD5SUM_LENGTH);

    // send the actual file
    FILE *fp = fopen(fileBuffer, "r");
    char dnldBuffer[MAX_BUFFER_SIZE];
    bzero((char*)&dnldBuffer, sizeof(dnldBuffer));

    int sent_file_bytes = 0;

    // send file in chunks of MAX_BUFFER_SIZE
    while(sent_file_bytes < file_size){
      int bytes_read = fread(dnldBuffer, sizeof(char), MAX_BUFFER_SIZE, fp);
      printf("Bytes read: %d, chunk of file read: %s\n\n", bytes_read, dnldBuffer);
      int sent = send_buffer(clientSocket, dnldBuffer, bytes_read);
      sent_file_bytes += sent;
      bzero((char*)&dnldBuffer, sizeof(dnldBuffer));
    }
  }
  else {
    // server sends negative confirmation (as the file size) back to the client
    int sent = send_int(-1, clientSocket);
  }
}

void upld_handler(int clientSocket){
  // Prepare to recieve file name and file name size
  char fileBuffer[MAX_BUFFER_SIZE];
  bzero((char*)&fileBuffer, sizeof(fileBuffer));

  // Recieve file name size and file name
  int file_name_size = receive_int(clientSocket);
  int bytesReceived = receive_buffer(clientSocket, fileBuffer, file_name_size);

  printf("Received file name and size.\n");

  // Acknowledge to client that server is ready to receive
  int ready = send_int(1, clientSocket);
  printf("Sent acknowledgment (1) that server is ready to receive.\n");

  // Receive file size
  int file_size = receive_int(clientSocket);

	// Create file
	FILE *fp = fopen(fileBuffer, "w");

	if (fp == NULL) {
		printf("ERROR: Failed to create file.\n");
		// TODO: what to do with this failure? server won't know this occured and could be left sending/reading
	} else {

    printf("Receiving data...\n");
		// Recieve file data
		int receivedBytes = 0;
		char buffer[MAX_BUFFER_SIZE + 1];
    long time_start = getMicrotime();
		while(receivedBytes < file_size) {
			bzero(buffer, sizeof(buffer));
			// TODO: Bailey: START TIMER FOR THROUGHPUT HERE (accumulate each loop)
			// there is a hint in the instructions on how to do this
			receivedBytes += receive_buffer(clientSocket, buffer, MIN(MAX_BUFFER_SIZE,file_size-receivedBytes));
			// TODO: Bailey: STOP TIMER FOR THROUGHPUT HERE AND
			// use receivedBytes for size in calculation
			//
			// the reason for this placement is to time only the recieveing, and not the disk writing
      printf("Received %d bytes.\n", receivedBytes);
			fwrite(buffer, sizeof(char), strlen(buffer), fp);
		}
    fclose(fp);
    long time_end = getMicrotime();

    // Calculation of Throughput
    long diff = time_end - time_start;
    double diff_s = (double)diff * 0.000001;
    double megabytes = (double)receivedBytes * 0.000001;
    double throughput = megabytes / diff_s;

    fflush(fp);

    printf("Calculating md5sum.\n");
		// Calculate md5hash
		char calculatedMd5sum[MD5SUM_LENGTH + 1]; // buffer to hold actual sum
		bzero(calculatedMd5sum, sizeof(calculatedMd5sum));
		char cmd[7 + strlen(fileBuffer) + 1]; // buffer to hold command for linux command
		sprintf(cmd, "md5sum %s", fileBuffer); // put the actual command string in its buffer

    FILE *p = popen(cmd, "r");
		if (p == NULL){
			printf("ERROR: Error calculating md5sum\n");
			// TODO: how should we return from here? maybe we just dont error check
		}
		// fetch the results of the command
		int i, ch;
		for (i = 0; i < 32 && isxdigit(ch = fgetc(p)); i++) {
				calculatedMd5sum[i] = ch;
		}
		calculatedMd5sum[MD5SUM_LENGTH] = '\0';
		pclose(p);

    printf("Calculated md5sum.\n");

    // Send throughput results
    printf("Sending throughput.\n");
    if ((write(clientSocket, (const char*)&throughput, sizeof(throughput))) == -1) {
      perror("ERROR: Server Sending\n");
      exit(1);
    }
    //send time results
    printf("Sending time.\n");
    if ((write(clientSocket, (const char *)&diff_s, sizeof(diff_s))) == -1)
    {
      perror("ERROR: Server Sending\n");
      exit(1);
    }

    // Send md5sum
    printf("Sending md5sum.\n");
    int sent_md5sum = send_buffer(clientSocket, calculatedMd5sum, MD5SUM_LENGTH);
	}
}
