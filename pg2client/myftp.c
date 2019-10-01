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

// send public key to the server (sentto() function)

int send_short(short value, int socket)
{
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

	char messageBuffer[MAX_BUFFER_SIZE];

  // connect
  if (connect(clientSocket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
    printf("Client: Error connecting to server: %s\n", strerror(errno));
    exit(0);
  }

  printf("Client: Connected\n");

	char *op;

	while(1) {

		// Prompt the user for an operation
		printf("Operations: DNLD: Download, UPLD: Upload, LIST: List, MKDR: Make Directory, RMDR: Remove Directory, CHDR: Change Directory, CRFL: Create File, RMFL: Remove File, QUIT: Quit\n");
		printf("Enter an operation: ");
		fgets(op, 50, stdin);

		// get rid of the endline character
		op[strlen(op)-1] = '\0';

	  printf("Sending the message\n");

	  // send the operation to the server
	  // int sent = sendto(clientSocket, op, 50, 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
		int sent = send_buffer(clientSocket, op, strlen(op));

	  printf("Sent the message, %d characters\n", sent);

		// DNLD: Download
		if (strcmp(op, "DNLD") == 0) {
			printf("Sent DNLD command\n");

		}
		// UPLD: Upload
		else if (strcmp(op, "UPLD") == 0) {
			printf("Sent UPLD command\n");
		}
		// LIST: List
		else if (strcmp(op, "LIST") == 0) {

			printf("Sent LIST command\n");

			// Get size of directory
			int directorySize = receive_int(clientSocket);
			printf("Size of directory: %d\n", directorySize);

			// Recieve listing
			int bytesRecvd = 0;
			while(bytesRecvd < directorySize) {
				memset(&messageBuffer, 0, sizeof(messageBuffer));
				bytesRecvd += receive_buffer(clientSocket, messageBuffer, sizeof(messageBuffer));
				printf("Bytes received: %d\n", bytesRecvd);
				printf("Msg buffer: %s\n", messageBuffer);

				// Escape reading loop if its empty or error
				if (bytesRecvd <= 0){
					break;
				}
			}
			fflush(stdout);
		}
		// MKDR: Make Directory
		else if (strcmp(op, "MKDR") == 0) {
			printf("Sent MKDIR command\n");

		}
		// RMDR: Remove Directory
		else if (strcmp(op, "RMDR") == 0) {
			printf("Sent RMDR command\n");

		}
		// CHDR: Change Directory
		else if (strcmp(op, "CHDR") == 0) {
			printf("Sent CHDR command\n");

		}
		// CRFL: Create File
		else if (strcmp(op, "CRFL") == 0) {
			printf("Sent CRFL command\n");


		}
		// RMFL: Remove File
		else if (strcmp(op, "RMFL") == 0) {
			printf("Sent RMFL command\n");

		}

		else if (strcmp(op, "QUIT") == 0){
			break;
		}

		else {
			printf("ERROR: Invalid command.\n");
		}

	}

	close(clientSocket);

	// TODO: catch a ^C on the client side and sent a message to the server to shut down
}
