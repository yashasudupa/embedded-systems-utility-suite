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

void parser (std::string fileName){
    
    std::ifstream is (fileName, std::ifstream::binary);

    std::string line;
    std::string word;
    std::vector<std::map<std::string, int>> history;
    std::map<std::string, int> line_no;

    while (std::getline(is, line)) 
    {
        std::stringstream ss(line);
        std::string token;
        std::getline(ss, token, ' ');
        
        line_no.push_back(token, 1);

        int pos = token.find(':');
        if(pos != EOF) 
            history.push_back({token, 1});     

        while(std::getline(ss, token, ' ')){
            std::vector<std::map<std::string, int>>::iterator it;
            for(it = begin(history); it != end(history); ++it)
            {
                for(auto& [key, value] : *it){
                    if(!strcmp(key, token)){
                        value += 1;
                    }
                    else if(key == it->end()->first){
                        history.back().insert({token, 1});
                    } 
                }
            }
        }
    }
}

int main(int argc, char *argv[]){
    
    std::string fileName;

    std::cout << "Enter the file" << std::endl;
    std::cin >> fileName;

    parser(fileName);

    return 0;
}