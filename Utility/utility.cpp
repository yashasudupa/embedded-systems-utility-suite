#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void find_length (std::string fileName){
    
    std::ifstream is (fileName, std::ifstream::binary);
    if (is) {

        std::string str;
        while (std::getline(is, str)) {
            std::cout << str << "\n";
        } 

        // get length of file:
        std::cout << typeid(is.end).name() << std::endl;
        std::cout << "Going out" << std::endl;
        /*
        is.seekg (0, is.end);
        int length = is.tellg();
        is.seekg (0, is.beg);

        char *buffer = new char [length];

        std::cout << "Reading " << length << " characters... ";
        // read data as a block:
        is.read (buffer,length);

        if (is)
        std::cout << "all characters read successfully." << std::endl;
        else
        std::cout << "error: only " << is.gcount() << " could be read";

        is.close();

        // ...buffer contains the entire file...

        std::cout << "Going out" << std::endl;

        delete[] buffer;
        */
    }
}

int main(int argc, char *argv[]){
    
    std::string fileName;

    std::cout << "Enter the file" << std::endl;
    std::cin >> fileName;

    find_length(fileName);

    return 0;
}