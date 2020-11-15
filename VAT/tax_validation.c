#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "tax_validation.h"
#define r 11

int tax_validation (char *line){
    
    int output=0;
    char *ch = strdup(line);
    char *token = strtok(ch, " ");

    assert((strlen(token) == 8) || (strlen(token) == 9) && "Invalid string length");

    int st = (token[0] == '0') ? 0 : 1; 
    int sum = (st)*(*token - '0');

    for(int i=1; i<strlen(token); i++){
        sum += (token[i] - '0') * pow(2, i);
    }

    printf("Sum of the element is %d\n", sum);
    int Y = sum % r;
    printf("Y of the element is %d\n", Y);

    if(((Y == 10) && (token[0] - '0' == 0)) || (Y == token[0])){
        printf("VAT number is numerically valid \n");
        unsigned int n;
        token = strtok(NULL, "\n");
        int st_i = strlen(token) - 1;
        for(int i=0; i < st_i; i++){
            output += (token[i] - '0') * pow(10, st_i - i);
        }
    }
    else{
        printf("VAT number is not valid \n");
    }
    return output;
}
