/*
 * CSE30264: Programming Assignment 2
 * Chris Foley (cfoley)
 * Catherine Markley (cmarkley)
 * Bailey Blum (bblum1)
 *
 * myftp.c (Client)
 *
 * TODO: CODE DESCRIPTION
*/

// TODO: go through demo video, and match print statements
// TODO: catch a ^C on the client side and sent a message to the server to shut down
// TODO: change sending name lengths to send_short as required by instructions ):

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

#define MAX_BUFFER_SIZE 64//4096
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
        perror("ERROR: Client Receive!");
        exit(1);
    }
    return len;
}

int send_int(int value, int s) {
	int len;
    int temp = htonl(value);
    if ((len = write(s, &temp, sizeof(temp))) == -1) {
        perror("ERROR: Client Send");
        exit(1);
    }

	return len;
}

int receive_int(int s) {
    int buffer;
    int len;
    bzero(&buffer, sizeof(buffer));
    if ((len = read(s, &buffer, sizeof(buffer))) == -1) {
        perror("ERROR: Client Receive");
        exit(1);
    }

    int temp = ntohl(buffer);
    return temp;
}

int main(int argc, char *argv[]) {

	// socket
	int clientSocket;
	if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0 )  {
		printf("Client: Error creating socket\n");
		exit(0);
	}

  printf("Client: Created socket\n");

  struct sockaddr_in servaddr;

  memset(&servaddr, 0, sizeof(servaddr));

  printf("Client: Got address info\n");

  // TODO: take in server name and port via command line arguements
	servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr*) gethostbyname("student02.cse.nd.edu")->h_addr_list[0]))); // ip addr of student02
  servaddr.sin_port = htons(41030);

	// servaddr.sin_port = htons(41015);
	// servaddr.sin_port = htons(41002);

  // connect
  if (connect(clientSocket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
    printf("Client: Error connecting to server: %s\n", strerror(errno));
    exit(0);
  }

  printf("Client: Connected\n");

	char *userInput;

	while(1) {

		// Prompt the user for an Operation
		printf("Operations:\n\tDNLD: Download,\n\tUPLD: Upload,\n\tLIST: List,\n\tMKDR: Make Directory,\n\tRMDR: Remove Directory,\n\tCHDR: Change Directory,\n\tCRFL: Create File,\n\tRMFL: Remove File,\n\tQUIT: Quit\n\n");
		printf("Enter an operation: ");
		fgets(userInput, 50, stdin);

		// Get rid of the endline character
		userInput[strlen(userInput)-1] = '\0';

    // Get operation out of user input
    char *operation = strtok(userInput, " "); // TODO: regex for any whitespace?
    // TODO: ENSURE THAT OPERATION IS 4 CHARACTERS
    // currently errors out: to reproduce:
    // run server and client
    // on client:
    // send: "MKDIR aa"
    // send: "MKDR aa"
		// TODO: ensure that if an operation is typed, it has the correct arguments
		// ex. "DNLD" with no arguments after will give you a seg fault
		// it should be "DNLD SmallFile.txt"

	  printf("Sending the operation\n");

	  // Send the operation to the server
	  int sent = send_buffer(clientSocket, operation, strlen(operation));
    // TODO: double check that strlen works

	  printf("Sent the operation, %d bytes sent\n", sent);

		// DNLD: Download
		if (strcmp(operation, "DNLD") == 0) {
			printf("Sent DNLD command\n");
			dnld_handler(clientSocket);

		}
		// UPLD: Upload
		else if (strcmp(operation, "UPLD") == 0) {
			printf("Sent UPLD command\n");
			upld_handler(clientSocket);
		}
		// LIST: List
		else if (strcmp(operation, "LIST") == 0) {
			printf("Sent LIST command\n");
      list_handler(clientSocket);
		}
		// MKDR: Make Directories
		else if (strcmp(operation, "MKDR") == 0) {
      printf("Sent MKDR command\n");
			mkdr_handler(clientSocket);
		}
		// RMDR: Remove Directory
		else if (strcmp(operation, "RMDR") == 0) {
			printf("Sent RMDR command\n");
      rmdr_handler(clientSocket);
		}
		// CHDR: Change Directory
		else if (strcmp(operation, "CHDR") == 0) {
			printf("Sent CHDR command\n");
      chdr_handler(clientSocket);
		}
		// CRFL: Create File
		else if (strcmp(operation, "CRFL") == 0) {
			printf("Sent CRFL command\n");
      crfl_handler(clientSocket);
		}
		// RMFL: Remove File
		else if (strcmp(operation, "RMFL") == 0) {
			printf("Sent RMFL command\n");
      rmfl_handler(clientSocket);
		}
    // QUIT: Quit
		else if (strcmp(operation, "QUIT") == 0){
			break;
		}

		else {
			printf("ERROR: Invalid command.\n");
		}

	}

	close(clientSocket);
}

