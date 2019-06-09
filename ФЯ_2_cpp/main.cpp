#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <set>
#include <algorithm>
#include "Grammar.h"


// ������������� ������������
std::string shielding(const std::string &string) {
	std::string result = string;
	const std::regex character("([\\-\\^\\$\\\\\\.\\*\\+\\?\\(\\)\\[\\]\\{\\}\\|])");
	std::sregex_iterator iter(string.begin(), string.end(), character), iterEnd;
	for (int count = 0; iter != iterEnd; ++iter, ++count) {
		result.insert((*iter).position(0) + count, "\\");
	}
	return result;
}

// ����������-��������� ����������
bool contextFree(const dim2vector &rules, const Alphabet &alphabet) {
	for (auto rule : rules) {
		if (!std::regex_match(rule.front(), std::regex("^[" + alphabet.nonterminal + "]$")))
			return false;
	}
	return true;
}

// ���������� ����������
bool regular(const dim2vector &rules, const Alphabet &alphabet) {
	int rightRegular = 1;
	for (auto rule : rules) {
		if (!std::regex_match(rule.front(), std::regex("^[" + alphabet.nonterminal + "]$")))
			return false;
			// �� ������������ � ���� �������������
			if (!std::regex_match(rule.back(), std::regex("^[" + alphabet.nonterminal + "]?[" + alphabet.terminal + "]+$")) || rightRegular == 2) {
				// �� ������������� � ���� ������������
				if (!std::regex_match(rule.back(), std::regex("^[" + alphabet.terminal + "]+[" + alphabet.nonterminal + "]?$")) || rightRegular == 0) {
					return false;
				}
				// �������������
				else
					rightRegular = 2;
			}
			// ������������
			else
				rightRegular = 0;
	}
	return true;
}

// ����� ����������
void showGrammar(const Grammar &grammar) {
	Grammar tempGrammar = grammar;
	std::cout << "\t���������: " << tempGrammar.alphabet.terminal << std::endl;
	std::cout << "\t�����������: " << tempGrammar.alphabet.nonterminal << std::endl;
	std::sort(tempGrammar.productionRules.begin(), tempGrammar.productionRules.end());
	std::cout << "\t�������:" << std::endl;
	std::for_each(tempGrammar.productionRules.begin(), tempGrammar.productionRules.end(), [](const std::vector<std::string> &line) {
		std::cout << "\t\t" << line.front() << " -> " << line.back();
		std::cout << std::endl;
	});
	std::cout << "\t��������� ������: " << tempGrammar.startSymbol << std::endl;
}

// ����������� ���� �������(0 - nonterminal, 1 - terminal)
bool typeSymbol(const char &symbol, const Alphabet &alphabet) {
	return (alphabet.nonterminal.find(symbol) == -1);
}

// ����� ���������� �����������
std::string unoccupiedNonterminal(const std::string &nonterminal) {
	char startSymbol;
	for (int i = 0;; ++i) {
		startSymbol = char(65 + i);
		if (nonterminal.find(startSymbol) == -1)
			break;
	}
	return std::string(1, startSymbol);
}


// �������� ������� �����
bool emptyLanguage(const Grammar &grammar) {
	std::string dataSymbols = grammar.alphabet.terminal;
	dim2vector oldRules = grammar.productionRules;

	while (true) {
		std::string addingSymbols = "";
		dim2vector tempRules = oldRules;
		for (auto rule : oldRules) {
			// ���� ������ �������� ��� ������� ��� �����������
			if (std::regex_match(rule.back(), std::regex("^[" + dataSymbols + "]+$"))) {
				if (addingSymbols.find(rule.front()) == -1)
					addingSymbols += rule.front();
				// �������� �������
				tempRules.erase(std::find(tempRules.begin(), tempRules.end(), rule));
			}
		}
		if (oldRules == tempRules)
			break;
		dataSymbols += addingSymbols;
		oldRules = tempRules;
	}

	return dataSymbols.find(grammar.startSymbol) == -1;
}

