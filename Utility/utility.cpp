#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void find_length (std::string fileName){
    
    std::ifstream is (fileName, std::ifstream::binary);
    if (is) {

        std::string line;
        std::vector<int> length;
        while (std::getline(is, line)) {
            std::cout << line << std::endl;
            int pos = line.find(':');
            if(pos == EOF) 
                length.back() += line.size();
            else
                length.push_back(line.size());        
        }

        std::vector<int> row_length;
        row_length.push_back(length.front());
        for (std::vector<int>::iterator it = length.begin() + 1; it != length.end() - 1; ++it){
            //row_length.push_back(*it + *(it + 1));
            row_length.push_back(row_length.back() + *it);
        }
        for (std::vector<int>::iterator it = row_length.begin(); it != row_length.end(); ++it){
            std::cout << *it << std::endl;
        }
    }
}

int main(int argc, char *argv[]){
    
    std::string fileName;

    std::cout << "Enter the file" << std::endl;
    std::cin >> fileName;

    find_length(fileName);

    return 0;
}