void list_handler(int clientSocket){
  char messageBuffer[MAX_BUFFER_SIZE];

  // Get size of directory
  int directorySize = receive_int(clientSocket);
  //printf("Size of directory: %d\n", directorySize);

  // Recieve listing
  int bytesRecvd = 0;
  while(bytesRecvd < directorySize) {
    memset(&messageBuffer, 0, sizeof(messageBuffer));
    bytesRecvd += receive_buffer(clientSocket, messageBuffer, sizeof(messageBuffer));
    //printf("Bytes received: %d\n", bytesRecvd);
    //printf("Msg buffer: %s\n", messageBuffer);
    printf("%s\n", messageBuffer);

    // Escape reading loop if its empty or error
    if (bytesRecvd <= 0){
      break;
    }
  }
}

void mkdr_handler(int clientSocket) {
	// Get directory name and size
	char* directory = strtok(NULL, " ");

	// Send length of directory name (short int)
	int received = send_int(strlen(directory), clientSocket);

	// Send directory name (string)
	send_buffer(clientSocket, directory, strlen(directory));

	// Recieve status update and inform user
	int status = receive_int(clientSocket);

	if (status == -2) {
		printf("ERROR: The directory already exists on the server.\n");
	}
	else if (status == -1) {
		printf("ERROR: Making directory.\n");
	}
	else {
		printf("The directory was successfully made!\n");
	}

}

void rmdr_handler(int clientSocket){
  // Get directory name and size
  char* directory = strtok(NULL, " ");

  printf("Directory is: %s\n", directory);

  // Send length of directory name (short int)
  int received = send_int(strlen(directory), clientSocket);

  // Send directory name (string)
  send_buffer(clientSocket, directory, strlen(directory));

  // Recieve status update and inform user
  int status = receive_int(clientSocket);

  printf("Recieved status: %d\n", status);

  if (status == -1) {
    // Directory DNE
    printf("ERROR: The directory does not exist on server.\n");
  }
  else if (status == -2) {
    // Directory not empty
    printf("ERROR: The directory is not empty.\n");
  }
  else if (status == 1){
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
        printf("Waiting for status\n");
        int status = receive_int(clientSocket);
        printf("Recieved status back: %d\n", status);

        if (status > 0) {
          printf("Directory deleted\n");
        } else if (status < 0) {
          printf("Failed to delete directory\n");
        } else {
          printf("ERROR: Unknown status received for RMDR from server.\n");
        }
    } else {
        // User does not want to delete
      printf("Delete abandoned by the user!\n");
    }

  } else {
    printf("ERROR: Unknown status received for RMDR from server.\n");
  }

  printf("ENDING RMDR PART\n");
}

void chdr_handler(int clientSocket){
  // Get directory name and size
  char* directory = strtok(NULL, " ");

  printf("Directory is: %s\n", directory);

  // Send length of directory name (short int)
  int received = send_int(strlen(directory), clientSocket);

  // Send directory name (string)
  send_buffer(clientSocket, directory, strlen(directory));

  // Recieve status update and inform user
  int status = receive_int(clientSocket);

  if (status > 0) {
    printf("Changed current directory.\n");
  } else if (status == -1) {
    printf("ERROR: Error in changing directory\n");
  } else if (status == -2) {
    printf("ERROR: The directory does not exist on server.\n");
  } else {
    printf("ERROR: Unknown status received for CHDR from server.\n");
  }
}

void crfl_handler(int clientSocket) {
  // Get file name and size
  char* filename = strtok(NULL, " ");

  printf("File is: %s\n", filename);

  // Send length of directory name (short int)
  send_int(strlen(filename), clientSocket);

  // Send filename (string)
  printf("sent size\n");
  send_buffer(clientSocket, filename, strlen(filename));

  // Recieve status update and inform user
  int status = receive_int(clientSocket);

  if (status > 0) {
    printf("The file was successfully created.\n");
  } else if (status < 0) {
    printf("ERROR: The file already exists.\n");
  } else {
    printf("ERROR: Unknown status received for CRFL from server.\n");
  }
}

void rmfl_handler(int clientSocket) {
  // Get file name and size
  char* filename = strtok(NULL, " ");

  printf("File is: %s\n", filename);

  // Send length of directory name (short int)
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
        printf("Waiting for status\n");
        int status = receive_int(clientSocket);
        printf("Recieved status back: %d\n", status);

        if (status > 0) {
          printf("File deleted\n");
        } else if (status < 0) {
          printf("Failed to delete file\n");
        } else {
          printf("ERROR: Unknown status received for RMFL from server.\n");
        }
    } else {
        // User does not want to delete
      printf("Delete abandoned by the user!\n");
    }

  } else {
    printf("ERROR: Unknown status received for RMFL from server.\n");
  }
}

