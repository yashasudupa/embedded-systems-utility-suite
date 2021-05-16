#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

int str_to_int_conversion (const char *a) {
  
  int str1 = strlen(a);
  int n1 = 0;
  int i = 0;

  while(str1){
      int ch = *(a + i) - 48;
      n1 +=  ch * pow(10, str1-1);
      str1--;
      i++;
    }
  
  return n1;
}
    
void int_to_str_conversion (int a) {
  
    char **buffer;

    asprintf(buffer, "%s", a);

    printf("%s", buffer);

   // return ch;
}

char *add(const char *a, const char *b) {

    //  <----  hajime!
    
    int n1 = str_to_int_conversion (a);
    int n2 = str_to_int_conversion (b);
    
    int res = n1 + n2;

    int_to_str_conversion(res);

    //return res;
}

int main(){

    const char *ch1 = "123";
    const char *ch2 = "321"; 

    char *res = add (ch1, ch2);

    return 0;
}