
#include "StringParser.h"
#define VALUESEPERATOR					","

StringParser::StringParser()
  //## begin StringParser::StringParser%3B6F8A3D037A_const.hasinit preserve=no
      : itsSeparatorPosition1(-1),
        itsSeparatorPosition2(0),
        itsString(""),
        itsSeparator(VALUESEPERATOR),
        itsEscSeq('/')
{
  
}


StringParser::~StringParser()
{
}


void StringParser::init (std::string String, std::string Separator, char EscSeq, int bytesToBeReserved)
{
  //## begin StringParser::init%3B6F8B60033E.body preserve=yes
	itsSeparatorPosition1 = -1;
	itsSeparatorPosition2 = 0;
	// check that separator and esc seq are not same
	if ( Separator.length() == 1 && Separator[0] == EscSeq || 
		Separator.find( EscSeq ) != Separator.npos )
	{
		std::string ErrStr = " Same character is not allowed for Separator (" + Separator +
			") and Escape sequence (" + EscSeq + ")";
	}
	itsSeparator = Separator;
	itsEscSeq = EscSeq;
	itsString = String;
	itsString.reserve( bytesToBeReserved );
  //## end StringParser::init%3B6F8B60033E.body
}

//## Operation: getNext2Tokens%3B6FDFFF01B8
//	Get next two tokens from the string. See description of getNextToken for
//	details.
//
//	Return value
//	SUCCESS
//	TOKEN1NOTPRESENT
//	TOKEN2NOTPRESENT
int StringParser::getNext2Tokens (std::string& Token1, std::string& Token2, bool RemoveEscSeq)
{
  //## begin StringParser::getNext2Tokens%3B6FDFFF01B8.body preserve=yes
	int res = SUCCESS;

	// @ flow 0 | get first token
	res = getNextToken( Token1, RemoveEscSeq );
	if ( res == SUCCESS )
	{
		// @ flow 0 | get second token
		res = getNextToken( Token2, RemoveEscSeq );
		if ( res == TOKEN1NOTPRESENT )
		{
			res = TOKEN2NOTPRESENT;
		}
	}
	return res;
  //## end StringParser::getNext2Tokens%3B6FDFFF01B8.body
}

//## Operation: getNextToken%3B6FE5BA0186
//	Get next token from the string
//	The escape sequence char itself may be part of token data . It is also
//	preceded by an esc seq char. The flag RemoveEscSeq indicates whether String
//	Parser should remove this esc seq character from the returned token. This
//	flag should be true only if the token does not contain any further
//	sub-tokens  which have a different separator, but same esc sequence.
//
//	The esc seq character preceding the separator is always removed before
//	returning the token.
//
//	Return value
//	SUCCESS
//	TOKEN1NOTPRESENT
int StringParser::getNextToken (std::string& Token1, bool RemoveEscSeq)
{
  //## begin StringParser::getNextToken%3B6FE5BA0186.body preserve=yes
	// @ flow 0 | return empty string if string to be parsed is empty
	// @ flow 0 | or separator2 is already at the end of string
	if ( itsString.length() == 0 || itsSeparatorPosition2 == itsString.npos )
	{
		return TOKEN1NOTPRESENT;
	}
	// find separator, but check that it is not preceded by esc sequence 
	// separator preceded by esc seq means it is part of data 
	// if esc seq is part of data, it is also preceded by another Escseq
	// hence for seperator to be a real separator, it must be preceded by even number of esc seq
	// string - value//# (# is separator), but value///# (# is part of data)
	bool found = false;
	int pos = itsSeparatorPosition1;
	while (!found && itsSeparatorPosition2 != itsString.npos )
	{
		itsSeparatorPosition2 = itsString.find( itsSeparator, pos+1 );
		if ( countEscSeq( itsSeparatorPosition2, itsString ) % 2 == 0 ) 
		{
			found = true;
		}
		else
		{
			pos = itsSeparatorPosition2;
		}
	}
	// @flow 0 | extract token
	int val = itsSeparatorPosition2 - itsSeparatorPosition1 - 1;
	if(val==0)
	{
		Token1 = "";
	}
	else
	{
		
		Token1 = itsString.substr( itsSeparatorPosition1 + 1 , 
		itsSeparatorPosition2 - itsSeparatorPosition1 - 1 );
		
		// if there was a trailing separator without any data after it, treat as token not present
		if ( Token1.length() == 0 && itsSeparatorPosition2 == itsString.npos )
		{
			return TOKEN1NOTPRESENT;
		}
	}
	// @ flow 0 | update separator index positions
	// separator can be more than 1 char, hence update itsSeparatorPosition1 accordingly
	itsSeparatorPosition1 = itsSeparatorPosition2 + itsSeparator.length() - 1;
	// @ flow 0 | remove leading and trailing spaces
	if(Token1 != "")
		removeSpaces( Token1  );
	// if remove escape sequence flag is set, remove it from token  
	// Calling object should send this flag true only if this token does not 
	// contain any sub tokens with same esc seq
	removeEscSeq( Token1, RemoveEscSeq );

	return SUCCESS;
  //## end StringParser::getNextToken%3B6FE5BA0186.body
}

