/*****************************************************************
 ** Name: WeiHao Kuang
 ** Date: 11.26.2018
 ** Program: otp_enc
 ** Description: client side encoder
 ** Input: encryption files has to have argc = 4
 ** Output: encodes plaintext into ciphertext
*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issuesv
/*****************************************************************
 ** Function: setup
 ** Description: used to parse in files and used to build string to
 ** 			 send to server
 ** Parameters: char **argv, char plainText[], char key[], char buffer[]
 ** Preconditions: files must exisit from command line
 ** Postconditions: the string that will be used to communication with
 **                server will be sent
 ** Return: none, of void
*******************************************************************/
void setup(char **argv, char plainText[], char key[], char buffer[]){
	FILE *fp1 = fopen(argv[1], "r"); // file ptr for text
	FILE *fp2 = fopen(argv[2], "r"); // file ptr for key
	int counter =0, status=0;
	char reader;

	reader = fgetc(fp1); // reads the text file first
    while (reader != EOF) { // reads till end of file
		if ( reader > 32 && reader < 65 ){ // if not space or CAP letters
			fprintf(stderr,"Invalid characters in %s\n", argv[1]);
			exit(1); // exit to stderr
		}
		plainText[counter] = reader; // stores in the plaintext array
		reader = fgetc(fp1); //char by char
		counter++; 
		
    }
	// gets rid of the newline char
	plainText[strcspn(plainText, "\n")] = '\0';
    fclose(fp1); // closes ptr to avoid problems
	counter=0;

	reader = fgetc(fp2); // reads the key text
    while (reader != EOF) { // until end of file
		if ( reader > 32 && reader < 65 ){ // if not CAP letters or space
			fprintf(stderr,"Invalid characters in key\n");
			exit(1); // exit 1 to stderr
		}
		key[counter] = reader; // key gets the the key from file
        reader = fgetc(fp2); // char by char
		counter++;
    }
	//removes the newline from the array
	key[strcspn(key, "\n")] = '\0';
    fclose(fp2); // closes to avoid problems

	// if the key is shorter than text
	if ( strlen(key) < strlen(plainText) ){
		fprintf(stderr,"Error: invalid key length\n");
		exit(1); // exit 1 to stderr
	}
	else {
		strcat(buffer, "e"); // denotes msg from encoder client
		strcat(buffer, "$"); // denotes start of text
		strcat(buffer, plainText); // text
		strcat(buffer, "#"); // denotes the start of key
		strcat(buffer, key); // key
		strcat(buffer, "@"); // denotes end of msg
	}
}
/*****************************************************************
 ** Function: main
 ** Description: everything is called and made here
 ** Parameters: argv and argc
 ** Preconditions: there must be command lines args, and side must
 ** 			   must be sent by client
 ** Postconditions: string text is sent to client side encoder
 ** Return: none, of void
*******************************************************************/
int main(int argc, char *argv[]) {
	int socketFD, portNumber, charsWritten=0, charsRead=0;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char plainText[200000];
	char key[200000];
	char buffer[400000];
    
	if (argc < 4) { fprintf(stderr,"USAGE: %s file key port\n", argv[0]); exit(0); } // Check usage & args

	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
	setup(argv, plainText, key, buffer);

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
	while(1){ // while the buffer is not completely sent
		if(charsWritten == strlen(buffer) ) // once it sent break
			break;
		// writing to established socket
		charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	}
	// error msg that checks if packets were completely sent
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) fprintf(stderr,"CLIENT: WARNING: Not all data written to socket!\n");

	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer)-1, 0); // Read the client's message from the socket
	while( strstr(buffer, "@") == NULL ){ // recieve till the end char "@" is in the recv;
		// handles when the enc or dec connects to the wrong server port
		if (strcmp(buffer, "otp_enc_d") == 0 || strcmp(buffer, "otp_dec_d") == 0){
			fprintf(stderr,"otp_enc can not use %s\n", buffer); // prints error msg
			exit(2);
		}
		char temp[200000];
		// reading from established socket
		charsRead = recv(socketFD, temp, sizeof(temp)-1, 0); // Read the client's message from the socket
		strcat(buffer, temp); // concatenation to achieve correct ordering of string
	}
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");

	else {
		int i;
		for ( i = 0; i< strlen(buffer); i++){
			if ( buffer[i] == '$') // prints the recieved msg from server till "$"
				break;
			printf("%c", buffer[i]); //printing the char out to avoid errors
		}
		printf("\n");

		close(socketFD); // Close the socket
		return 0;
	}
}