/* �������������� ���������� */

// �������� ����������� ��������
Grammar eliminateUselessSymbols(const Grammar &grammar) {
	Grammar result;
	std::string dataSymbols = grammar.alphabet.terminal;
	dim2vector oldRules = grammar.productionRules;
	std::set<char> newTerminal, newNonterminal;
	std::set<std::vector<std::string>> newRules;

	while (true) {
		std::string addingSymbols = "";
		dim2vector tempRules = oldRules;
		for (auto rule : oldRules) {
			// ���� ������ �������� ��� ������� ��� �����������
			if (std::regex_match(rule.back(), std::regex("^[" + dataSymbols + "]+$"))) {
				if (addingSymbols.find(rule.front()) == -1)
					addingSymbols += rule.front();
				// ������ ����� ����������
				newNonterminal.insert(rule.front()[0]);
				for (auto symbol : rule.back()) {
					if (typeSymbol(symbol, grammar.alphabet))
						newTerminal.insert(symbol);
					else
						newNonterminal.insert(symbol);
				}
				newRules.insert({ rule.front(), rule.back() });
				// �������� �������
				tempRules.erase(std::find(tempRules.begin(), tempRules.end(), rule));
			}
		}
		if (oldRules == tempRules)
			break;
		dataSymbols += addingSymbols;
		oldRules = tempRules;
	}
	// ������������� ����� ����������
	result.alphabet.terminal.insert(result.alphabet.terminal.end(), newTerminal.begin(), newTerminal.end());
	result.alphabet.nonterminal.insert(result.alphabet.nonterminal.end(), newNonterminal.begin(), newNonterminal.end());
	result.alphabet.alphabet = result.alphabet.terminal + result.alphabet.nonterminal;
	result.productionRules.insert(result.productionRules.end(), newRules.begin(), newRules.end());
	result.startSymbol = grammar.startSymbol;

	return result;
}

// ���������� ������������ ��������
Grammar eliminateUnattainableSymbols(const Grammar &grammar) {
	Grammar result;
	std::string dataSymbols = grammar.startSymbol;
	dim2vector oldRules = grammar.productionRules;
	std::set<char> newTerminal, newNonterminal;
	std::set<std::vector<std::string>> newRules;

	while (true) {
		std::string addingSymbols = "";
		dim2vector tempRules = oldRules;
		for (auto rule : oldRules) {
			// ���� ����� ��������� ������ ��� ��� �������
			if (std::regex_search(rule.front(), std::regex("[" + dataSymbols + "]+"))) {
				if (addingSymbols.find(rule.back()) == -1)
					addingSymbols += rule.back();
				// ������ ����� ����������
				newNonterminal.insert(rule.front()[0]);
				for (auto symbol : rule.back()) {
					if (typeSymbol(symbol, grammar.alphabet))
						newTerminal.insert(symbol);
					else
						newNonterminal.insert(symbol);
				}
				newRules.insert({ rule.front(), rule.back() });
				// �������� �������
				tempRules.erase(std::find(tempRules.begin(), tempRules.end(), rule));
			}
		}
		if (oldRules == tempRules)
			break;
		dataSymbols += shielding(addingSymbols);
		oldRules = tempRules;
	}
	// ������������� ����� ����������
	result.alphabet.terminal.insert(result.alphabet.terminal.end(), newTerminal.begin(), newTerminal.end());
	result.alphabet.nonterminal.insert(result.alphabet.nonterminal.end(), newNonterminal.begin(), newNonterminal.end());
	result.alphabet.alphabet = result.alphabet.terminal + result.alphabet.nonterminal;
	result.productionRules.insert(result.productionRules.end(), newRules.begin(), newRules.end());
	result.startSymbol = grammar.startSymbol;

	return result;
}

