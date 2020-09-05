/**************************************************************************************************************
 ** Name: WeiHao Kuang
 ** Date: October 21, 2018
 ** Program Name and Description: buildroom with pid, using file I/O
 ** Input: Randomly generate rooms, "random" input
 ** Return: 7 rooms files, that are housed in one room folder with pid as name (folder)
 ** References: https://cboard.cprogramming.com/c-programming/165757-using-process-id-name-file-directory.html
                https://www.geeksforgeeks.org/c-program-count-number-lines-file/
                https://www.quora.com/How-do-I-read-nth-line-of-a-file-in-C-using-File-handling

***************************************************************************************************************/
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "stdlib.h"
#include "stdbool.h"
#include "time.h"
#include "string.h"
#include "dirent.h"
#include "pthread.h"
#include "fcntl.h"

// declartion of global variables

pthread_mutex_t myMutex =PTHREAD_MUTEX_INITIALIZER; // mutex 
pthread_t timeID; // time mutex

const char *roomNames[] = { "Dankpit", "Dankrip", "Dankest", "Dankstall", "Dankpill", 
                          "Dankfall", "Dankmeme", "Dankness", "Dankpain", "Dankfun"};
const char *roomTypes[] = { "START_ROOM", "MID_ROOM", "END_ROOM" };
char **list;
int *roomsConnected[6]; // rooms
int pathCounter=0; // path steps

//game functions // protopypes
void *getTime();
void writeTime();
void iniPath();
void newestDir(char *);
void namesGet();
int lineGet(char*, FILE*);
void startEndFind( FILE*, FILE*, int*);
bool gameLoop(FILE*, FILE*, int*, char [][10]);
int displayFile( int*, int, char [][256] );
bool errorCheck(char*, char[][256], int );
int getIndex(char*);

/****************************************************************************************************************
 ** Function: main 
 ** Description: runs all of the functions and the heart of the program call
 ** Parameters: none, for this implementation
 ** Pre-conditions: complilation of this have to be completed and the executable has to run
 ** Post-conditions: the game files will be made
 ** Return: 0
****************************************************************************************************************/
int main() {
    
    pthread_mutex_lock(&myMutex);
    pthread_create ( &timeID, NULL, getTime, (void*) &myMutex );

    bool gameEnded = false; // initaliazaiion of the variables
    FILE *reader, *lineReader;
    int i, lines, pathCount=0;
    int startEnd[7]; // first 1st index is start room, last index is the end room
    char newestDirName[256], buffer[256];
    char path[150][10]; // the path checking and holding

    // used for dynamic memory allocation for names arary;
    list = malloc(7*sizeof(char*));
    for ( i=0; i<7; i++){
        list[i]= malloc(10*sizeof(char));
    }
    
    newestDir( newestDirName ); // find newest dir
    chdir(newestDirName); // change to that dir
    namesGet(); // get the name of the new dir
 
    startEndFind(reader, lineReader, startEnd); // find the all of the room value of the array
    gameEnded = gameLoop(reader, lineReader, startEnd, path); // runs the game
 
    // prints out the correct ending statments.
    if ( gameEnded ) {
        printf ( "%s%d%s\n", "YOU HAVE FOUND THE END ROOM. CONGRATULATIONS! YOU TOOK ",
                        pathCounter, " STEPS. YOUR PATH TO VICTORY WAS: " );
            for ( i=0; i < pathCounter ; i++){
                printf( "%s\n", path[i] );
            }
    }

    // freeeing the memeory that was allocated.
    for( i=0; i<7; i++ ){
        free(list[i]);
    }
    free(list);
   
    return 0;
}

