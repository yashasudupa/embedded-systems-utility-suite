#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define SIZE 100

int main(int argc, char *argv[]){

    char line[SIZE];

    char ps_1[] = {'R', 'G', 'Y'};
    char ps_2[] = {'P', 'C'};

    printf("Enter the sequence \n");
    //getline(&line, &len, stdin);

    gets(line);

    char *ch = strdup(line);
    char *rest = ch;
    char *token; 
    token = strtok_r(rest, " ", &rest);
    assert((ps_1[0] == *token) && "Invalid sequence or ERROR");
    while(rest != NULL){
        token = strtok_r(rest, " ", &rest);
        switch(*token){
            case 'R':
                assert(((*(token - 2)  == ps_1[2]) || 
                      (*(token - 2)  == ps_2[0]) ||
                      (*(token - 2)  == ps_2[1])) && "Invalid sequence or ERROR");
                break;

            case 'G': 
                assert((*(token - 2) == ps_1[0]) && "Invalid sequence or ERROR");
                break;

            case 'Y':
                assert((*(token - 2) == ps_1[1]) && "Invalid sequence or ERROR");
                break;

            case 'P':
                assert((*(token - 2) == ps_1[0]) && "REJECT or ERROR");
               break;

            case 'C':
                assert((*(token - 2) == ps_1[0]) && "REJECT or ERROR");
                break;

            default:
                printf("ERROR");
                exit(1);
        }
    }
    printf("ACCEPT\n");
}