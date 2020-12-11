/* MBus_decoder.c 
 * Author : Yashas Nagaraj Udupa 
 */
/***

Decode the Mbus message and print a human readable
manufacturer code and serial number.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

// Upper limit of Manufacturer ID and byte array size is declared
#define MAN_ID_SIZE 2
#define ARRAY_SIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))

// ASCII declarations
#define ASC_A 65
#define ASC_Z 90
#define ASC_0 48
#define ASC_9 57
#define ASC_a 97
#define ASC_z 122

/***
Handler that decodes the encoded Manufacturer's code.

Decoding logic : (ASCII letter at its right position / (32 ^ pos)) + 64
e.g : (0xA000/ 0x32 ^ 2) + 64
The above decoding logic is deduced from the encoding logic.
Logic is computed for every alphabet and the decoded character is 
concatenated to a string.
Note : Magic numbers such as j=2, pow(0x32, j) etc. are used for the  
convenience of the computation

Returns decoded Manufacturer code.
*/
static char* decode_manufacturer(const uint8_t* data){

    // Heap memory allocation for manufacturer's code
    char *id = (char *) calloc(MAN_ID_SIZE, sizeof(char)); 
    memset(id, 0 , MAN_ID_SIZE * sizeof(char));

    // Declaration of variables that support in decoding the byte array 
    uint8_t mask = 0xf0;
    uint32_t ch;
    char *ptr_to_ch;

    // Decodes 1st and 2nd letter with the defined logic  
    for(int i=0, j=2; i < (MAN_ID_SIZE - 1)*2; i++, j--){
        ch = (((*data & mask) * pow(0x10, 2)) / pow(0x32, j)) + 64;
        mask = mask >> 4;
        ptr_to_ch = (char *) &ch;
        assert((ch >= ASC_A && ch <= ASC_Z) || (ch >= ASC_0 && ch <= ASC_9) ||
               (ch >= ASC_a && ch <= ASC_z) && "Unrecognized encoding");
        strcat(id, ptr_to_ch);
    }

    // Decodes 3rd ASCII letter with the defined logic
    ch = data[MAN_ID_SIZE - 1] + 64;
    ptr_to_ch = (char *) &ch;
    assert((ch >= ASC_A && ch <= ASC_Z) || (ch >= ASC_0 && ch <= ASC_9) ||
           (ch >= ASC_a && ch <= ASC_z) && "Unrecognized encoding");
    strcat(id, ptr_to_ch);

    // 4th character is assumed as '-' to 
    // supposedly act as a seperating character that 
    // seperates Manufacturer's ID from serial number
    ch = '-';
    strcat(id, ptr_to_ch);
    return id;
}

/***
Handler that decodes the encoded Serial number.

Implemented logic : The encoded hexadecimal
serial number is inversed and converted into original
value (decimal) with the suitable logic.
Note : Magic numbers such as 0x10, 10 etc. are used for the  
convenience of the computation

Returns decoded serial key.
*/
static int32_t* decode_serial(const uint8_t* data, uint8_t arr_size){

    // Heap memory allocation for serial number
    uint32_t *id = (uint32_t *) malloc(sizeof(uint32_t)); 
    memset(id, 0 , sizeof(uint32_t));

    // Logic to decode the encoded BCD value into decimal value.
    for(int i = arr_size - 1, j = arr_size; i > MAN_ID_SIZE - 1; i--, j=j-2){
        if (data[i] >= 0x10){
            uint32_t data_int = (data[i]/ 0x10)*10;
            data_int += data[i] % 0x10;
            *id += data_int * (uint32_t)pow(10, j);
        }
        else{
            *id += data[i] * (uint32_t)pow(10, j);
        }
    }  
    return id;
}

/***
Invokes decode_manufacturer_code and decode_serial_number handlers

Prints human readable manufacturer code and serial number

*/
static void printinfo(const uint8_t* encoded_data, uint8_t arr_size){
    
    // Data pointer to the encoded byte array 
    const uint8_t *data = encoded_data;

    // Test cases
    assert(data != NULL && "Cannot be an empty array");
    assert(arr_size == 6 && "Invalid size of data array");

    // Process the data
    char *manufacturer_id = decode_manufacturer(data);
    uint32_t *serial_no = decode_serial(data, arr_size);

    // Prints or logs human readable manufacturer code
    // and serial number
    printf("%s", manufacturer_id);
    printf("%lu \n", (unsigned long) *serial_no);

    // Deallocation of the memory from the heap 
    free(manufacturer_id);
    free(serial_no);
}

int main (int argc, char **argv){
    
    // 6 byte encoded data array
    uint8_t encoded_data[] = {0xA5, 0x11, 0x06, 0x05, 0x60, 0x70};
    printinfo(encoded_data, ARRAY_SIZE(encoded_data));
}