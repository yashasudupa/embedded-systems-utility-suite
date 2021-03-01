#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <map>

void find_length (std::string fileName){
    
    std::ifstream is (fileName, std::ifstream::binary);
    if (is) {

        std::string line;
        std::vector<int> length;
        
        while (std::getline(is, line)) {
            int pos = line.find(':');
            if(pos == EOF) 
                length.back() += line.size();
            else
                length.push_back(line.size());        
        }

        std::ifstream is (fileName, std::ifstream::binary);
        if (is)
            std::cout << "all characters read successfully." << std::endl;
        else
            std::cout << "error: only " << is.gcount() << " could be read";
        
        std::vector<int> row_length;
        row_length.push_back(0);
        row_length.push_back(length.front() + 1);
        for (std::vector<int>::iterator it = length.begin() + 1; it != length.end() - 1; ++it){
            row_length.push_back(row_length.back() + *it + 1);
        }

        std::vector<int>::iterator row_l_it = row_length.begin();
        std::vector<int>::iterator l_it = length.begin();
        std::map<std::string, int> history;
        std::vector<std::string> line_no;

        while(row_l_it != row_length.end() ||
                l_it != length.end()){
            std::ifstream is (fileName, std::ifstream::binary);
            is.seekg (*row_l_it, is.beg);
            int l = *l_it;
            
            char *buffer = new char [l];
            
            std::cout << "Reading " << l << " characters... ";

            is.read (buffer,l);
                    
            char *token;

            token = strtok(buffer, " ");
            line_no.push_back(token);

            if(history.empty()){
                token = strtok(NULL, " ");
                history.insert({token, 1});
            }
            
            token = strtok(NULL, " ");
            while(token != NULL) {
                
                std::map<std::string, int>::iterator it;
                for(it = begin(history); it != end(history); ++it){
                    if (it->first == token){
                        it->second += 1;
                        break;
                    }
                    else if (it == --history.end()){
                        history.insert({token, 1});
                        break;
                    }
                }
                token = strtok(NULL, " ");
            }
            
            std::map<std::string, int>::iterator it;
            for(it = begin(history); it != end(history); ++it){
                std::cout << it->first << "\t";
                std::cout << it->second << std::endl;
            }
            is.close();

            delete[] buffer;
            ++row_l_it;
            ++l_it;
        }
    }
    std::cout << "Going out" << std::endl;
}

int main(int argc, char *argv[]){
    
    std::string fileName;

    std::cout << "Enter the file" << std::endl;
    std::cin >> fileName;

    find_length(fileName);

    return 0;
}