// �������������� ������� ��������� ������ ������������
std::set<std::vector<std::string>> combiningRule(const std::string &leftRule, const std::string &rightRule, const std::string &dataSymbols) {
	std::set<std::vector<std::string>> result;
	std::string subRule = rightRule;
	const std::regex pattern("[" + dataSymbols + "]");
	std::sregex_iterator iter(subRule.begin(), subRule.end(), pattern), iterEnd;
	for (; iter != iterEnd; ++iter) {
		std::string tempRule = subRule;
		std::vector<std::string> elem({ leftRule, tempRule.erase((*iter).position(0), 1) });
		if (!elem.back().empty()) {
			result.insert(elem);
			std::set<std::vector<std::string>> combRule = combiningRule(leftRule, tempRule, dataSymbols);
			result.insert(combRule.begin(), combRule.end());
		}
	}
	return result;
}

// ���������� ������ ������
Grammar eliminateEmptyRules(const Grammar &grammar) {
	Grammar result;
	std::string dataSymbols = "e";
	std::set<std::vector<std::string>> newRules(grammar.productionRules.begin(), grammar.productionRules.end()), tempRules;

	while (true) {
		std::string addingSymbols = dataSymbols;
		std::set<std::vector<std::string>> oldRules = newRules;
		for (auto rule : newRules) {
			// ���� ������ ������ ������ ��� ��� �������
			if (std::regex_search(rule.back(), std::regex("[" + dataSymbols + "]+"))) {
				if (addingSymbols.find(rule.front()) == -1)
					addingSymbols += rule.front();
				// ������ ������ ������
				if (rule.back().find("e") == -1) {
					tempRules.insert(rule);
					// �������������� �������
					std::set<std::vector<std::string>> combRule = combiningRule(rule.front(), rule.back(), dataSymbols);
					tempRules.insert(combRule.begin(), combRule.end());
				}
				else
					// �������� ������ ������
					oldRules.erase(std::vector<std::string>({ rule.front(), rule.back() }));
			}
		}
		if (dataSymbols == addingSymbols)
			break;
		dataSymbols = addingSymbols;
		newRules = oldRules;
	}
	// ���������� ����� �������
	for (auto rule : tempRules) {
		newRules.insert(rule);
	}
	// ������������� ����� ����������
	result.alphabet = grammar.alphabet;
	// ����� ��������� ������
	if (dataSymbols.find(grammar.startSymbol) != -1) {
		// ����� ���������� �����������
		std::string startSymbol = unoccupiedNonterminal(grammar.alphabet.nonterminal);
		// ���������� ������ ��� ������ ���������� �������
		newRules.insert({ startSymbol, grammar.startSymbol });
		newRules.insert({ startSymbol, "e" });

		result.startSymbol = startSymbol;
		result.alphabet.nonterminal += result.startSymbol;
		result.alphabet.alphabet += result.startSymbol;
	}
	else
		result.startSymbol = grammar.startSymbol;
	// ������������� ������
	result.productionRules.insert(result.productionRules.end(), newRules.begin(), newRules.end());

	return eliminateUselessSymbols(result);
}

