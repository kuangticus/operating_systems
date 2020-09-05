/*****************************************************************
 ** Name: WeiHao Kuang
 ** Date: 11.26.2018
 ** Program: otp_enc
 ** Description: server side encoder
 ** Input: port number argc = 2
 ** Output: encodes plaintext into ciphertext
*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char *msg) { perror(msg); exit(1); } 

/*****************************************************************
 ** Function: parse
 ** Description: used to parse in string send fro client into sub arrays
 ** Parameters: char buffer[], char key[], char text[]
 ** Preconditions: the string sent from client must exist
 ** Postconditions: plaintext will be encoded
 ** Return: none, of void
*******************************************************************/
void parse(char buffer[], char key[], char text[]){
	int i, keyBool=0, textBool=0, keyiter=0, textiter=0;

	for (i=0; i<strlen(buffer); i++){ 
		//if the "$" then it mean the text is starting
		if( buffer[i] == '$' ){
			textBool = 1; // text logic true
			continue; // avoid writing '$' to string
		}
		//if the "#" then the key is starting
		else if ( buffer[i] == '#'){
			textBool = 0; // text logic false
			keyBool = 1; // key logic true
			continue; // avoid writing '#' to string
		}
		// if @ then end
		else if (  buffer[i] == '@' ){
			break;
		}
		// writing the text to sub array text for later use
		if ( textBool == 1 ) { // if text logic true
			// writing to string
			text[textiter] = buffer[i];
			textiter++;
		}
		// writing the key to sub array text for later use
		if ( keyBool ==  1) { //if key logic true
			// writing to string
			key[keyiter] = buffer[i];
			keyiter++;
		}
	}
}

/*****************************************************************
 ** Function: encrypt
 ** Description: encrypts plaintext to ciphertext
 ** Parameters: char crypted[], char key[], char text[]
 ** Preconditions: all of parameters must exist
 ** Postconditions: will make a cipher text string
 ** Return: none, of void
*******************************************************************/
void encrypt (char crypted[], char key[], char text[]) {
	// hardcoded CAP alphabet and space for comparison
	char *alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "; 
	int holder=0, holder2=0;
	int i,j; 
	// loops through plaintext length
	for ( i=0; i < strlen(text); i++){ 
		// loops through CAP alphabet and space 
		for ( j=0; j < strlen(alpha); j++){
			// setting the text letter to index of alpha
			if ( text[i] == alpha[j] ) 
				holder=j;
			// setting the key letter to index of alpha
			if ( key[i] == alpha[j] )
				holder2=j;
		}
		// encrypt using the modulo 27 method mentioned in class
		int encryptedChar = (holder + holder2) % 27;
		crypted[i] = alpha[encryptedChar]; // ciphertext array
	}
}

/*****************************************************************
 ** Function: main
 ** Description: everything is called and made here
 ** Parameters: argv and argc
 ** Preconditions: there must be command lines args
 ** Postconditions: ciphertext is sent to client side encoder
 ** Return: none, of void
*******************************************************************/
int main(int argc, char *argv[]) {
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead=0;
	socklen_t sizeOfClientInfo;
	char plainText[200000];
	char key[200000];
	char buffer[300000];
	struct sockaddr_in serverAddress, clientAddress;
	pid_t pid;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	while(1) {
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");
		// Get the message from the client and display it
		pid = fork(); // forking to get concurrent processes
		if( pid < 0 ) // if fork fail tell me bout it
			error("Error Forking");

		else if ( pid == 0 ){ // if inside of the child
			memset(buffer, '\0', sizeof(buffer)); // reset buffer for ease of use
			charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); // Read the client's message from the socket
			while( strstr(buffer, "@") == NULL ){ // recieve till the "@" in the buffer read
				char temp[200000];
				charsRead = recv(establishedConnectionFD, temp, sizeof(temp)-1, 0); // Read the client's message from the socket
				strcat(buffer, temp); // concatenate to get correct order
			}
			if ( buffer[0] != 'e') { // authentication if the msg isnt from otp_enc 
				charsRead = send(establishedConnectionFD, "otp_enc_d", 9, 0); // Send error back to process trying to acces server
			}
			else{
				parse(buffer, key, plainText); // run the parsing
				char crypted[sizeof(plainText)]; // initialize a cipher array
				memset(crypted, '\0', sizeof(buffer)); // clear and set array for later use
				encrypt( crypted, key, plainText); // run encryption algorithm
				memset(buffer, '\0', sizeof(buffer)); // memset buffer for future use
				char encryptedMessage[strlen(key)+strlen(crypted)+2]; // create a string to send everything back
				strcat(encryptedMessage,crypted); // add cipher to string
				strcat(encryptedMessage,"$"); // denotes beginning of key
				strcat(encryptedMessage,key); // key
				strcat(encryptedMessage,"@"); // end of string sent back

				// Send a Success message back to the client
				while(1){ // send until the end of the string "@"
					if(charsRead == strlen(buffer) ) // if the end of the string is reach break
					break;
					// send on established socket
					charsRead = send(establishedConnectionFD, encryptedMessage, strlen(encryptedMessage), 0); // Write to the server // Send success back
				}
				if (charsRead < 0) error("ERROR writing to socket");
			}
		}
		close(establishedConnectionFD); // Close the existing socket which is connected to the client
	}
	close(listenSocketFD); // Close the listening socket
	return 0;
}