/* 
 * Tax validation code
 * 
 * author : Yashas Nagaraj Udupa
*/

#include <stdio.h>
#include "tax_validation.h"

#define SIZE 256

int main(int argc, char *argv[]){

    if(argc > 1){
        char const* const fileName = argv[1];
        FILE *file = fopen(fileName, "r");
        char line[SIZE];

        int euro_valid = 0;
        while(fgets(line, sizeof(line), file)){
            euro_valid += tax_validation (line);
        }
        printf("Total result is %d\n", euro_valid);
    }
    else{
        printf("File is not entered \n");
    }
    return 0;
    
}