// ���������� ������ ������
Grammar eliminateChainRules(const Grammar &grammar) {
	Grammar result = grammar;
	// ����� ������� ������� ��� ������� �����������
	std::vector<std::string> chainRules;
	for (char nonterminal : grammar.alphabet.nonterminal) {
		std::string dataSymbols = std::string(1, nonterminal);
		dim2vector oldRules = grammar.productionRules;

		while (true) {
			std::string addingSymbols = "";
			dim2vector tempRules = oldRules;
			for (auto rule : oldRules) {
				// ���� ����� ������ ���������� ��� ��� ������ ������� � ������ ����������
				if (std::regex_match(rule.front(), std::regex("^[" + dataSymbols + "]$")) &&
					std::regex_match(rule.back(), std::regex("^[" + grammar.alphabet.nonterminal + "]$")))
				{
					addingSymbols += rule.back();
					// �������� �������
					tempRules.erase(std::find(tempRules.begin(), tempRules.end(), rule));
				}
			}
			if (oldRules == tempRules)
				break;
			dataSymbols += shielding(addingSymbols);
			oldRules = tempRules;
		}
		chainRules.push_back(dataSymbols);
	}

	// �������� ������ ������
	for (std::string chainRule : chainRules) {
		for (auto rightRule = chainRule.begin() + 1; rightRule != chainRule.end(); ++rightRule) {
			// ����� ������� �������
			auto iter = std::find(result.productionRules.begin(), result.productionRules.end(), 
				std::vector<std::string>({ std::string(1, chainRule.front()), std::string(1, *rightRule) }));
			if (iter != result.productionRules.end()) 
			{ 
				result.productionRules.erase(iter);
			}
		}
	}

	// ���������� ������
	for (std::string chainRule : chainRules) {
		dim2vector oldRules = result.productionRules;

		while (true) {
			dim2vector tempRules = oldRules;
			for (auto rule : oldRules) {
				// ����� ������ �������
				if (std::regex_match(rule.front(), std::regex("^[" + chainRule.substr(1, chainRule.size() - 1) + "]$"))) {
					// ������ �� ������ ������� ��� ������� �����������
					result.productionRules.emplace_back(std::vector<std::string>({ std::string(1, chainRule.front()), rule.back() }));
				}
			}
			if (oldRules == tempRules)
				break;
			oldRules = tempRules;
		}
	}

	return result;
}

// ���������� ����� ������������
Grammar eliminateleftFactorization(const Grammar &grammar) {
	Grammar result = grammar;

	while (true) {
		Grammar tmpGrammar = result;
		// ������ ������� �������
		for (auto sourceRule : result.productionRules) {
			std::set<std::vector<std::string>> facRules;
			unsigned count = 0;
			std::string prefix = sourceRule.back().substr(0, ++count);
			// ����� ����������� ��������
			while (true) {
				std::set<std::vector<std::string>> tmpFacRules;

				for (auto rule : result.productionRules) {
					// ����� ������ ���������� � ������ ����� ���������� � ��������
					if (std::regex_match(rule.front(), std::regex("^[" + sourceRule.front() + "]$"))
						&& std::regex_search(rule.back(), std::regex("^" + prefix)))
					{
						tmpFacRules.insert(rule);
					}
				}
				// ���� ���������� ��� ������� ��������, ����� ������ ����
				if (tmpFacRules.size() == 1) {
					prefix.erase(prefix.size() - 1);
					break;
				}
				facRules = tmpFacRules;
				// ������� ����� ������ ���� ������� ������� (�� ����� �������������)
				if (prefix == sourceRule.back()) {
					++count;
					break;
				}
				prefix = sourceRule.back().substr(0, ++count);
			}
			// �� ���� ���������� ��� ������� �������
			if (facRules.empty())
				break;
			// ������ ����� ����������
			std::string newNonterm = unoccupiedNonterminal(result.alphabet.nonterminal);
			result.alphabet.nonterminal += newNonterm;
			// �������� ������� � ����� ������������
			result.productionRules.emplace_back(std::vector<std::string>({ sourceRule.front(), prefix + newNonterm }));
			for (auto rule : facRules) {
				// �������� ������
				result.productionRules.erase(std::find(result.productionRules.begin(), result.productionRules.end(), rule));
				// �������� ����� ������
				std::string rightRule = rule.back().substr(count - 1);
				if (rightRule.empty())
					rightRule = "e";
				result.productionRules.emplace_back(std::vector<std::string>({ newNonterm, rightRule }));
			}
			// ���������� ��������� ����������
			break;
		}
		// ���� ���������� �� ���������� - ��������� �, ����� �������� ���������� ����� ������������
		if (result.alphabet.nonterminal == tmpGrammar.alphabet.nonterminal)
			break;
	}

	return result;
}

