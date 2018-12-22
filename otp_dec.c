/*****************************************************************
 ** Name: WeiHao Kuang
 ** Date: 11.26.2018
 ** Program: otp_dec
 ** Description: client side decoder
 ** Input: decryption files has to have argc = 4
 ** Output: decoded ciphertext - into plaintext output
 ** Notes: client code provided by intructor on canvas
*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


// Error function used for reporting issues
void error(const char *msg) { perror(msg); exit(0); } 
/*****************************************************************
 ** Function: setup
 ** Description: used to parse in files and used to build string to
 ** 			 send to server
 ** Parameters: char **argv, char cipherText[], char key[], char buffer[]
 ** Preconditions: files must exisit from command line
 ** Postconditions: the string that will be used to communication with
 **                server will be sent
 ** Return: none, of void
*******************************************************************/
void setup(char **argv, char cipherText[], char key[], char buffer[]){
	FILE *fp1 = fopen(argv[1], "r"); // file ptr for ciphertext
	FILE *fp2 = fopen(argv[2], "r"); // file ptr for key
	int counter =0, status=0;
	char reader;

	reader = fgetc(fp1); // start with text ptr
    while (reader != EOF) { // read to end of file
		if ( reader > 32 && reader < 65 ){ // if not space or CAP letters
			fprintf(stderr,"Invalid characters in %s\n", argv[1]);
			exit(1); // stderr print and the exit
		}
		cipherText[counter] = reader;
		reader = fgetc(fp1); // reading by char
		counter++; // keep index correctly
    }
	//gets rid of the newline character
	cipherText[strcspn(cipherText, "\n")] = '\0';
    fclose(fp1); // closing the ptr to aovid and later errors
	counter=0;

	reader = fgetc(fp2); // reads the the key ptr
    while (reader != EOF) { // goes until end of file
		if ( reader > 32 && reader < 65 ){ // if not space or CAP letters
			fprintf(stderr,"Invalid characters in key\n");
			exit(1); // stderr exit 1
		}
		key[counter] = reader; // key is stored in key array
        reader = fgetc(fp2); // char read
		counter++; // counting
    }
	key[strcspn(key, "\n")] = '\0'; // eliminate the newline
    fclose(fp2); // closes ptr for error avoidance

	if ( strlen(key) < strlen(cipherText) ){
		// if the key is shorter than the textfile
		fprintf(stderr,"Error: invalid key length\n");
		exit(1); // exit to stderr 1
	}
	else { // this else statment build the string that will be sent to server
		strcat(buffer, "d"); // denotes its from the decoder client
		strcat(buffer, "$"); // denotes start of text
		strcat(buffer, cipherText); // text
		strcat(buffer, "#"); // denotes start of key
		strcat(buffer, key); // key
		strcat(buffer, "@"); // denotes end of string for loops
	}
}
/*****************************************************************
 ** Function: main
 ** Description: everything is called and made here
 ** Parameters: argv and argc
 ** Preconditions: there must be command lines args
 ** Postconditions: string is sent to server side encoder
 ** Return: none, of void
*******************************************************************/
int main(int argc, char *argv[]) {
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[400000], key[200000], cipher[200000];
    
	if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
    setup(argv, cipher, key, buffer); // setting up the reading in information arg line

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Send message to server
	while(1){ // the the chars are not all sent
		if(charsWritten == strlen(buffer) )
			break;
		// writing to socket for send
		charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	}
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) fprintf(stderr,"CLIENT: WARNING: Not all data written to socket!\n");

	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
    while( strstr(buffer, "@") == NULL ){ // while the buffer doesn't recieve a terminate char "@"
		if (strcmp(buffer, "otp_enc_d") == 0 || strcmp(buffer, "otp_dec_d") == 0){
			fprintf(stderr,"otp_dec can not use %s\n", buffer); // printd to stderr if the connection was prohibited
			exit(2); // exit value 2 when that happens
		}
		char temp[200000];
		// receiving the buffer from socket
		charsRead = recv(socketFD, temp, sizeof(temp)-1, 0); // Read the client's message from the socket
		strcat(buffer, temp); // concatenting to get the correct order of strings
	}
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");

	else {
		int i;
		for ( i = 0; i< strlen(buffer); i++){
			if ( buffer[i] == '$') // is this "$" then end
				break;
			// prints the decoded message out to user
			printf("%c", buffer[i]);
		}
		printf("\n");

		close(socketFD); // Close the socket
		return 0;
	}
}
