/**************************************************************************************************************
 ** Name: WeiHao Kuang
 ** Date: October 21, 2018
 ** Program Name and Description: buildroom with pid, using file I/O
 ** Input: Randomly generate rooms, "random" input
 ** Return: 7 rooms files, that are housed in one room folder with pid as name (folder)
 ** References: https://cboard.cprogramming.com/c-programming/165757-using-process-id-name-file-directory.html
                http://mathworld.wolfram.com/AdjacencyMatrix.html
                https://oregonstate.instructure.com/courses/1725991/pages/2-dot-2-program-outlining-in-program-2
                https://oregonstate.instructure.com/courses/1725991/pages/2-dot-5-arrays-vs-pointers
***************************************************************************************************************/
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "stdlib.h"
#include "stdbool.h"

// making the global variables 
int connections[7][7]; // global adjacency matrix

// hardcoded roomeNames
const char *roomNames[] = { "Lakers", "Bulls", "Celtics", "Raptors", "Nets", 
                          "Nuggets", "Spurs", "Heat", "Suns", "Wizards"};
// Hardcoded room types;
const char *roomTypes[] = { "START_ROOM", "MID_ROOM", "END_ROOM" };
// hardcoded prefix
const char folderName[] = "kuangw.rooms.";

int *roomsArray ();
void randType (int*);
bool graphStatus();
void addConnection(int*);
int randomRoom( int*);
bool ableToAdd(int);
bool connectionExists(int, int);
void roomConnection(int, int);
bool isSame(int, int);

/****************************************************************************************************************
 ** Function: main 
 ** Description: runs all of the functions and the heart of the program call
 ** Parameters: none, for this implementation
 ** Pre-conditions: complilation of this have to be completed and the executable has to run
 ** Post-conditions: the game files will be made
 ** Return: 0
****************************************************************************************************************/
int main () {
    // initializing all of the need variable for main
    int *roomsUsed = NULL; // roomsUsed pointer
    int randRoomType[7] = {0,2,1,1,1,1,1}; // this arary is always shuffled, to ensure random room types always!
    int i, j, number, count=1;
    srand(time(NULL));
    FILE* rooms; // file pointer for file reading and writing
    
    int processID = getpid(); // getting the Process ID
    
    char fileName[sizeof(processID)+sizeof(folderName)+10]; // making a buffer array to hold the name of directory
    char roomS[256]; // used as buffer for making the directoy name
    
    snprintf(fileName, sizeof(fileName), "%s%d", folderName, processID); // puts folder prefix with PID
    mkdir(fileName, 0755); // making a directory with permissions

    sprintf(roomS, "./%s", fileName); // making the name so that I change directories to the newly created directory
    chdir( roomS ); // changing directorys to the newly made directory for file making

    roomsUsed = roomsArray(); // this will hold the 7 names that are randomly generated each time the game runs.

    // this while loop will use the graphing function from the Canvas webpage to make random connections between
    // rooms, at least 3 and at most 6
    while (graphStatus() == false) {
       addConnection(roomsUsed);
    }

    randType (randRoomType); // this function is responsible for randomly choosing roomTypes

    // this big for loop is used for file printing, I use this for loop to store all of the data from
    // the graph and random array to the newly created file.
    for (i=0; i<7; i++){ // runs 7 times
        // next three lines are used to store the room name to the file
        number = roomsUsed[i]; 
        rooms = fopen( roomNames[number], "w");
        fprintf ( rooms, "%s%s\n", "ROOM NAME: ", roomNames[number] ); // print to the rooom name to the file

        for ( j=0; j <7; j++){ // loop runs 7 times to print the connections
            if ( connections[i][j] == 1) // in the adj matrix, if its is 1 then there is connection.
            // print to the file with connection data 
               fprintf ( rooms, "%s%d%s%s\n", "CONNECTION ", count++ ,": ", roomNames[roomsUsed[j]] );
        }
        count =1; // reset the counter variable for connection number

        // next three conditions check the room type and print that accodiningly
        // the array that is used the RandType array.
        if ( i == 0 )
            fprintf ( rooms, "%s%s\n", "ROOM TYPE: ", roomTypes[0] ); //printing data to file
        else if  ( i == 2 ) 
            fprintf ( rooms, "%s%s\n", "ROOM TYPE: ", roomTypes[2] ); //printing data to file
        else
            fprintf ( rooms, "%s%s\n", "ROOM TYPE: ", roomTypes[1] ); //printing data to file

        fclose (rooms);
    }
    free( roomsUsed );// freeing memory

    return 0;
}

/****************************************************************************************************************
 ** Function roomsArray
 ** Description: returns an array that randomly chooses which hardcoded room names are used
 ** Parameters: none
 ** Pre-conditions: must be called in main, and assume that malloc works, and srand is seeded
 ** Post-conditions: array is made, and the randomly selected indexes are used
 ** Return: a dynamically allocated array of room names that are gonna be used.
****************************************************************************************************************/
int *roomsArray () {
    int i, temp, randomIndex;
    int array[10];
    int *rooms = malloc(7*sizeof(int)); // allocating memory

    // fill up an array with 1-10, to be used for shuffling
    for (i = 0; i < 10; i++) {
        array[i] = i;    
    }

    // shuffling the array to ensure random rooms names everyime
    for (i = 0; i < 10; i++) { 
        temp = array[i];
        randomIndex = rand() % 10; // using random function get names
        array[i] = array[randomIndex];
        array[randomIndex] = temp;
    }

    // after I shuffle the array I choose the first 7 of the 10, 
    // always will be random
    for (i = 0; i < 7; i++){
        rooms[i] = array[i];
    }
    return rooms;
}