// ���������� ������ ����� ��������
Grammar eliminateleftDirectLeftRecursion(const Grammar &grammar) {
	Grammar result = grammar;
	// ����� ������ ����� �������� ��� ������� �����������
	for (char nonterminal : result.alphabet.nonterminal) {
		dim2vector leftRecRules, nontermRules;

		while (true) {
			dim2vector tempRules = result.productionRules;
			for (auto rule : result.productionRules) {
				// ����� ������ ����������
				if (std::regex_match(rule.front(), std::regex("^[" + std::string(1, nonterminal) + "]$"))) {
					// ������ ����� ���������� � ������� �����������
					if (std::regex_search(rule.back(), std::regex("^[" + std::string(1, nonterminal) + "]"))) {
						leftRecRules.push_back(rule);
						// �������� �������
						tempRules.erase(std::find(tempRules.begin(), tempRules.end(), std::vector<std::string>({ rule.front(), rule.back() })));
					}
					else {
						// ���� ������� ��� �������� ���
						if (std::find(nontermRules.begin(), nontermRules.end(), rule) == nontermRules.end())
							nontermRules.push_back(rule);
					}
				}
			}
			if (result.productionRules == tempRules)
				break;
			result.productionRules = tempRules;
		}
		// ��� ������� ��������� ���� ������� ������ ����� ��������
		if (!leftRecRules.empty()) {
			std::string newNonterm;
			// ��� ������� ��������� ���� ������� ����� ������� � ������ ����� ��������
			if (!nontermRules.empty()) {
				// �������� ������ �����������
				newNonterm = unoccupiedNonterminal(result.alphabet.nonterminal);
				result.alphabet.nonterminal += newNonterm;
				// �������� ����� ������ � ����� ������������ � ������ �����
				for (auto rule : nontermRules) {
					result.productionRules.emplace_back(std::vector<std::string>({ rule.front(), rule.back() + newNonterm }));
				}
			}
			// ������ � ������ ������������ ���, ����� � ������ ����� ��������
			else {
				newNonterm = std::string(1, nonterminal);
			}
			// �������� ����� ������ ��� ������ �����������
			for (auto rule : leftRecRules) {
				result.productionRules.emplace_back(std::vector<std::string>({ newNonterm, rule.back().substr(1, rule.back().size() - 1) + newNonterm }));
				result.productionRules.emplace_back(std::vector<std::string>({ newNonterm, rule.back().substr(1, rule.back().size() - 1) }));
			}
		}
	}

	return result;
}

