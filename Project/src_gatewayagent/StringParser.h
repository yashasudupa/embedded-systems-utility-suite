#ifndef StringParser_h
#define StringParser_h 1
#pragma warning (disable: 4251 )

#include <string>
#include "Common.h"


#define TOKEN1NOTPRESENT		-1
#define TOKEN2NOTPRESENT		-2
#define TOKEN3NOTPRESENT		-3


class StringParser 
{
  public:
      StringParser();
      ~StringParser();


      void init (std::string String, std::string Separator, 	// Separator to be used for parsing
      char EscSeq, int bytesToBeReserved = 50);

      
      int getNext2Tokens (std::string& Token1, std::string& Token2, bool RemoveEscSeq);

      //## Operation: getNextToken%3B6FE5BA0186
      int getNextToken (std::string& Token1, bool RemoveEscSeq);

      //## Operation: getNext3Tokens%3B6FE5CC0316
      int getNext3Tokens (std::string& Token1, std::string& Token2, std::string& Token3, bool RemoveEscSeq);

      //## Operation: addToken%3D3FC8040276
      void addToken (std::string Token, bool CheckForEscSeq);

      //## Operation: addInt%3DBD0CC50334
      void addInt (int Token);

      //## Operation: addLong%3D40F6DE0000
      void addLong (long Token);

      //## Operation: addDouble%3D40F70E0186
      void addDouble (double Token);

      //## Operation: addChar%3D40F7240050
      void addChar (char Token);

      const std::string getString () const;

      const char getEscSeq () const;
      void removeSpaces( std::string& ParamString );

  protected:

  private:

      int countEscSeq (int Pos, std::string Str);

      void removeEscSeq (std::string& Token, bool RemoveEscSeq);

      void insertEscSeq (std::string& Token, bool CheckForEscSeq);

  private: //## implementation
        int itsSeparatorPosition1;
        int itsSeparatorPosition2;
        std::string itsString;
        std::string itsSeparator;
        char itsEscSeq;

};

#endif
