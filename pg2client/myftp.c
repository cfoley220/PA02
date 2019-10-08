/*
 * CSE30264: Programming Assignment 2
 * Chris Foley (cfoley)
 * Catherine Markley (cmarkley)
 * Bailey Blum (bblum1)
 *
 * myftp.c (Client)
 *
 * This is our client for our PA02 TCP assignment. The client begins by establishing a connection
 * to the already-running server. It then accepts operations from the user (DNLD: Download,
 * UPLD: Upload, LIST: List, MKDR: Make Directory, RMDR: Remove Directory, CHDR: Change Directory,
 * CRFL: Create File, RMFL: Remove File, QUIT: Quit) and handles them appropriately via handler
 * methods. The client loops to allow the user to enter multiple operations. It loops until the
 * user enters QUIT.
 */

/* TODO LIST
   - catch ^C and send message to server to shut it down  (ASK TA)
   - handle errors better (not just exit. part of the project rubric. ASK TA)
   - write what does getMicrotime() do in comment
   - timing/throughput gives 0 and nan results most times. unsure why
   - spell check entire file

   non-file specific items
   - make README
   - test on multiple student machines
   - create zipped file
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/stat.h>

#define MAX_BUFFER_SIZE 4096
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
																perror("Client Send\n");
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
																perror("Client Receive!");
																exit(1);
								}
								return len;
}

/*
 * send_int(value, clientSocket)
 *
 * Sends an int over the TCP connection on clientSocket
 */
int send_int(int value, int clientSocket) {
								int len;
								uint32_t temp = htonl(value);
								if ((len = write(clientSocket, &temp, sizeof(temp))) == -1) {
																perror("Client Send");
																exit(1);
								}

								return len;
}

/*
 * receive_int(clientSocket)
 *
 * Receives an int over the TCP connection on clientSocket
 */
int receive_int(int clientSocket) {
								int buffer;
								int len;
								bzero(&buffer, sizeof(buffer));
								if ((len = read(clientSocket, &buffer, sizeof(buffer))) == -1) {
																perror("Client Receive Error");
																exit(1);
								}

								int temp = ntohl(buffer);
								return temp;
}

int main(int argc, char *argv[]) {

								// parse command line arguments
								if(argc != 3) {
																printf("%s: Incorrect usage.\n Usage: %s ADDR PORT \n", argv[0], argv[0]);
																exit(1);
								}

								int port = atoi(argv[2]);

								// Create socket
								int clientSocket;
								if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0 )  {
																printf("Client: Error creating socket\n");
																exit(0);
								}

								struct sockaddr_in servaddr;

								memset(&servaddr, 0, sizeof(servaddr));

								servaddr.sin_family = AF_INET;
								servaddr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr*) gethostbyname(argv[1])->h_addr_list[0])));
								servaddr.sin_port = htons(port);

								// connect
								printf("Connection to %s on port %d\n", argv[1], port);
								if (connect(clientSocket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
																printf("Client: Error connecting to server: %s\n", strerror(errno));
																exit(0);
								}
								printf("Connection established\n");

								char *userInput;

								while(1) {

																// Prompt the user for an Operation
																printf("> ");
																fgets(userInput, 50, stdin);

																// Get rid of the endline character
																userInput[strlen(userInput)-1] = '\0';

																// Get operation out of user input
																char * operation = strtok(userInput, " \t\n");

																if (strlen(operation) != 4) {
																								printf("Operation must be 4 characters.\n");
																								continue;
																}

																int sent = send_buffer(clientSocket, operation, strlen(operation));

																// DNLD: Download
																if (strcmp(operation, "DNLD") == 0) {
																								dnld_handler(clientSocket);

																}
																// UPLD: Upload
																else if (strcmp(operation, "UPLD") == 0) {
																								upld_handler(clientSocket);
																}
																// LIST: List
																else if (strcmp(operation, "LIST") == 0) {
																								list_handler(clientSocket);
																}
																// MKDR: Make Directories
																else if (strcmp(operation, "MKDR") == 0) {
																								mkdr_handler(clientSocket);
																}
																// RMDR: Remove Directory
																else if (strcmp(operation, "RMDR") == 0) {
																								rmdr_handler(clientSocket);
																}
																// CHDR: Change Directory
																else if (strcmp(operation, "CHDR") == 0) {
																								chdr_handler(clientSocket);
																}
																// CRFL: Create File
																else if (strcmp(operation, "CRFL") == 0) {
																								crfl_handler(clientSocket);
																}
																// RMFL: Remove File
																else if (strcmp(operation, "RMFL") == 0) {
																								rmfl_handler(clientSocket);
																}
																// QUIT: Quit
																else if (strcmp(operation, "QUIT") == 0) {
																								break;
																}
																// Default: invalid
																else {
																								printf("Invalid command.\n");
																}

								}

								close(clientSocket);
}

