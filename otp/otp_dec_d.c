/*****************************************************************
 ** Name: WeiHao Kuang
 ** Date: 11.26.2018
 ** Program: otp_enc
 ** Description: server side decoder
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
 ** Postconditions: ciphertext will be decoded
 ** Return: none, of void
*******************************************************************/
void parse(char buffer[], char key[], char text[]){
	int i, keyBool=0, textBool=0, keyiter=0, textiter=0;

	//if the "$" then it mean the text is starting
	for (i=0; i<strlen(buffer); i++){
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
		if ( keyBool ==  1) {  //if key logic true
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
 ** Postconditions: will make a plaintext string
 ** Return: none, of void
*******************************************************************/
void decrypt (char crypted[], char key[], char text[]) {
	// hardecode CAP alphabet and space for comparison
	char *alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	int holder=0, holder2=0;
	int i,j; 
	// for length of text
	for ( i=0; i < strlen(text); i++){
		// fo length of alpha
		for ( j=0; j < strlen(alpha); j++){
			// assign holder one the index at which text and alpha are equal
			if ( text[i] == alpha[j] )
				holder=j;// assigned
			// assign holder2 the index at which text and alpha are equal
			if ( key[i] == alpha[j] )
				holder2=j; //assigned
		}
		// alogrithm using modulo 27 talked about in class
		int encryptedChar = (holder - holder2); // subtract to get number
		if (encryptedChar < 0 ) // if negative then add 27
			encryptedChar = encryptedChar + 27;
		else // if not negative then modulo 27 to get orginal char
			encryptedChar = encryptedChar % 27;
		crypted[i] = alpha[encryptedChar]; // the plaintext is remade
	}
}

int main(int argc, char *argv[]) {
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[400000], key[200000], cipher[200000];
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
		
		pid = fork(); // concurrency for the fork
		if( pid < 0 ) // if the forking failed
			error("Error Forking");

		else if ( pid == 0 ){ // if inside the child
			memset(buffer, '\0', sizeof(buffer)); // reset buffer for future use and safe usage
			charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); // Read the client's message from the socket
			
			while( strstr(buffer, "@") == NULL ){ // whiles until the "@" terminator char is seen
				char temp[200000];
				// recieve from established socket 
				charsRead = recv(establishedConnectionFD, temp, sizeof(temp)-1, 0); // Read the client's message from the socket
				strcat(buffer, temp); // concatentate to keep the order of the string
			}
			if ( buffer[0] != 'd') { // authentication if the msg is not from otp_dec then send connection error message back
				charsRead = send(establishedConnectionFD, "otp_dec_d", 9, 0); // Send success back
			}
			else{
				parse(buffer, key, cipher); // run the parse to get arrays
				char crypted[sizeof(cipher)]; // make a plaintext array called crypted
				memset(crypted, '\0', sizeof(cipher)); // memset to size of cipher
				decrypt( crypted, key, cipher); // run the decryption alogrithm
				memset(buffer, '\0', sizeof(buffer)); // memset buffer for future use
				char encryptedMessage[strlen(key)+strlen(crypted)+2]; //making string for later use
				strcat(encryptedMessage,crypted); // store plaintext on string
				strcat(encryptedMessage,"$"); // denotes where the key starts
				strcat(encryptedMessage,key); // the key
				strcat(encryptedMessage,"@"); // denotes when the string ends

				// Send a Success message back to the client
				while(1){
					if(charsRead == strlen(buffer) ) //loop contines until chars read is == to buffer
						break;
					charsRead = send(establishedConnectionFD, encryptedMessage, sizeof(encryptedMessage), 0); // Write to the server // Send success back
				}
				if (charsRead < 0) error("ERROR writing to socket");
			}
		}
		close(establishedConnectionFD); // Close the existing socket which is connected to the client
	}
	close(listenSocketFD); // Close the listening socket
	return 0;
}
