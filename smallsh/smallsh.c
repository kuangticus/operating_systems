/******************************************************************************************
 ** Name: WeiHao Kuang 
 ** Date: 11/14/18
 ** Program: smallsh
 ** Functions: basically a functioning shell with minimal functions compared 
    to a really shell
 ** Input: User input that will specify the commands they want to run
 ** Output: to stdout mainly, if bckgnd process then dev null is default
 ** Cites: // https://www.geeksforgeeks.org/how-to-split-a-string-in-cc-python-and-java/
******************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>

#define SIGINT 2   // Interrupt the process
#define SIGTSTP 20 // Stops the process

// global setup and definations
const char *prebuilt[] = {"exit", "cd", "status"};
char *argList[512]; // handles 512 arguments with "X" amount of characters, 
int processPID[1000]; // used to keep track of processes.
int stats;     // is used for the 
int numArgs=0; // number of arguments that are inputted
int numProcess=0; // number of processes 
char *inputFrom; // file input descriptor
char *outputTo; // file ouput descriptor
bool foreGND = true;
int allowBackground = 1;

struct sigaction SIGINT_action, SIGTSTP_action, INTERRUPTED_action;

int argGather(char*); // gets the argumets from the user input
void commandCall(); // compare funciotn that will call other functions
void Exit();
void status(int*); //done will return the status or the terminated by signal 
void cd(); // cd is done and will move user to directories they specify
void otherCmds(int*); 
void pidGet(pid_t); //getst he pid and puts them in to a global array
void catchSIGTSTP(int); // works and will handle the forground only mode

/****************************************************************************************************************
 ** Function: main 
 ** Description: runs all of the functions and the heart of the program call
 ** Parameters: none, for this implementation
 ** Pre-conditions: complilation of this have to be completed and the executable has to run
 ** Post-conditions: the game files will be made
 ** Return: 0
****************************************************************************************************************/
int main (){

    char readArgs[2048]; // read string buffer
    char firstArg[2048]; // read string buffer

    // used to handle the control-z with function
    // this lines below are used to setup the signal catching of
    // control Z with is shell script
    SIGTSTP_action.sa_handler=catchSIGTSTP; 
    SIGTSTP_action.sa_flags = SA_RESTART; // reset the state before interrupt
    sigfillset(&SIGTSTP_action.sa_mask);
    sigaction(SIGTSTP, &SIGTSTP_action, NULL); // put into action the the handler statment

    SIGINT_action.sa_handler=SIG_IGN; // ignoring control C from the very beginning
    sigfillset(&SIGINT_action.sa_mask); // using mask to intitialize and fill the signal set
    sigaction(SIGINT, &SIGINT_action, NULL); //runnes the handler function

    while(1){ // this will be the while loop the runs the shell
        numArgs = argGather(readArgs); // getting num args and arg list
        argList[numArgs] = NULL; // setting the last 
        commandCall( numArgs ); // calling the right command functions
    }
    return 0;
}

/****************************************************************************************************************
 ** Function: catchSIGTSTP
 ** Description: this is the control z handler that will show the forground only prompt and exting prompt
 ** Parameters: int signo, the type of signal pased in in numbers
 ** Pre-conditions: there must be a struct signal made and intialized
 ** Post-conditions: the code will either be forground only or both
 ** Return: none
****************************************************************************************************************/
void catchSIGTSTP(int signo) {
	// If it's 1, set it to 0 and display a message reentrantly
	if (allowBackground == 1) {
		char* message = "Entering foreground-only mode (& is now ignored)\n";
        char* message1 = ": "; // entrance of forground only mode
		write(1, message, 49); // message one
        write(1, message1, 2); // messgae two
		fflush(stdout);
		allowBackground = 0; //makes the condition that will ignore the background descriptor
	}

	// If it's 0, set it to 1 and display a message reentrantly
	else {
		char* message = "Exiting foreground-only mode\n";
        char* message1 = ": ";
		write (1, message, 29); // messgae one
        write(1, message1, 2);  // message two
		fflush(stdout);
		allowBackground = 1; // set he allowBackground varible back into place
	}
}