void list_handler(int clientSocket){
								char messageBuffer[MAX_BUFFER_SIZE];
								char dirList[MAX_BUFFER_SIZE];
								bzero(messageBuffer, sizeof(messageBuffer));
								bzero(dirList, sizeof(dirList));

								// Get size of directory
								int directorySize = receive_int(clientSocket);

								// Recieve listing
								int bytesRecvd = 0;
								while(bytesRecvd < directorySize) {
																memset(&messageBuffer, 0, sizeof(messageBuffer));
																bytesRecvd += receive_buffer(clientSocket, messageBuffer, sizeof(messageBuffer)-1);
																strcat(dirList, messageBuffer);
																bzero(messageBuffer, sizeof(messageBuffer));

																// Escape reading loop if its empty or error
																if (bytesRecvd <= 0) {
																								break;
																}
								}
								bzero(messageBuffer, sizeof(messageBuffer));
								printf("%s", dirList);
}

void mkdr_handler(int clientSocket) {
								// Get directory name and size
								char* directory = strtok(NULL, " ");

								if (directory == NULL) {
																printf("Improper use of command. Need to include a directory\n");
																// TODO: handle error. should we exit?
								}

								// Send length of directory name (int)
								int received = send_int(strlen(directory), clientSocket);

								// Send directory name (string)
								send_buffer(clientSocket, directory, strlen(directory));

								// Recieve status update and inform user
								int status = receive_int(clientSocket);

								if (status == -2) {
																printf("The directory already exists on the server.\n");
								}
								else if (status == -1) {
																printf("Making directory.\n");
								}
								else {
																printf("The directory was successfully made!\n");
								}

}

void rmdr_handler(int clientSocket){
								// Get directory name and size
								char* directory = strtok(NULL, " ");

								if (directory == NULL) {
																printf("Improper use of command. Need to include a file name\n");
																// TODO: handle error. should we exit?
								}

								// Send length of directory name (int)
								int received = send_int(strlen(directory), clientSocket);

								// Send directory name (string)
								send_buffer(clientSocket, directory, strlen(directory));

								// Recieve status update and inform user
								int status = receive_int(clientSocket);

								if (status == -1) {
																// Directory DNE
																printf("The directory does not exist on server.\n");
								}
								else if (status == -2) {
																// Directory not empty
																printf("The directory is not empty.\n");
								}
								else if (status == 1) {
																// Directory can be deleted

																// Get user confirmation
																printf("Are you sure you would like to delete %s? [Yes/No]\n", directory);

																char userConfirmation[5];
																bzero(userConfirmation, sizeof(userConfirmation));
																fgets(userConfirmation, 5, stdin);

																// Get rid of the endline character
																userConfirmation[strlen(userConfirmation)-1] = '\0';
																// Ensure last two characters are null
																userConfirmation[4] = '\0';
																userConfirmation[5] = '\0';

																// Send confirmation
																send_buffer(clientSocket, userConfirmation, 5);


																if (strcmp(userConfirmation, "Yes") == 0) {
																								// User wants to delete
																								int status = receive_int(clientSocket);

																								if (status > 0) {
																																printf("Directory deleted\n");
																								} else if (status < 0) {
																																printf("Failed to delete directory\n");
																								} else {
																																printf("Unknown status received for RMDR from server.\n");
																								}
																} else {
																								// User does not want to delete
																								printf("Delete abandoned by the user!\n");
																}

								} else {
																printf("Unknown status received for RMDR from server.\n");
								}
}

void chdr_handler(int clientSocket){
								// Get directory name and size
								char* directory = strtok(NULL, " ");

								if (directory == NULL) {
																printf("Improper use of command. Need to include a file name\n");
																// TODO: handle error. should we exit?
								}

								// Send length of directory name (int)
								int received = send_int(strlen(directory), clientSocket);

								// Send directory name (string)
								send_buffer(clientSocket, directory, strlen(directory));

								// Recieve status update and inform user
								int status = receive_int(clientSocket);

								if (status > 0) {
																printf("Changed current directory\n");
								} else if (status == -1) {
																printf("Error in changing directory\n");
								} else if (status == -2) {
																printf("The directory does not exist on server.\n");
								} else {
																printf("Unknown status received for CHDR from server.\n");
								}
}