int main() {
	setlocale(NULL, "RUS");

	Grammar grammar;

	std::cout << "�������������� ��-���������� G = (T, N, P, S)" << std::endl
		<< "������ 'e' �������������� ��� ������� �������!" << std::endl << "������� ������������ ������� T: ";
	getline(std::cin, grammar.alphabet.terminal);
	grammar.alphabet.terminal = std::regex_replace(grammar.alphabet.terminal, std::regex("[ ,]"), "");
	grammar.alphabet.terminal = shielding(grammar.alphabet.terminal);

	std::cout << "������� �������������� ������� N: ";
	getline(std::cin, grammar.alphabet.nonterminal);
	grammar.alphabet.nonterminal = std::regex_replace(grammar.alphabet.nonterminal, std::regex("[ ,]"), "");

	grammar.alphabet.alphabet = grammar.alphabet.terminal + grammar.alphabet.nonterminal;

	int count;
	std::cout << "������� ���������� ������: ";
	std::cin >> count;
	std::cout << "���� ������� ������������ '->', ���� ��� '|'" << std::endl;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	const std::regex pattern("^[" + grammar.alphabet.alphabet + "]+ *-> *" + "(?:(?:[" + grammar.alphabet.alphabet + "]+)|(?:e$))(?:(?: *\\| *)(?:(?:[" + grammar.alphabet.alphabet + "]+)|(?:e$)))*$");
	for (int i = 0; i < count; ++i) {
		std::cout << "������� ������� ����� " << i + 1 << ": ";
		std::string inputString;
		getline(std::cin, inputString);
		// �������� ������������ �����
		if (std::regex_match(inputString.begin(), inputString.end(), pattern)) {
			// ���������� �� �������
			const std::regex split(" *-> *| *\\| *");
			std::regex_token_iterator<std::string::iterator> iter(inputString.begin(), inputString.end(), split, -1), iterEnd;
			auto leftRule = *iter;
			for (++iter; iter != iterEnd; ++iter) {
				grammar.productionRules.emplace_back(std::vector<std::string>({ leftRule, *iter }));
			}
		}
		else {
			std::cout << "������� ������� �� �����!" << std::endl;
			system("pause");
			return 0;
		}
	}
	// ��������� ���� ������ ����� � �������� ��� ��������� ������!
	grammar.alphabet.terminal = "e" + grammar.alphabet.terminal;

	std::cout << "������� ��������� ������: ";
	getline(std::cin, grammar.startSymbol);

	// ��������� ������� ������� ������� ������� � ��������� ��������
	if (grammar.productionRules[0][0] == grammar.startSymbol) {
		// ��������� �� ��
		if (contextFree(grammar.productionRules, grammar.alphabet)) {
			// ��������� �� ������������
			if (false) {//(regular(grammar.productionRules, grammar.alphabet)) {
				std::cout << "���������� �� �������� ����������-���������!" << std::endl;
				system("pause");
				return 0;
			}
		}
		else {
			std::cout << "���������� �� �������� ����������-���������!" << std::endl;
			system("pause");
			return 0;
		}
	}
	else {
		std::cout << "������ ������ �� ������������� ����������!" << std::endl;
		system("pause");
		return 0;
	}
	
	/* ������ � ����������� */

	std::cout << "\n�������� ������������� �����:" << std::endl;
	if (emptyLanguage(grammar)) {
		std::cout << "\t���� ����." << std::endl;
		system("pause");
		return 0;
	}
	else
		std::cout << "\t���� ����������" << std::endl;

	int select;
	while (true) {
		std::cout << "\n�������� ��������:\n" << "\t1 - ���������� ����������� ��������\n" << "\t2 - ���������� ������������ ��������\n"
			<< "\t3 - �������� ������ ������\n" << "\t4 - ���������� ������ ������\n" << "\t5 - ���������� ������ � ����� ������������\n"
			<< "\t6 - ���������� ������ � ������ ����� ��������\n" << "\t0 - ���������� ���������\n";
		std::cin >> select;
		switch (select) {
		case 1:
			grammar = eliminateUselessSymbols(grammar);
			std::cout << "\n���������� ����� ���������� ����������� ��������:" << std::endl;
			showGrammar(grammar);
			break;
		case 2:
			grammar = eliminateUnattainableSymbols(grammar);
			std::cout << "\n���������� ����� ���������� ������������ ��������:" << std::endl;
			showGrammar(grammar);
			break;
		case 3:
			grammar = eliminateEmptyRules(grammar);
			std::cout << "\n���������� ����� �������� ������ ������:" << std::endl;
			showGrammar(grammar);
			break;
		case 4:
			grammar = eliminateChainRules(grammar);
			std::cout << "\n���������� ����� ���������� ������ ������:" << std::endl;
			showGrammar(grammar);
			break;
		case 5:
			grammar = eliminateleftFactorization(grammar);
			std::cout << "\n���������� ����� ���������� ������ � ����� ������������:" << std::endl;
			showGrammar(grammar);
			break;
		case 6:
			grammar = eliminateleftDirectLeftRecursion(grammar);
			std::cout << "\n���������� ����� ���������� ������ � ������ ����� ��������:" << std::endl;
			showGrammar(grammar);
			break;
		case 0:
			return 0;
		default:
			std::cout << "������� �������� ����!" << std::endl;
			break;
		}
	}

	return 0;
}