/****************************************************************************************************************
 ** Function: randType
 ** Description: makes a 7 sized random array to choose rooms 
 ** Parameters: randRoomType hard coded array
 ** Pre-conditions: There has to be randRoomType variable exisit
 ** Post-conditions: the hardcoded global array will be edited, and then random room types will be assigned
 ** Return: none
****************************************************************************************************************/
void randType (int *randRoomType) {

    int i, temp, randomIndex;
    for (i = 0; i < 7; i++) {    // shuffle array, runing  7 times
        temp = randRoomType[i];
        randomIndex = rand() % 7; // choose randomely between 0 and 6
        randRoomType[i] = randRoomType[randomIndex];
        randRoomType[randomIndex] = temp;
    }
}

/****************************************************************************************************************
 ** Function: graphStatus
 ** Description: this returns whether the graph is full or not, helper function for graph
 ** Parameters: none
 ** Pre-conditions: an adjaceny matrix must exist and must be a nxn
 ** Post-conditions: the matrix the status of the matrix will get returned
 ** Return: true if the full, and false is the not full
 ** Note: this function is from the OS canvas readings and example code
****************************************************************************************************************/
bool graphStatus() {
    int i, j;
    int counter=0;

    // double for loops to step through the adj matrix
    for ( i=0; i<7; i++){
        for ( j=0; j<7; j++) {
            // check if the 1 if the one than increment counter
            if ( connections[i][j] == 1 )
                counter++;
        }
        // if the counter is less than 3 and greater than 6 graph is not full
        if (counter < 3 || counter > 6)
            return false;
        else
            counter = 0; // otherwise you get to move on to the next room 
    }
    // if false isnt returned in the function then the graph is full
    return true;
}

/****************************************************************************************************************
 ** Function: addConnection
 ** Description: This will make random connections between rooms
 ** Parameters: int *roomsUsed, used a refeernces
 ** Pre-conditions: there must be an adjacency matrix that already exists
 ** Post-conditions: a connection will be added to the adj matrixs
 ** Return: none
 ** Note: this function is from the OS canvas readings and example code
****************************************************************************************************************/
void addConnection(int *roomsUsed) {
    int A;  // int connection variable
    int B; // int conneciotn variable

    while(true) {
        A = randomRoom( roomsUsed ); // random room getting

        if (ableToAdd(A) == true) // check to see if we can add a connection to the graph
        break;
    }

    do { // only i can add connection returns false or if the room is that same or connextion already exists
        B = randomRoom( roomsUsed ); // get the second random number to double connect the adj matrix
    } while(ableToAdd(B) == false || isSame(A, B) == true || connectionExists(A, B) == true);

    roomConnection(A, B);  // TODO: Add this connection to the real variables, 
    roomConnection(B, A);  //  because this A and B will be destroyed when this function terminates
}

/****************************************************************************************************************
 ** Function: randomRoom
 ** Description: Returns a random Room, does NOT validate if connection can be added
 ** Parameters: roomUsed
 ** Pre-conditions: roomUsed used as reference 
 ** Post-conditions: the random room will get choosen
 ** Return: the random room choosen
 ** Note: this function is from the OS canvas readings and example code
****************************************************************************************************************/
int randomRoom( int *roomsUsed ) {
    int randomness; 
    randomness = rand()%7;
    return randomness;
}

/****************************************************************************************************************
 ** Function: ableToAdd
 ** Description: Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
 ** Parameters: int B
 ** Pre-conditions: graph must exist, adj must be there 
 ** Post-conditions: checks the able to add fucntionality of the graph
 ** Return: true or false
 ** Note: this function is from the OS canvas readings and example code
****************************************************************************************************************/
bool ableToAdd(int B) {
    int i;
    int counter=0;
    
    for ( i = 0; i<7; i++){ // using a set room check to see if the connections are less than 6
        if ( connections[B][i] == 1)
            counter++;
    }

    if ( counter < 6 ) // if the connections are less than 6 return can add
        return true;
    else
        return false; // if the connections are greater than 6 retuen cannot add
}

/****************************************************************************************************************
 ** Function: connectionExists
 ** Description: Returns true if a connection from Room x to Room y already exists, false otherwise
 ** Parameters: A and B
 ** Pre-conditions: graph must exist, adj must be there 
 ** Post-conditions: checks to see if the connections exists of the graph
 ** Return: true or false
 ** Note: this function is from the OS canvas readings and example code
****************************************************************************************************************/
bool connectionExists(int A, int B) {
    if ( connections[A][B] == 1 ) // if they the same return true
        return true;
    else // if not then false
        return false;
}

/****************************************************************************************************************
 ** Function: roomConnection
 ** Description: Connects Rooms x and y together, does not check if this connection is valid
 ** Parameters: A and B
 ** Pre-conditions: graph must exist, adj must be there 
 ** Post-conditions: makes the actual connection to on the adj matrix
 ** Return: none
 ** Note: this function is from the OS canvas readings and example code
****************************************************************************************************************/
void roomConnection(int A, int B) {
    connections[A][B] = 1;
}

/****************************************************************************************************************
 ** Function: ableToAdd
 ** Description: Returns true if Rooms x and y are the same Room, false otherwise
 ** Parameters: int B, int A
 ** Pre-conditions: graph must exist, adj must be there 
 ** Post-conditions: checksis the same fucntionality of the graph
 ** Return: true or false
 ** Note: this function is from the OS canvas readings and example code
****************************************************************************************************************/
bool isSame(int A, int B) {
    if ( A == B ) // if the same then return true
        return true;
    else // else not true
        return false;
}