void crfl_handler(int clientSocket) {
								// Get file name and size
								char* filename = strtok(NULL, " ");

								if (filename == NULL) {
																printf("Improper use of command. Need to include a file name\n");
																// TODO: handle error. should we exit?
								}

								// Send length of directory name (int)
								send_int(strlen(filename), clientSocket);

								// Send filename (string)
								send_buffer(clientSocket, filename, strlen(filename));

								// Recieve status update and inform user
								int status = receive_int(clientSocket);

								if (status > 0) {
																printf("The file was successfully created.\n");
								} else if (status < 0) {
																printf("The file already exists.\n");
								} else {
																printf("Unknown status received for CRFL from server.\n");
								}
}

void rmfl_handler(int clientSocket) {
								// Get file name and size
								char* filename = strtok(NULL, " ");

								if (filename == NULL) {
																printf("Improper use of command. Need to include a file name\n");
																// TODO: handle error. should we exit?
								}


								// Send length of directory name (int)
								int received = send_int(strlen(filename), clientSocket);

								// Send filename (string)
								send_buffer(clientSocket, filename, strlen(filename));

								// Receive status update and inform user
								int status = receive_int(clientSocket);

								if (status < 0) {
																printf("The file does not exist on server.\n");
								} else if (status > 0) {
																// Get user confirmation
																printf("Are you sure you would like to delete %s? [Yes/No]\n", filename);

																char userConfirmation[5];
																bzero(userConfirmation, sizeof(userConfirmation));
																fgets(userConfirmation, 5, stdin);

																// Get rid of the endline character
																userConfirmation[strlen(userConfirmation)-1] = '\0';
																// Ensure last two characters are null
																userConfirmation[4] = '\0';
																userConfirmation[5] = '\0';

																// Send confirmation
																send_buffer(clientSocket, userConfirmation, 5);

																if (strcmp(userConfirmation, "Yes") == 0) {
																								// User wants to delete
																								int status = receive_int(clientSocket);

																								if (status > 0) {
																																printf("File deleted.\n");
																								} else if (status < 0) {
																																printf("Failed to delete file\n");
																								} else {
																																printf("Unknown status received for RMFL from server.\n");
																								}
																} else {
																								// User does not want to delete
																								printf("Delete abandoned by the user!\n");
																}

								} else {
																printf("Unknown status received for RMFL from server.\n");
								}
}

void dnld_handler(int clientSocket){
								// Get file name and size
								char* filename = strtok(NULL, " ");
								//printf("in download\n");

								if (filename == NULL) {
																printf("Improper use of command. Need to include a file name\n");
																// TODO: handle error. should we exit?

								}

								// Send length of filename (int)
								send_int(strlen(filename), clientSocket);

								// Send filename (string)
								send_buffer(clientSocket, filename, strlen(filename));

								// Recieve status update and inform user
								int status = receive_int(clientSocket);
								printf("    \n"); // TODO: wow.

								if (status == -1) {
																printf("File %s does not exist on server.\n", filename);
																return;
								} else {
																int filesize = status;

																// Receive md5sum
																char receiveMd5sum[MD5SUM_LENGTH + 1];
																bzero(receiveMd5sum, sizeof(receiveMd5sum));

																receive_buffer(clientSocket, receiveMd5sum, MD5SUM_LENGTH);

																// Create file
																FILE *fp = fopen(filename, "w");

																if (fp == NULL) {
																								printf("Failed to create file.\n");
																								// TODO: what to do with this failure? server won't know this occured and could be left sending/reading
																} else {

																								// Recieve file data
																								int totalReceivedBytes = 0;
																								int receivedBytes;
																								char buffer[MAX_BUFFER_SIZE];
																								bzero((char*)&buffer, sizeof(buffer));
																								long time_start = getMicrotime();
																								while(totalReceivedBytes < filesize) {
																																receivedBytes = receive_buffer(clientSocket, buffer, MIN(MAX_BUFFER_SIZE,filesize-receivedBytes));
																																totalReceivedBytes += receivedBytes;
																																// printf("Bytes received: %d, chunk of file received: %s\n\n", receivedBytes, buffer);
																																if (fwrite(buffer, sizeof(char), receivedBytes, fp) < receivedBytes) {
																																								// TODO: how to handle this failure. server won't know this occured and could be left sending/reading
																																								printf("Writing to file error!\n");
																																}
																																bzero((char*)&buffer, sizeof(buffer));
																								}
																								long time_end = getMicrotime();

																								//Calculation of Throughput
																								long diff = time_end - time_start;
																								float diff_s = (float)diff * 0.000001;
																								float megabytes = (float) totalReceivedBytes * 0.000001;
																								float throughput = megabytes / diff_s;

																								fclose(fp);

																								//fflush(fp);

																								// Calculate md5hash
																								char calculatedMd5sum[MD5SUM_LENGTH + 1]; // buffer to hold actual sum
																								bzero(calculatedMd5sum, sizeof(calculatedMd5sum));
																								char cmd[7 + strlen(filename) + 1]; // buffer to hold command for linux command

																								sprintf(cmd, "md5sum %s", filename); // put the actual command string in its buffer

																								FILE *p = popen(cmd, "r");
																								if (p == NULL) {
																																printf("Error calculating md5sum\n");
																																// TODO: how should we return from here? maybe we just dont error check
																								}
																								// fetch the results of the command
																								int i, ch;
																								for (i = 0; i < 32 && isxdigit(ch = fgetc(p)); i++) {
																																calculatedMd5sum[i] = ch;
																								}

																								calculatedMd5sum[MD5SUM_LENGTH] = '\0';

																								pclose(p);

																								// Output data transfer
																								printf("%d bytes transferred in %f seconds: %f MegaBytes\\sec.\n", receivedBytes, diff_s, throughput); //TODO: error here. got nan instead of a number for throughput when downloading medium file

																								// Compare md5hash
																								printf("MD5Hash: %s (%s)\n", calculatedMd5sum, (strcmp(receiveMd5sum, calculatedMd5sum) == 0) ? "matches" : "doesn't match");
																}
								}
}