//## Operation: getNext3Tokens%3B6FE5CC0316
//	Get next three tokens from the string. See description of getNextToken for
//	details.
//
//	Return value
//	SUCCESS
//	TOKEN1NOTPRESENT
//	TOKEN2NOTPRESENT
//	TOKEN3NOTPRESENT
int StringParser::getNext3Tokens (std::string& Token1, std::string& Token2, std::string& Token3, bool RemoveEscSeq)
{
  //## begin StringParser::getNext3Tokens%3B6FE5CC0316.body preserve=yes
	int res = SUCCESS;

	// @ flow 0 | get first token
	res = getNextToken( Token1, RemoveEscSeq );
	if ( res == SUCCESS )
	{
		// @ flow 0 | get second token
		res = getNextToken( Token2, RemoveEscSeq );
		if ( res == SUCCESS )
		{
			// @ flow 0 | get third token
			res = getNextToken( Token3, RemoveEscSeq );
			if ( res == TOKEN1NOTPRESENT )
			{
				res = TOKEN3NOTPRESENT;
			}
		}
		else
		{
			res = TOKEN2NOTPRESENT;
		}
	}
	return res;
  //## end StringParser::getNext3Tokens%3B6FE5CC0316.body
}

//## Operation: countEscSeq%3D3FC7090230
//	This function counts number of escape sequences preceding a given position
//	in string. If there are 0 or even number of esc sequence characters
//	preceding a separator, then the separator is a real separator else it is
//	part of data.
//
//	returns
//	number of esc sequences found
int StringParser::countEscSeq (int Pos, std::string Str)
{
  //## begin StringParser::countEscSeq%3D3FC7090230.body preserve=yes
	// check if itsSeparatorPosition2 is already at npos
	if ( Pos == Str.npos )
	{
		return 0;
	}

	bool ret = true;
	int Count = 0;
	// check if characters preceding itsSeparatorPosition2 are esc seq
	if ( Str[Pos-1] == itsEscSeq )
	{
		int pos1 = Pos - 1;
		while ( pos1 != Str.npos && Str[pos1] == itsEscSeq )
		{
			Count++;
			pos1--;
		}
	}
	return Count;
}

//	Remove all occurances of escape sequences that are preceding separator. If
//	RemoveEscSeq  flag is true, esc sequence preceding esc seq is also removed,
//	otherwise it is kept as it is.
void StringParser::removeEscSeq (std::string& Token, bool RemoveEscSeq)
{
  //## begin StringParser::removeEscSeq%3D3FC76202F8.body preserve=yes
	int pos = 0, pos1 = 0;
	// search for escseq + separator
	std::string str = itsEscSeq + itsSeparator;
	while ( pos != Token.npos )
	{
		if ( ( pos = Token.find( str, pos1 ) ) != Token.npos )	// escape sequence found
		{
			// remove the escape sequence
			Token.erase( pos, 1 );
			pos1 = pos + itsSeparator.length();
		}
	}

	// remove esc seq preceding other esc seq based on flag
	if ( RemoveEscSeq )
	{
		// search for escseq + escseq
		// but ignore if there is a separator immediately following esc sequences. 
		// this is already processed above
		pos = 0;
		pos1 = 0;
		str = itsEscSeq;	// CAUTION - statement str = itsEscSeq + itsEscSeq; will add two ints and assign to str
		str += itsEscSeq;
		while ( pos != Token.npos )
		{
			if ( ( pos = Token.find( str, pos1 ) ) != Token.npos )	// escape sequence found
			{
				// remove one escape sequence character
				Token.erase( pos, 1 );
				pos1 = pos + 1;			// position after the esc seq that is aprt of data
			}
		}
	}
  //## end StringParser::removeEscSeq%3D3FC76202F8.body
}

//## Operation: addToken%3D3FC8040276
//	Append given token at the end of curent string. If separator is part of
//	string, add esc seq before the separator.
//	If the flag CheckForEscSeq is true, then if esc seq is part of token,
//	precede it by esc seq. This flag should be true only for innermost tokens
//	(i.e. tokens not containing any subtokens)
void StringParser::addToken (std::string Token, bool CheckForEscSeq)
{
  //## begin StringParser::addToken%3D3FC8040276.body preserve=yes
	std::string Token1 = Token;
	
	// check for separator or esc seq as part of token
	insertEscSeq( Token1, CheckForEscSeq );

	// append token at the end of itsString
	itsString += Token1;
	itsString += itsSeparator;
  //## end StringParser::addToken%3D3FC8040276.body
}