/****************************************************************************************************************
 ** Function: argGather
 ** Description: 
 ** Parameters: args
 ** Pre-conditions: the is a string input from user stdin
 ** Post-conditions: the array is made and the arguments are stored into them
 ** Return: int numArgs, # of args
****************************************************************************************************************/
int argGather(char*args) {
    int counter = 0; // remeber to +1 for printing out since it is at 0
    printf("%s", ": "); // prompt beginning
    fflush(stdout);
    fgets(args,2048,stdin); // read in what the user wants
    strtok(args, "\n"); // removing the newlines from getline
    char buffer [2048];
    int i, j;
   
    char *word = strtok(args, " "); // spliting the string to get every word
    
    // using tokens to split up the string
    while (word != NULL) {  // token word that will iterated
        argList[counter] = word;
        word = strtok(NULL, " ");  // move to next word 
        counter++;
    }

    // this handles the expansion of the $$
    for ( i=0; i< counter; i++) {
        for ( j =0; j < strlen(argList[i]); j++){ // looks for the $$
            if ( argList[i][j] == '$' && argList[i][j+1] == '$'){
                argList[i][j] = '\0'; // replaces the $$ with NULLS
                argList[i][j+1] = '\0'; // replace the $$ with NULLS
                // appending to make files with process IDs
                snprintf(buffer,2048,"%s%d", argList[i], getpid() );
                argList[i] = buffer; 
            }
        }
    }
    return counter; // return 
}

/****************************************************************************************************************
 ** Function: commandCall 
 ** Description: this will call the prebuilt and shell commmands based on user input, using if statements
 ** Parameters: none
 ** Pre-conditions: arglist exists and args are strings
 ** Post-conditions: depending on the user input different commands will be ran
 ** Return: none 
****************************************************************************************************************/
void commandCall() {
    int sigHold, called = 0;

    if ( *argList[0] == '\n' || *argList[0] == '#' ){
        // nothing goes here becuase it is used to ignore stuff
    }
    else if ( strcmp( argList[0], prebuilt[0] ) == 0 ){
        Exit(); // calling the exit function is prebuilt exit
    }
    else if ( strcmp( argList[0], prebuilt[1] ) == 0 ){
        cd (numArgs); // calls the chdir function prebuit cd
    }
    else if ( strcmp( argList[0], prebuilt[2] ) == 0 ){
        status(&called); // gets the status function prebuilt
    }
    else{
        foreGND = true; // if the foreground is true
        // handles the forground only mode with &
        if ( allowBackground == 0 && strcmp(argList[numArgs-1],"&")==0 ){
            argList[numArgs-1 ] = NULL; // sets the & to null for execvp
        }  
        else if (strcmp(argList[numArgs-1],"&")==0) {
            argList[numArgs-1]=NULL; // index holding the designator gets null
            foreGND = false; // set background designator
        }
        otherCmds(&called); // calling this function to handle none prebuilt func.nalities

        if ( WIFSIGNALED(stats) && called == 0 ){ // if child exited by only if killed by child
            status(&called); // handles the terminated by control C mesage
        }
    }
}

/****************************************************************************************************************
 ** Function: cd
 ** Description: prebuilt cd, this cd function will be used to change directories within the shell 
 ** Parameters: none
 ** Pre-conditions: the path that is passed in has to be a string
 ** Post-conditions: it will either not change dirs or it will change to the path specified 
 ** Return: none
****************************************************************************************************************/
void cd(){
    if( numArgs == 1 ){ // if only cd is called
        chdir(getenv("HOME")); // this command will move to the Home directory
    }

    else{ // if there are more than one argument, only will read the the first after chdir
        if ( chdir(argList[1]) != 0 ){
        }
    }
}

/****************************************************************************************************************
 ** Function: Exit
 ** Description: prebuilt exit function that kill all of the processes that are around running
 ** Parameters: none
 ** Pre-conditions: must have an array that can hold pids
 ** Post-conditions: the pids in the array are all killed expect for the parent, in which case exit is used 
 ** Return: 0, exit(0)
****************************************************************************************************************/
void Exit () {
    int i=0, pid;
    for ( i= 0; i < 1000; i++) {
        if ( pid == 0 ){ // if parent just exit 
            exit(0); // exit on a good note
        }
        kill(processPID[i], SIGTERM); // kill the pid    
    }
}

