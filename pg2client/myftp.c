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

#define MAX_BUFFER_SIZE 4096

void list_handler(int);
void mkdr_handler(int);
void rmdr_handler(int);
void chdr_handler(int);
void crfl_handler(int);
void rmfl_handler(int);


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

	  printf("Sending the operation\n");

	  // Send the operation to the server
	  int sent = send_buffer(clientSocket, operation, strlen(operation));
    // TODO: double check that strlen works

	  printf("Sent the operation, %d bytes sent\n", sent);

		// DNLD: Download
		if (strcmp(operation, "DNLD") == 0) {
			printf("Sent DNLD command\n");

		}
		// UPLD: Upload
		else if (strcmp(operation, "UPLD") == 0) {
			printf("Sent UPLD command\n");
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

	// TODO: catch a ^C on the client side and sent a message to the server to shut down
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
    printf("Are you sure you would like to delete %s?\n", directory);

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

  // Recieve status update and inform user
  int status = receive_int(clientSocket);

  if (status < 0) {
    printf("The file does not exist on server.\n");
  } else if (status > 0) {
    // Get user confirmation
    printf("Are you sure you would like to delete %s?\n", filename);

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
