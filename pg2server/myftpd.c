/*
 * CSE30264: Programming Assignment 2
 * Chris Foley (cfoley)
 * Catherine Markley (cmarkley)
 * Bailey Blum (bblum1)
 *
 * myftpd.c (Server)
 *
 * Server for our PA02 TCP assignemnt. The server opens a port and goes into "wait for connection" state.
 * Server accepts a connection from a client. The server then waits for commands from the client (DNLD: Download,
 * UPLD: Upload, LIST: List, MKDR: Make Directory, RMDR: Remove Directory, CHDR: Change Directory,
 * CRFL: Create File, RMFL: Remove File, QUIT: Quit). It then handles each operation appropriately by calling
 * a handler for each opeartion. The server loops as to allow multiple operations to be called. It does not allow multiple clients.
 */

/* TODO LIST
   - handle errors better (not just exit. part of the project rubric. ASK TA)
   - catch ^C (ASK TA)
   - write what does getMicrotime() do in comment
   - spell check entire file
   - timing and throughput stuff
   - in UPLD, check if the file exists before uploading it/opening it
 */

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
#include <dirent.h>

#define MAX_BUFFER_SIZE 4096
#define EMPTY_DIRECTORY_ENTRIES_COUNT 2
#define EMPTY_FILE_SIZE 0
#define MD5SUM_LENGTH 32
#define MIN(a,b) (((a)<(b)) ? (a) : (b))

void list_handler(int);
void mkdr_handler(int);
void rmdr_handler(int);
void chdr_handler(int);
void crfl_handler(int);
void rmfl_handler(int);
void dnld_handler(int);
void upld_handler(int);

/*
 * getMicrotime()
 *
 * TODO: what does this do
 */