void dnld_handler(int clientSocket){
	// Get file name and size
  char* filename = strtok(NULL, " ");

  // Send length of filename (short int)
  send_int(strlen(filename), clientSocket);

  // Send filename (string)
  send_buffer(clientSocket, filename, strlen(filename));

  // Recieve status update and inform user
  int status = receive_int(clientSocket);

	if (status == -1) {
		printf("ERROR: File %s does not exist on server.\n", filename);
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
			printf("ERROR: Failed to create file.\n");
			// TODO: what to do with this failure? server won't know this occured and could be left sending/reading
		} else {

			// Recieve file data
			int receivedBytes = 0;
			char buffer[MAX_BUFFER_SIZE+1];
			bzero((char*)&buffer, sizeof(buffer));

			while(receivedBytes < filesize) {
				// TODO: Bailey: START TIMER FOR THROUGHPUT HERE (accumulate each loop)
				// there is a hint in the instructions on how to do this
				receivedBytes += receive_buffer(clientSocket, buffer, MIN(MAX_BUFFER_SIZE,filesize-receivedBytes));
				printf("Bytes received: %d, chunk of file received: %s\n\n", receivedBytes, buffer);
				// TODO: Bailey: STOP TIMER FOR THROUGHPUT HERE AND
				// use receivedBytes for size in calculation
				//
				// the reason for this placement is to time only the recieveing, and not the disk writing
				fwrite(buffer, sizeof(char), strlen(buffer), fp);
				bzero((char*)&buffer, sizeof(buffer));
			}


			fflush(fp);

			// Calculate md5hash
			char calculatedMd5sum[MD5SUM_LENGTH + 1]; // buffer to hold actual sum
			bzero(calculatedMd5sum, sizeof(calculatedMd5sum));
			char cmd[7 + strlen(filename) + 1]; // buffer to hold command for linux command

			sprintf(cmd, "md5sum %s", filename); // put the actual command string in its buffer

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

			// Compare md5hash
			printf("Calculated md5sum does%s match recieved md5sum\nDownload %ssuccessful!\n", \
			(strcmp(receiveMd5sum, calculatedMd5sum) == 0) ? "" : "n't", \
			(strcmp(receiveMd5sum, calculatedMd5sum) == 0) ? "" : "not "
			);

			// TODO: Bailey: rename and calculate seconds and throughput
			int seconds = 5;
			int rate = receivedBytes/seconds;
			printf("%d bytes transferred in %d seconds: %d MegaBytes\\sec.\n", receivedBytes, seconds, rate);
			printf("MD5Hash: %s (%s)\n", calculatedMd5sum, (strcmp(receiveMd5sum, calculatedMd5sum) == 0) ? "matches" : "doesn't match");
		}
	}
}

void upld_handler(int clientSocket){
	printf("within udld Handler\n");
	// Get file name and size
	char* filename = strtok(NULL, " ");

	printf("sending file name length\n");
	// Send length of filename (short int)
	send_int(strlen(filename), clientSocket);

	// Send filename (string)
	printf("sending filename\n");
	send_buffer(clientSocket, filename, strlen(filename));

	// Wait for server acknowledgment
	printf("awaiting acknowledgment\n");
	receive_int(clientSocket);
	printf("got acknowledgment\n");

	// Check if file already exists
	struct stat st;

	if (stat(filename, &st) == 0 && S_ISREG(st.st_mode)) {
		// Send file size
		printf("sending file size\n");
		int fileSize = st.st_size;
		int sent_size = send_int(fileSize, clientSocket);

		// Send the actual file
		FILE *fp = fopen(filename, "r");
		char buffer[MAX_BUFFER_SIZE + 1];
		bzero(buffer, sizeof(buffer));

		// Send file in chunks of MAX_BUFFER_SIZE
		printf("sending file data\n");
		int sentBytes = 0;
		while(sentBytes < fileSize){
			int bytesRead = fread(buffer, sizeof(char), MAX_BUFFER_SIZE, fp);
			sentBytes += send_buffer(clientSocket, buffer, bytesRead);
			printf("sent %d bytes so far\n", sentBytes);
			bzero(buffer, sizeof(buffer));
		}
		printf("done sending bytes: sent %d\n", sentBytes);

		// Receive throughput
		printf("waiting for thruput\n");
		int throughput = receive_int(clientSocket);
		printf("got thruput: %d\n",throughput);
		// TODO: do something with throughput

		// Receive md5sum

		char receiveMd5sum[MD5SUM_LENGTH + 1];
		bzero(receiveMd5sum, sizeof(receiveMd5sum));
		receive_buffer(clientSocket, receiveMd5sum, MD5SUM_LENGTH);
		printf("got md5sum from server: %s\n", receiveMd5sum);

		// Calculate md5hash
		char calculatedMd5sum[MD5SUM_LENGTH + 1]; // buffer to hold actual sum
		bzero(calculatedMd5sum, sizeof(calculatedMd5sum));
		char cmd[7 + strlen(filename) + 1]; // buffer to hold command for linux command

		sprintf(cmd, "md5sum %s", filename); // put the actual command string in its buffer

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
		printf("calculated md5sum as: %s\n", calculatedMd5sum);

		if (strcmp(receiveMd5sum, calculatedMd5sum) == 0) {
			printf("Transfer success!\n");
			// TODO: format this correctly and for throughput
		} else {
			printf("Transfer failed.\n");
		}


	} else {
		// TODO: WHAT TO DO HERE (file DNE)
	}
	return;
}