/****************************************************************************************************************
 ** Function: newestDir
 ** Description: Gets the most current directory. 
 ** Parameters: dir name that will get the name of the dir
 ** Pre-conditions: current directory works here goes in
 ** Post-conditions: goes to the directory
 ** Return: 0
****************************************************************************************************************/
void newestDir ( char *newestDirName ) {
    int newestDirTime = -1; // Modified timestamp of newest subdir examined
    char targetDirPrefix[32] = "kuangw.rooms."; // Prefix we're looking for
    memset(newestDirName, '\0', sizeof(newestDirName));

    DIR* dirToCheck; // Holds the directory we're starting in
    struct dirent *fileInDir; // Holds the current subdir of the starting dir
    struct stat dirAttributes; // Holds information we've gained about subdir

    dirToCheck = opendir("."); // Open up the directory this program was run in

    if (dirToCheck > 0) // Make sure the current directory could be opened
    {
        while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
        {
        if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
        {
            stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

            if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
            {
            newestDirTime = (int)dirAttributes.st_mtime;
            memset(newestDirName, '\0', sizeof(newestDirName));
            strcpy(newestDirName, fileInDir->d_name);
            }
        }
        }
    }
    closedir(dirToCheck); // Close the directory we opened
}
/****************************************************************************************************************
 ** Function: namesGet
 ** Description: // used to get the names of the files that are in the current directory
 ** Parameters: none, for this implementation
 ** Pre-conditions: has to be in the correct directory
 ** Post-conditions: the game files will be made
 ** Return: 0
****************************************************************************************************************/
void namesGet () {
    int i=0;
    DIR *d; // directory pointer
    struct dirent *dir;
    d = opendir("."); // open the current directory
    if (d) {
        while ((dir = readdir(d)) != NULL) { // read all of the files in the dir
            //printf("%s\n", dir->d_name);
            if ( dir->d_name[0] != '.' ){
                strcpy(list[i],dir->d_name); // puts all of the names into an array
                i++;
            }
        }
        closedir(d); // close the dir
    }
}
/****************************************************************************************************************
 ** Function: lineGet
 ** Description: used to get the lines of the files that are then used to file manipulation/reading.
 ** Parameters: filename, file pointer
 ** Pre-conditions: has to have file, exists
 ** Post-conditions: getst he lines of the file
 ** Return: lines of the file
****************************************************************************************************************/
int lineGet (char *filename, FILE* reader) { 
    int lines = 0;
    char c;

    reader = fopen(filename, "r"); // open the file that we want # lines for 

    for (c = getc(reader); c != EOF; c = getc(reader)) // read till the end of file 
        if (c == '\n')
            lines = lines + 1; // increment lines
   
    fclose(reader); // close file

    return lines;
}
/****************************************************************************************************************
 ** Function: startEndFind
 ** Description: this funciton si used to find the room types with arrays, sorts them 
 ** Parameters: 2 file pointers, array
 ** Pre-conditions: startend array has to exist, file pointers have to exisit
 ** Post-conditions: array gets filled
 ** Return: none
****************************************************************************************************************/
void startEndFind(FILE *reader, FILE *lineReader, int* startEnd){

    int i, holder =1;

    for ( i =0; i<7; i++ ){ // for the size of the arary

        int condition =1;
        char line[256]; // read buffer
        char compare[256]; // compare buffer array
        int counter=0; 
        
        lineReader = fopen(list[i], "r");  // open the file to read   
        int lines = lineGet( list[i], reader ); // read the # of lines of the file and read the last line
        while (fgets(line, sizeof(line), lineReader)) {
            counter++;
            // room type sorting
            if( counter == lines ) { // when the counter equals the lines.
                strcpy ( compare , line+11 );
                strtok(compare, "\n"); // gets rid of the new lines in the string
                if ( strcmp(compare, "START_ROOM") == 0 ) // if start room
                    startEnd[0] = i;
                else if ( strcmp(compare, "END_ROOM") == 0 ) // if end room
                    startEnd[6] = i;
                else if ( strcmp(compare, "MID_ROOM") == 0){ // if mid room
                    startEnd[holder] = i;
                    holder++;
                }
            }
        }
        fclose(lineReader); // closes the file to keep from memepry loss
    }
}
/****************************************************************************************************************
 ** Function: gameloop
 ** Description: this is the game loop that runns all of the game functions
 ** Parameters: everything that is important, such as the room type array, and the path array
 ** Pre-conditions: all of the things that are in the parameter are correct and exist
 ** Post-conditions: the game runs fine and exits
 ** Return: returns true when the game is done
****************************************************************************************************************/
bool gameLoop(FILE* reader, FILE* lineReader, int* startEnd, char path [][10]){
    int i, room=startEnd[0], connections, index;
    char input[256]; // used for thet the user in put for name
    char pointer[6][256]; // used for the pointing or the connection array.

    // intital display of the start room information
    connections = displayFile ( startEnd, room, pointer ); // display the start room
    printf( ".\n%s", "WHERE TO? >"); 
    fgets(input, sizeof(input), stdin); // reads to buffer
    strtok(input, "\n"); // removes newline chars

    // checks that input to see if they match with connection array
    while ( errorCheck(input, pointer, connections) ){ // if the connection array path DNE
        if( strcmp(input, "time") == 0 ){
            pthread_mutex_unlock(&myMutex); // unlocks mutex, allowing time thread to lock it and run
            pthread_join(timeID, NULL); //join time thread so current thread stop of a bit
            pthread_mutex_lock(&myMutex); // time thread is now done, lock up the mutex
            pthread_create ( &timeID, NULL, getTime, (void*) &myMutex ); // make a new time thread
            writeTime(); // display the time
            printf("\n");
            printf( "\n%s", "WHERE TO? >");
            fgets(input, sizeof(input), stdin);
            strtok(input, "\n"); // rm the the \n
        }
        else {
            printf( "%s", "HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN. ");
            fgets(input, sizeof(input), stdin);
            strtok(input, "\n"); // remove newline
        }
        printf("\n");
    }
    strcpy ( path[pathCounter], input); // copy to the path array
    pathCounter++; // iterate the path array

    room = getIndex(input);  // gets the room to that is next in the connections'

    while ( strcmp (list[1], input) != 0 ){ // loop that goes on until the end room is found
        connections = displayFile ( startEnd, room, pointer ); // dipslay current room details
        printf( ".\n%s", "WHERE TO? >");
        fgets(input, sizeof(input), stdin);
        strtok(input, "\n"); // removes \n
        while ( errorCheck(input, pointer, connections) ){ // error checking to check the input
            if( strcmp(input, "time") == 0 ){
                pthread_mutex_unlock(&myMutex); // unlocks mutex, allowing time thread to lock it and run
                pthread_join(timeID, NULL); //join time thread so current thread stop of a bit
                pthread_mutex_lock(&myMutex); // time thread is now done, lock up the mutex
                pthread_create ( &timeID, NULL, getTime, (void*) &myMutex ); // make a new time thread
                printf("%c", ' ');
                writeTime(); // display the time
                printf("\n");
                printf( "\n%s", "WHERE TO? >");
                fgets(input, sizeof(input), stdin);
                strtok(input, "\n"); // rm the the \n
            }
            else {
                printf( "%s", "HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN. ");
                fgets(input, sizeof(input), stdin);
                strtok(input, "\n"); // rm the the \n
            }
            printf("\n");
        }
        // the path is used to store what the user took to get to the end
        strcpy ( path[pathCounter], input);
        pathCounter++;
        room = getIndex(input); // get index again
    }
    return true;
}
/****************************************************************************************************************
 ** Function: displayFile
 ** Description:this function displays the room, that is passed to it
 ** Parameters: room array, room and char**
 ** Pre-conditions: all of the parrameters have to exist
 ** Post-conditions: the contents of the room file get displayed
 ** Return: count
****************************************************************************************************************/
int displayFile (int* startEnd, int room, char pointer[][256] ) {
    char line[256], compare[256], buffer[256]; // the buffer chars
    FILE *read; // file ptr
    int i,count=0;
    int numLine= lineGet(list[room], read); // gets the rows in the rooomsfile
    
    read = fopen ( list[room], "r"); // open the file room

    for ( i=0; i<numLine; i++){ // rums the amount of rows in the file
        if ( i == 0 ) { // if the first line
            fgets(line, sizeof(line), read); // read in line
            strcpy ( compare , line+11 ); // get the name in the first line
            strtok(compare, "\n");
            printf( "%s%s\n", "Current Location: ", compare);
            printf( "%s", "Possible Connections: " ); 
        }
        else if ( i == numLine-2) { // if the third to last line dont print comma
            fgets(line, sizeof(line), read);
            strcpy ( compare , line+14 ); // gets the connection name
            strtok(compare, "\n");
            strcpy ( pointer[count], compare );
            printf( "%s", compare); // dont print comma, and not space
            count++;
        }
        else if ( i != 0 && i != numLine-1) { // seond to last line
            fgets(line, sizeof(line), read);
            strcpy ( compare , line+14 ); // get connections name
            strtok(compare, "\n");
            strcpy ( pointer[count], compare );
            printf( "%s,%c", compare, ' '); //prints a space, with comma
            count++;
        }
    }
    fclose( read); //clos file
    return count;
}
/****************************************************************************************************************
 ** Function: errorcheck
 ** Description: check if the input from the user is correct
 ** Parameters:  connections, input char, and comparison char **
 ** Pre-conditions: all of the parameters must exist and should not be gone
 ** Post-conditions: checks the error
 ** Return: bool
****************************************************************************************************************/
bool errorCheck (char *input, char pointer[][256], int connections) {
    bool goodNah; // bool var
    int i;

    for ( i =0; i <connections; i++ ) { // runs that amount of connections
        if ( strcmp(input, pointer[i]) == 0 ) { // if the strings are the same false
            goodNah = false;
            break; //break to exist
        }
        else if ( strcmp(input, pointer[i]) != 0 )
            goodNah = true; // if the string is not good then true
    }

    return goodNah; // return this.
}
/****************************************************************************************************************
 ** Function: getIndex
 ** Description: gets the index of an array
 ** Parameters:  input array
 ** Pre-conditions: array must exist
 ** Post-conditions: finds the array idex
 ** Return: index
****************************************************************************************************************/
int getIndex (char *input) {
    int i;
    for ( i=0; i<7; i++){ // runs 7 times
        if ( strcmp(input, list[i]) == 0 )
            return i; // return when the strings are the same
    }
}
/****************************************************************************************************************
 ** Function: getTime
 ** Description: gets the current time of the project
 ** Parameters: none
 ** Pre-conditions: time lib must exist
 ** Post-conditions: finds the time and outputs it to the file
 ** Return: void
****************************************************************************************************************/
void *getTime () {
   pthread_mutex_lock(&myMutex);
    time_t timer; // time holder
    struct tm* timeStuff; // struct to get the time information
    char buffer[256]; // read buffer
    FILE *dir = fopen("currentTime.txt", "w"); // opens the file

    time (&timer);
    timeStuff = localtime(&timer);

    strftime ( buffer, 80, " %I:%M%p, %A, %B %d, %Y", timeStuff); // prints the correct output to the file
    fprintf( dir, "%s", buffer); //now it gets printed to the file that will be used for mutex

    fclose(dir); //close
   pthread_mutex_unlock(&myMutex);
    pthread_exit(NULL);

}
/****************************************************************************************************************
 ** Function: writeTime
 ** Description: reads the file that has the time information
 ** Parameters: none
 ** Pre-conditions: there is a file that holds the time information
 ** Post-conditions: reads the file and outputs that information
 ** Return: void
****************************************************************************************************************/
void writeTime(){
    FILE* myFile; // file ptr
    myFile = fopen("currentTime.txt", "r");   //Read from the file
    char buffer[100]; // buffer
    
    if(myFile == NULL){               //currentTime.txt must exist
        perror("Not found\n"); // if the file DNE
    }
    else{
        fgets(buffer, 100, myFile);     //Else read it into buffer and print the time string
        printf("\n%s\n", buffer);
        fclose(myFile); // closes the file
    }
}