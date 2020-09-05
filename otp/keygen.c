/*****************************************************************
 ** Name: WeiHao Kuang
 ** Date: 11.26.2018
 ** Program: keygen
 ** Description: makes a random string of Upper case letter and spaces 
                 user defined
 ** Input: number of random chars argc = 2
 ** Output: Random letters printed out in string
*******************************************************************/
#include "stdio.h"
#include "stdlib.h"

/*****************************************************************
 ** Function: main
 ** Description: main driver
 ** Parameters: int argc and char** argv 
 ** Input: argv
 ** Output: Random letters printed out in string
*******************************************************************/
int main ( int argc, char** argv ){
    srand(time(NULL)); // seeding srand with time
    int character=0; // initializing an int
    int i;
    
    // if the usage if incorrect exit 0, but say the usage correctly
    if ( argc != 2 ){printf("Usage: %s numChar\n", argv[0]); exit(0);};

    for ( i=0; i< atoi(argv[1]); i++){ // for the number user defs
        character = ( rand() % (90 - 32 + 1)) + 32; // choosing charaters
        if ( character == 32) // if the charater is a space '32'
            printf("%c", character);
        // if character is a non space from "A-Z"
        else if ( character > 32 && character >= 65 ){ 
            printf("%c", character);
        }
        else { // otherwise randomly pick again
            while (character > 32 && character < 65 ){
                character = ( rand() % (90 - 32 + 1)) + 32;
            }
            printf("%c", character);
        }
    }
    printf("\n"); // adds new line
}