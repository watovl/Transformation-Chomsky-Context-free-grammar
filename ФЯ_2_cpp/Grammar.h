#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "Alphabet.h"
#include <vector>
#include <string>

typedef std::vector <std::vector <std::string> > dim2vector;

struct Grammar {
	Alphabet alphabet;
	dim2vector productionRules;
	std::string startSymbol;
};

#endif // !GRAMMAR_H