static long getMicrotime() {
        struct timeval currentTime;
        gettimeofday(&currentTime, NULL);
        return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

/*
 * send_buffer(clientSocket, buffer, size)
 *
 * Send the data within buffer parameter to the TCP connection on clientSocket
 */
int send_buffer(int clientSocket, char* buffer, int size) {
        int len;
        if ((len = write(clientSocket, buffer, size)) == -1) {
                perror("ERROR: Client Send\n");
                exit(1);
        }
        return len;
}

/*
 * receive_buffer(clientSocket, buffer, size)
 *
 * Receives a buffer of data from the TCP connection on clientSocket
 */
int receive_buffer(int clientSocket, char* buffer, int size) {
        int len;
        bzero(buffer, sizeof(buffer));
        if ((len = read(clientSocket, buffer, size)) == -1) {
                perror("ERROR: Receive Error!");
                exit(1);
        }
        return len;
}

/*
 * send_int(value, clientSocket)
 *
 * Sends an int over the TCP connection on clientSocket
 */
int send_int(int value, int socket) {
        int len;
        uint32_t temp = htonl(value);
        if ((len = write(socket, &temp, sizeof(temp))) == -1) {
                perror("ERROR: Client Send");
                exit(1);
        }

        return len;
}

/*
 * receive_int(clientSocket)
 *
 * Receives an int over the TCP connection on clientSocket
 */
int receive_int(int s) {
        int buffer;
        int len;
        // bzero(&buffer, sizeof(buffer));
        if ((len = read(s, &buffer, sizeof(buffer))) == -1) {
                perror("ERROR: Client Receive");
                exit(1);
        }

        return ntohl(buffer);
}

int main(int argc, char* argv[]) {

        if(argc != 2) {
                printf("%s: Incorrect usage.\n Usage: %s PORT \n", argv[0], argv[0]);
                exit(1);
        }

        int port = atoi(argv[1]);

        // signal (SIGINT, my_handler); // TODO this shit. ask TA

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

        if (bind(sockfd, (const struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0 ) {
                fprintf(stderr, "%s: Failed to bind socket: %s\n", argv[0], strerror(errno));
                exit(-1);
        }

        if((listen(sockfd, SOMAXCONN)) < 0) {
                fprintf(stderr, "%s: Failed to listen: %s\n", argv[0], strerror(errno));
                exit(-1);
        }

        printf("Waiting for connection on port %d\n", port);
        int clientSocket;
        if((clientSocket = accept(sockfd, (struct sockaddr *) &clientAddr, &clientAddrLen)) < 0) {
                fprintf(stderr, "%s: Failed to accept: %s\n", argv[0], strerror(errno));
                exit(-1);
        }
        printf("Connection established\n");

        // Receive first client operation
        char buf[MAX_BUFFER_SIZE];
        memset(buf, 0, sizeof(buf));
        socklen_t addr_len = sizeof(clientAddr);
        int n_received = receive_buffer(clientSocket, buf, 4);

        while (strcmp(buf, "QUIT") != 0 && n_received >= 0) {
                // DNLD: Download
                if (strcmp(buf, "DNLD") == 0) {
                        dnld_handler(clientSocket);
                }
                // UPLD: Upload
                else if (strcmp(buf, "UPLD") == 0) {
                        upld_handler(clientSocket);
                }
                // LIST: List
                else if (strcmp(buf, "LIST") == 0) {
                        list_handler(clientSocket);
                }
                // MKDR: Make Directory
                else if (strcmp(buf, "MKDR") == 0) {
                        mkdr_handler(clientSocket);
                }
                // RMDR: Remove Directory
                else if (strcmp(buf, "RMDR") == 0) {
                        rmdr_handler(clientSocket);
                }
                // CHDR: Change Directory
                else if (strcmp(buf, "CHDR") == 0) {
                        chdr_handler(clientSocket);
                }
                // CRFL: Create File
                else if (strcmp(buf, "CRFL") == 0) {
                        crfl_handler(clientSocket);
                }
                // RMFL: Remove File
                else if (strcmp(buf, "RMFL") == 0) {
                        rmfl_handler(clientSocket);
                }
                // Default case
                else {
                        printf("Received unknown command\n");
                        // TODO: Handle this
                }

                memset(buf, 0, sizeof(buf));
                int n_received = receive_buffer(clientSocket, buf, 4);
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
        char buffer[MAX_BUFFER_SIZE]; // change back to Max buffer size
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

                // Count entries
                DIR *openedDirectory = opendir(dirBuffer);
                int entries = 0;
                struct dirent *d;
                while((d = readdir(openedDirectory)) != NULL) {
                        entries++;
                }

                if (entries == EMPTY_DIRECTORY_ENTRIES_COUNT) {
                        int sent = send_int(1, clientSocket);

                        char confBuffer[5];
                        bzero((char*)&confBuffer, sizeof(confBuffer));
                        int received = receive_buffer(clientSocket, confBuffer, sizeof(confBuffer));
                        if (strcmp(confBuffer, "Yes") == 0) {
                                int status = rmdir(dirBuffer);
                                if (status == 0) {
                                        sent = send_int(1, clientSocket);
                                } else {
                                        sent = send_int(-1, clientSocket);
                                }
                        }
                } else {
                        int sent = send_int(-2, clientSocket);
                }
        } else {
                // Directory DNE
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
                if (p == NULL) {
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
                while(sent_file_bytes < file_size) {
                        int bytes_read = fread(dnldBuffer, sizeof(char), MAX_BUFFER_SIZE, fp);
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

        // Acknowledge to client that server is ready to receive
        int ready = send_int(1, clientSocket);

        // Receive file size
        int file_size = receive_int(clientSocket);

        // Create file
        FILE *fp = fopen(fileBuffer, "w");

        if (fp == NULL) {
                printf("ERROR: Failed to create file.\n");
                // TODO: what to do with this failure? server won't know this occured and could be left sending/reading
        } else {
                // Recieve file data
                int totalReceivedBytes = 0;
                int receivedBytes = 0;
                char buffer[MAX_BUFFER_SIZE + 1];
                long time_start = getMicrotime();
                while(totalReceivedBytes < file_size) {
                        bzero(buffer, sizeof(buffer));
                        // TODO: Bailey: START TIMER FOR THROUGHPUT HERE (accumulate each loop)
                        // there is a hint in the instructions on how to do this
                        receivedBytes = receive_buffer(clientSocket, buffer, MIN(MAX_BUFFER_SIZE,file_size-receivedBytes));

                        totalReceivedBytes += receivedBytes;
                        // TODO: Bailey: STOP TIMER FOR THROUGHPUT HERE AND
                        // use receivedBytes for size in calculation
                        //
                        // the reason for this placement is to time only the recieveing, and not the disk writing

                        if (fwrite(buffer, sizeof(char), receivedBytes, fp) < receivedBytes) {
                                // TODO: how to handle this failure. server won't know this occured and could be left sending/reading
                                printf("ERROR: Writing to file error!\n");
                        }
                }
                fclose(fp);
                long time_end = getMicrotime();

                // Calculation of Throughput
                long diff = time_end - time_start;
                double diff_s = (double)diff * 0.000001;
                double megabytes = (double) totalReceivedBytes * 0.000001;
                double throughput = megabytes / diff_s;

                //fflush(fp);

                // Calculate md5hash
                char calculatedMd5sum[MD5SUM_LENGTH + 1]; // buffer to hold actual sum
                bzero(calculatedMd5sum, sizeof(calculatedMd5sum));
                char cmd[7 + strlen(fileBuffer) + 1]; // buffer to hold command for linux command
                sprintf(cmd, "md5sum %s", fileBuffer); // put the actual command string in its buffer

                FILE *p = popen(cmd, "r");
                if (p == NULL) {
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

                // Send throughput results
                if ((write(clientSocket, (const char*)&throughput, sizeof(throughput))) == -1) {
                        perror("ERROR: Server Sending\n");
                        exit(1);
                }
                //send time results
                if ((write(clientSocket, (const char *)&diff_s, sizeof(diff_s))) == -1)
                {
                        perror("ERROR: Server Sending\n");
                        exit(1);
                }

                // Send md5sum
                int sent_md5sum = send_buffer(clientSocket, calculatedMd5sum, MD5SUM_LENGTH);
        }
}