//## Operation: addInt%3DBD0CC50334
//	Add one token of type int to itsString
void StringParser::addInt (int Token)
{
  //## begin StringParser::addInt%3DBD0CC50334.body preserve=yes
	char str[32];
	sprintf( str, "%d", Token );

	// check for separator or esc seq as part of token
	std::string Token1 = str;
	insertEscSeq( Token1, true );

	itsString += Token1;
	itsString += itsSeparator;
  //## end StringParser::addInt%3DBD0CC50334.body
}

//## Operation: addLong%3D40F6DE0000
//	Add one token of type long to itsString
void StringParser::addLong (long Token)
{
  //## begin StringParser::addLong%3D40F6DE0000.body preserve=yes
	char str[32];
	sprintf( str, "%ld", Token );

	// check for separator or esc seq as part of token
	std::string Token1 = str;
	insertEscSeq( Token1, true );

	itsString += Token1;
	itsString += itsSeparator;
  //## end StringParser::addLong%3D40F6DE0000.body
}

//## Operation: addDouble%3D40F70E0186
//	Add one token of type double to itsString
void StringParser::addDouble (double Token)
{
  //## begin StringParser::addDouble%3D40F70E0186.body preserve=yes
	char str[32];
	sprintf( str, "%f", Token );

	// check for separator or esc seq as part of token
	std::string Token1 = str;
	insertEscSeq( Token1, true );

	itsString += Token1;
	itsString += itsSeparator;
  //## end StringParser::addDouble%3D40F70E0186.body
}

//## Operation: addChar%3D40F7240050
//	Add one token of type char to itsString
void StringParser::addChar (char Token)
{
  //## begin StringParser::addChar%3D40F7240050.body preserve=yes
	char str[2];
	sprintf( str, "%c", Token );

	// check for separator or esc seq as part of token
	std::string Token1 = str;
	insertEscSeq( Token1, true );

	itsString += Token1;
	itsString += itsSeparator;
  //## end StringParser::addChar%3D40F7240050.body
}

//## Operation: insertEscSeq%3D8EC7D5019A
//	Check if sepaerator is part of token and precede it with escape sequence. If
//	argument, CheckForEscSeq is true, check if escape seq is part of token and
//	precede that also with an esc seq.
//	This flag should be tru only for innermost tokens (i.e. tokens not
//	containing any subtokens).
void StringParser::insertEscSeq (std::string& Token, bool CheckForEscSeq)
{
  //## begin StringParser::insertEscSeq%3D8EC7D5019A.body preserve=yes

	char EscSeq[2];
	sprintf( EscSeq, "%c", itsEscSeq );

	// check for separator or esc seq as part of token
	int pos = 0, pos1 = -1;
	for ( int i = 0; i < (int)Token.length(); i++ )
	{
		if ( CheckForEscSeq && Token[i] == itsEscSeq )
		{
			Token.insert( i++, EscSeq );
		}
		if ( Token[i] == itsSeparator[0] && (int)Token.find(itsSeparator, pos) == i )
		{
			Token.insert( i++, EscSeq );
			pos = i + 1;	// next find should start after current separator occurance
		}
	}
  //## end StringParser::insertEscSeq%3D8EC7D5019A.body
}

//## Get and Set Operations for Class Attributes (implementation)

const std::string StringParser::getString () const
{
  //## begin StringParser::getString%3B6F8B4B0208.get preserve=no
  return itsString;
  //## end StringParser::getString%3B6F8B4B0208.get
}

const char StringParser::getEscSeq () const
{
  //## begin StringParser::getEscSeq%3D3EAB5A0244.get preserve=no
  return itsEscSeq;
  //## end StringParser::getEscSeq%3D3EAB5A0244.get
}


void StringParser::removeSpaces( std::string& ParamString )
{
	int PosSpace = 0;
	int PosSpace1 = 0;
	int  Length = ParamString.length();
	const char *TempBuf = ParamString.c_str();

	//removeleading spaces
	while ( TempBuf[PosSpace++] == ' ' );
	// get the string after removing leading spaces
	ParamString = ParamString.substr( PosSpace-1, Length );

	// ignore spaces in middle and remove only trailing spaces
	while ( true )
	{
		PosSpace = ParamString.find( " ", PosSpace1 );
		// check if there are nonspace characters after this space
		if ( PosSpace == -1 || ParamString.find_first_not_of( " ", PosSpace ) == -1 )
		{
			break;
		}
		PosSpace1 = PosSpace+1;
	}
	if ( PosSpace > 0 )
	{
		// get the string after removing trailing spaces
		ParamString = ParamString.substr( 0, PosSpace );
	}
}