/****************************************************************************************************************
 ** Function: otherCmds
 ** Description: this will run the non prebuilt fucntion of the shell using execvp
 ** Parameters: int called, to handle the terminated by control C signal 
 ** Pre-conditions: there must be a arglist that is already made or else it will segfault
 ** Post-conditions: the non built in functions should be ran
 ** Return: none
****************************************************************************************************************/
void otherCmds ( int* called) {
    pid_t c_pid, pid, tempPID; // initializing the parent and the the 
    int fileDes0=0, fileDes1=0, i=0, inFrom=0, outTo=0; // intializing vars
    char inputFrom[2048],  outputTo[2048]; // input and output file buffer

    c_pid = fork(); // forking of a child
    pidGet(c_pid);

    if (c_pid == 0) { // if the child

        for( i=0; argList[i]!=NULL; i++) { //iterate until null terminator

            if(strcmp(argList[i],"<")==0) { // if the input designator
                inFrom=1; // conditional statement when this is true
                argList[i]=NULL; // the index holding the the desginator gets null
                strcpy(inputFrom, argList[i+1]); // copying the str over to get input
            }               

            else if(strcmp(argList[i],">")==0) { // if the output designator
                outTo=1; // conditional statement requirement when this is true
                argList[i]=NULL; // index holding the designator gets null
                strcpy(outputTo, argList [i+1]); // copying to str to get output     
            }        
        }   

        // Back GND  process that will be redirected to dev/null  
        if ( foreGND == false && outTo==0 && inFrom==0){
            freopen ("/dev/null", "w", stdout); // or "nul" instead of "/dev/null"
            freopen ("/dev/null", "r", stdin); // or "nul" instead of "/dev/null" 
            fcloseall();// closing all of the streams
        }   
        
        if(inFrom) {  // if the input designator called '<' 
            if ((fileDes0 = open(inputFrom, 0)) < 0) { // open the inputFrom desginator 
                perror("Couldn't open input file"); // if can't open error
                exit(1); // exit the process
            }    
            dup2(fileDes0, 0); // else if good redirect from stdin from file
            close(fileDes0); // closing the redirection
        }

        if (outTo) { // if the input designator called '>'
            if ((fileDes1 = creat(outputTo, 0644)) < 0) { // creating a writing file
                perror("Couldn't open the output file"); // if can't open error
                exit(1); // exit the process
            }              
            dup2(fileDes1, 1); // redirection from stdout to file
            close(fileDes1); // close filepath
        }

        //this will be true for forground processes lets control C kil the process since forground
        if (foreGND == true )
            SIGINT_action.sa_handler=SIG_DFL; // default handler means kill the process
        
        sigaction( SIGINT, &SIGINT_action, NULL ); // take action on sigint

        if ( execvp(argList[0], argList ) == -1 ) {//calling execute to run non built in commands
            perror(argList[0]); // perror is the command is non existent
            exit(1); // if there is problem print error
        }
    }

    else if (c_pid > 0){ // if parent
    
        if (foreGND == false && allowBackground == 1 ) { // executing the command like  normally
				tempPID = waitpid(c_pid, &stats, WNOHANG); // with no hang and check child
				printf("Background pid: %d\n", c_pid); // prints the background
				fflush(stdout); 
		}
        else {  //Executing the cmd it like a normal one
            // printf("else is working!\n");
            fflush(stdout);
            tempPID = waitpid(c_pid, &stats, 0);
        }
    }
    else { // if fork fails
        perror("fork");
        exit(1); // exit process
    }

    while ((c_pid = waitpid(-1, &stats, WNOHANG)) > 0) { // wait to read out children deaths
        printf("background pid %d is done: ", c_pid);
        status(called); //status of the deceased child 
        fflush(stdout);
	}
}

/****************************************************************************************************************
 ** Function: status
 ** Description: returns the status of the terminations, or the killed by whom signal reporter
 ** Parameters: called, this is to handle killed by whom for contl C
 ** Pre-conditions: there must be process that are made in order for this to work, process must exist
 ** Post-conditions: the status of the process in question will be returned
 ** Return: none
****************************************************************************************************************/
void status (int *called) {
    int errHold=0, sigHold=0, competency;

    waitpid(getpid(), &stats, 0); //geting the new stat from the child process

    if ( WIFEXITED(stats) )  // return a value > 0 if child exits norm
        errHold = WEXITSTATUS(stats); // give status of the child that dead

    if ( WIFSIGNALED(stats) ) // if child exited by only if killed by child
        sigHold = WTERMSIG(stats); // interprete the signal that killed it

    if ( errHold == 0 && sigHold == 0 ) // if there are no problems
        competency = 0;

    else // if problems occur on exit
        competency = 1;

    if ( sigHold != 0 ){ // if there is a signal other than zero 
        *called = 1; // handles the terminated by signal for control c
        printf( "terminated by signal %d\n", sigHold); // spit out signal
        fflush(stdout);
    }
    else { // otherwize print the exit status
        printf( "exit value %d\n", competency);
        fflush(stdout); // flusing to avoid errors
    }
}

/****************************************************************************************************************
 ** Function: pidGet
 ** Description: gets the pid of the processes and stores them into a global process array
 ** Parameters: pid_t num which is the passed in array
 ** Pre-conditions:  must have a pid avaible for pass in
 ** Post-conditions: the pid of the process will be inputted into the array
 ** Return: none
****************************************************************************************************************/
void pidGet ( pid_t num ) { // get pid
    processPID[numProcess] = num;
    numProcess++; // the numbur of processes are counted
}