void upld_handler(int clientSocket){
								// Get file name and size
								char* filename = strtok(NULL, " ");

								if (filename == NULL) {
																printf("Improper use of command. Need to include a file name\n");
																// TODO: handle error. should we exit?

								}

								// Send length of filename (int)
								send_int(strlen(filename), clientSocket);

								// Send filename (string)
								send_buffer(clientSocket, filename, strlen(filename));

								// Wait for server acknowledgment
								receive_int(clientSocket);

								// Check if file already exists
								struct stat st;

								if (stat(filename, &st) == 0 && S_ISREG(st.st_mode)) {
																// Send file size
																int fileSize = st.st_size;
																int sent_size = send_int(fileSize, clientSocket);

																// Send the actual file
																FILE *fp = fopen(filename, "r");
																char buffer[MAX_BUFFER_SIZE + 1];
																bzero(buffer, sizeof(buffer));

																// Send file in chunks of MAX_BUFFER_SIZE
																int sentBytes = 0;
																while(sentBytes < fileSize) {
																								int bytesRead = fread(buffer, sizeof(char), MAX_BUFFER_SIZE, fp);
																								sentBytes += send_buffer(clientSocket, buffer, bytesRead);
																								bzero(buffer, sizeof(buffer));
																}

																// Receive throughput
																double throughput;
																int len;
																if ((len = read(clientSocket, &throughput, sizeof(throughput))) == -1) {
																								perror("Client Receive\n"); //TODO: fix this error statement to something descriptive
																								exit(1);
																}

																// Receive time
																double diff_s;
																if ((len = read(clientSocket, &diff_s, sizeof(diff_s))) == -1)
																{
																								perror("Client Receive\n");//TODO: fix this error statement to something descriptive
																								exit(1);
																}

																// Receive md5sum
																char receiveMd5sum[MD5SUM_LENGTH + 1];
																bzero(receiveMd5sum, sizeof(receiveMd5sum));
																receive_buffer(clientSocket, receiveMd5sum, MD5SUM_LENGTH);

																// Calculate md5hash
																char calculatedMd5sum[MD5SUM_LENGTH + 1]; // buffer to hold actual sum
																bzero(calculatedMd5sum, sizeof(calculatedMd5sum));
																char cmd[7 + strlen(filename) + 1]; // buffer to hold command for linux command

																sprintf(cmd, "md5sum %s", filename); // put the actual command string in its buffer

																FILE *p = popen(cmd, "r");
																if (p == NULL) {
																								printf("Error calculating md5sum\n");
																								// TODO: how should we return from here? maybe we just dont error check
																}
																// fetch the results of the command
																int i, ch;
																for (i = 0; i < 32 && isxdigit(ch = fgetc(p)); i++) {
																								calculatedMd5sum[i] = ch;
																}

																calculatedMd5sum[MD5SUM_LENGTH] = '\0';
																printf("Calculated: %s\n", calculatedMd5sum);
																pclose(p);

																if (strcmp(receiveMd5sum, calculatedMd5sum) == 0) {
																								printf("%d bytes transferred in %f seconds: %f Megabytes/sec\n", fileSize, diff_s, throughput);
																								printf("\tMD5Hash: %s (%s)\n", calculatedMd5sum, (strcmp(receiveMd5sum, calculatedMd5sum) == 0) ? "matches" : "doesn't match");
																} else {
																								printf("Transfer failed.\n");
																}


								} else {
																// TODO: WHAT TO DO HERE (file DNE)
								}
								return;
}
