#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <set>
#include <algorithm>
#include "Grammar.h"


// экранирование спецсимволов
std::string shielding(const std::string &string) {
	std::string result = string;
	const std::regex character("([\\-\\^\\$\\\\\\.\\*\\+\\?\\(\\)\\[\\]\\{\\}\\|])");
	std::sregex_iterator iter(string.begin(), string.end(), character), iterEnd;
	for (int count = 0; iter != iterEnd; ++iter, ++count) {
		result.insert((*iter).position(0) + count, "\\");
	}
	return result;
}

// контекстно-свободная грамматика
bool contextFree(const dim2vector &rules, const Alphabet &alphabet) {
	for (auto rule : rules) {
		if (!std::regex_match(rule.front(), std::regex("^[" + alphabet.nonterminal + "]$")))
			return false;
	}
	return true;
}

// регулярная грамматика
bool regular(const dim2vector &rules, const Alphabet &alphabet) {
	int rightRegular = 1;
	for (auto rule : rules) {
		if (!std::regex_match(rule.front(), std::regex("^[" + alphabet.nonterminal + "]$")))
			return false;
			// не леволинейная и была праволинейной
			if (!std::regex_match(rule.back(), std::regex("^[" + alphabet.nonterminal + "]?[" + alphabet.terminal + "]+$")) || rightRegular == 2) {
				// не праволинейная и была леволинейной
				if (!std::regex_match(rule.back(), std::regex("^[" + alphabet.terminal + "]+[" + alphabet.nonterminal + "]?$")) || rightRegular == 0) {
					return false;
				}
				// праволинейная
				else
					rightRegular = 2;
			}
			// леволинейная
			else
				rightRegular = 0;
	}
	return true;
}

// вывод грамматики
void showGrammar(const Grammar &grammar) {
	Grammar tempGrammar = grammar;
	std::cout << "\tТерминалы: " << tempGrammar.alphabet.terminal << std::endl;
	std::cout << "\tНетерминалы: " << tempGrammar.alphabet.nonterminal << std::endl;
	std::sort(tempGrammar.productionRules.begin(), tempGrammar.productionRules.end());
	std::cout << "\tПравила:" << std::endl;
	std::for_each(tempGrammar.productionRules.begin(), tempGrammar.productionRules.end(), [](const std::vector<std::string> &line) {
		std::cout << "\t\t" << line.front() << " -> " << line.back();
		std::cout << std::endl;
	});
	std::cout << "\tНачальный символ: " << tempGrammar.startSymbol << std::endl;
}

// определение типа символа(0 - nonterminal, 1 - terminal)
bool typeSymbol(const char &symbol, const Alphabet &alphabet) {
	return (alphabet.nonterminal.find(symbol) == -1);
}

// поиск незанятого нетерминала
std::string unoccupiedNonterminal(const std::string &nonterminal) {
	char startSymbol;
	for (int i = 0;; ++i) {
		startSymbol = char(65 + i);
		if (nonterminal.find(startSymbol) == -1)
			break;
	}
	return std::string(1, startSymbol);
}


// проверка пустоты языка
bool emptyLanguage(const Grammar &grammar) {
	std::string dataSymbols = grammar.alphabet.terminal;
	dim2vector oldRules = grammar.productionRules;

	while (true) {
		std::string addingSymbols = "";
		dim2vector tempRules = oldRules;
		for (auto rule : oldRules) {
			// если справа терминал или цепочка его порождающая
			if (std::regex_match(rule.back(), std::regex("^[" + dataSymbols + "]+$"))) {
				if (addingSymbols.find(rule.front()) == -1)
					addingSymbols += rule.front();
				// удаление правила
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

/* ПРЕОБРАЗОВАНИЕ ГРАММАТИКИ */

// устрание бесполезных символов
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
			// если справа терминал или цепочка его порождающая
			if (std::regex_match(rule.back(), std::regex("^[" + dataSymbols + "]+$"))) {
				if (addingSymbols.find(rule.front()) == -1)
					addingSymbols += rule.front();
				// запись новой грамматики
				newNonterminal.insert(rule.front()[0]);
				for (auto symbol : rule.back()) {
					if (typeSymbol(symbol, grammar.alphabet))
						newTerminal.insert(symbol);
					else
						newNonterminal.insert(symbol);
				}
				newRules.insert({ rule.front(), rule.back() });
				// удаление правила
				tempRules.erase(std::find(tempRules.begin(), tempRules.end(), rule));
			}
		}
		if (oldRules == tempRules)
			break;
		dataSymbols += addingSymbols;
		oldRules = tempRules;
	}
	// переписывание новой грамматики
	result.alphabet.terminal.insert(result.alphabet.terminal.end(), newTerminal.begin(), newTerminal.end());
	result.alphabet.nonterminal.insert(result.alphabet.nonterminal.end(), newNonterminal.begin(), newNonterminal.end());
	result.alphabet.alphabet = result.alphabet.terminal + result.alphabet.nonterminal;
	result.productionRules.insert(result.productionRules.end(), newRules.begin(), newRules.end());
	result.startSymbol = grammar.startSymbol;

	return result;
}

// устранение недостижимых символов
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
			// если слева начальный символ или его цепочка
			if (std::regex_search(rule.front(), std::regex("[" + dataSymbols + "]+"))) {
				if (addingSymbols.find(rule.back()) == -1)
					addingSymbols += rule.back();
				// запись новой грамматики
				newNonterminal.insert(rule.front()[0]);
				for (auto symbol : rule.back()) {
					if (typeSymbol(symbol, grammar.alphabet))
						newTerminal.insert(symbol);
					else
						newNonterminal.insert(symbol);
				}
				newRules.insert({ rule.front(), rule.back() });
				// удаление правила
				tempRules.erase(std::find(tempRules.begin(), tempRules.end(), rule));
			}
		}
		if (oldRules == tempRules)
			break;
		dataSymbols += shielding(addingSymbols);
		oldRules = tempRules;
	}
	// переписывание новой грамматики
	result.alphabet.terminal.insert(result.alphabet.terminal.end(), newTerminal.begin(), newTerminal.end());
	result.alphabet.nonterminal.insert(result.alphabet.nonterminal.end(), newNonterminal.begin(), newNonterminal.end());
	result.alphabet.alphabet = result.alphabet.terminal + result.alphabet.nonterminal;
	result.productionRules.insert(result.productionRules.end(), newRules.begin(), newRules.end());
	result.startSymbol = grammar.startSymbol;

	return result;
}

// комбинирование правила удалением пустых нетерминалов
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

// устранение пустых правил
Grammar eliminateEmptyRules(const Grammar &grammar) {
	Grammar result;
	std::string dataSymbols = "e";
	std::set<std::vector<std::string>> newRules(grammar.productionRules.begin(), grammar.productionRules.end()), tempRules;

	while (true) {
		std::string addingSymbols = dataSymbols;
		std::set<std::vector<std::string>> oldRules = newRules;
		for (auto rule : newRules) {
			// если справа пустой символ или его цепочка
			if (std::regex_search(rule.back(), std::regex("[" + dataSymbols + "]+"))) {
				if (addingSymbols.find(rule.front()) == -1)
					addingSymbols += rule.front();
				// запись пустых правил
				if (rule.back().find("e") == -1) {
					tempRules.insert(rule);
					// комбинированые правила
					std::set<std::vector<std::string>> combRule = combiningRule(rule.front(), rule.back(), dataSymbols);
					tempRules.insert(combRule.begin(), combRule.end());
				}
				else
					// удаление пустых правил
					oldRules.erase(std::vector<std::string>({ rule.front(), rule.back() }));
			}
		}
		if (dataSymbols == addingSymbols)
			break;
		dataSymbols = addingSymbols;
		newRules = oldRules;
	}
	// дописываем новые правила
	for (auto rule : tempRules) {
		newRules.insert(rule);
	}
	// переписывание новой грамматики
	result.alphabet = grammar.alphabet;
	// новый начальный символ
	if (dataSymbols.find(grammar.startSymbol) != -1) {
		// поиск незанятого нетерминала
		std::string startSymbol = unoccupiedNonterminal(grammar.alphabet.nonterminal);
		// добавление правил для нового начального символа
		newRules.insert({ startSymbol, grammar.startSymbol });
		newRules.insert({ startSymbol, "e" });

		result.startSymbol = startSymbol;
		result.alphabet.nonterminal += result.startSymbol;
		result.alphabet.alphabet += result.startSymbol;
	}
	else
		result.startSymbol = grammar.startSymbol;
	// переписывание правил
	result.productionRules.insert(result.productionRules.end(), newRules.begin(), newRules.end());

	return eliminateUselessSymbols(result);
}

// устранение цепных правил
Grammar eliminateChainRules(const Grammar &grammar) {
	Grammar result = grammar;
	// поиск цепного правила для каждого нетерминала
	std::vector<std::string> chainRules;
	for (char nonterminal : grammar.alphabet.nonterminal) {
		std::string dataSymbols = std::string(1, nonterminal);
		dim2vector oldRules = grammar.productionRules;

		while (true) {
			std::string addingSymbols = "";
			dim2vector tempRules = oldRules;
			for (auto rule : oldRules) {
				// если слева данный нетерминал или его цепное правило И справа нетерминал
				if (std::regex_match(rule.front(), std::regex("^[" + dataSymbols + "]$")) &&
					std::regex_match(rule.back(), std::regex("^[" + grammar.alphabet.nonterminal + "]$")))
				{
					addingSymbols += rule.back();
					// удаление правила
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

	// удаление цепных правил
	for (std::string chainRule : chainRules) {
		for (auto rightRule = chainRule.begin() + 1; rightRule != chainRule.end(); ++rightRule) {
			// поиск цепного правила
			auto iter = std::find(result.productionRules.begin(), result.productionRules.end(), 
				std::vector<std::string>({ std::string(1, chainRule.front()), std::string(1, *rightRule) }));
			if (iter != result.productionRules.end()) 
			{ 
				result.productionRules.erase(iter);
			}
		}
	}

	// добавление правил
	for (std::string chainRule : chainRules) {
		dim2vector oldRules = result.productionRules;

		while (true) {
			dim2vector tempRules = oldRules;
			for (auto rule : oldRules) {
				// слева цепное правило
				if (std::regex_match(rule.front(), std::regex("^[" + chainRule.substr(1, chainRule.size() - 1) + "]$"))) {
					// создаём не цепное правило для данного нетерминала
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

// устранение левой факторизации
Grammar eliminateleftFactorization(const Grammar &grammar) {
	Grammar result = grammar;

	while (true) {
		Grammar tmpGrammar = result;
		// проход каждого правила
		for (auto sourceRule : result.productionRules) {
			std::set<std::vector<std::string>> facRules;
			unsigned count = 0;
			std::string prefix = sourceRule.back().substr(0, ++count);
			// поиск наибольшего префикса
			while (true) {
				std::set<std::vector<std::string>> tmpFacRules;

				for (auto rule : result.productionRules) {
					// слева данный нетерминал И правая часть начинается с префикса
					if (std::regex_match(rule.front(), std::regex("^[" + sourceRule.front() + "]$"))
						&& std::regex_search(rule.back(), std::regex("^" + prefix)))
					{
						tmpFacRules.insert(rule);
					}
				}
				// нету совпадений для данного префикса, кроме самого себя
				if (tmpFacRules.size() == 1) {
					prefix.erase(prefix.size() - 1);
					break;
				}
				facRules = tmpFacRules;
				// префикс равен правой часи данного правила (не будет увеличиваться)
				if (prefix == sourceRule.back()) {
					++count;
					break;
				}
				prefix = sourceRule.back().substr(0, ++count);
			}
			// не было совпадений для данного правила
			if (facRules.empty())
				break;
			// создаём новый нетерминал
			std::string newNonterm = unoccupiedNonterminal(result.alphabet.nonterminal);
			result.alphabet.nonterminal += newNonterm;
			// создание правила с новым нетерминалом
			result.productionRules.emplace_back(std::vector<std::string>({ sourceRule.front(), prefix + newNonterm }));
			for (auto rule : facRules) {
				// удаление правил
				result.productionRules.erase(std::find(result.productionRules.begin(), result.productionRules.end(), rule));
				// создание новых правил
				std::string rightRule = rule.back().substr(count - 1);
				if (rightRule.empty())
					rightRule = "e";
				result.productionRules.emplace_back(std::vector<std::string>({ newNonterm, rightRule }));
			}
			// возвращаем изменённую грамматику
			break;
		}
		// если грамматика не изменилась - возращаем её, иначе повторим устранение левой факторизации
		if (result.alphabet.nonterminal == tmpGrammar.alphabet.nonterminal)
			break;
	}

	return result;
}

// устранение прямой левой рекурсии
Grammar eliminateleftDirectLeftRecursion(const Grammar &grammar) {
	Grammar result = grammar;
	// поиск прямой левой рекурсии для каждого нетерминала
	for (char nonterminal : result.alphabet.nonterminal) {
		dim2vector leftRecRules, nontermRules;

		while (true) {
			dim2vector tempRules = result.productionRules;
			for (auto rule : result.productionRules) {
				// слева данный нетерминал
				if (std::regex_match(rule.front(), std::regex("^[" + std::string(1, nonterminal) + "]$"))) {
					// правая часть начинается с данного нетерминала
					if (std::regex_search(rule.back(), std::regex("^[" + std::string(1, nonterminal) + "]"))) {
						leftRecRules.push_back(rule);
						// удаление правила
						tempRules.erase(std::find(tempRules.begin(), tempRules.end(), std::vector<std::string>({ rule.front(), rule.back() })));
					}
					else {
						// если правила нет доавляем его
						if (std::find(nontermRules.begin(), nontermRules.end(), rule) == nontermRules.end())
							nontermRules.push_back(rule);
					}
				}
			}
			if (result.productionRules == tempRules)
				break;
			result.productionRules = tempRules;
		}
		// для данного нетермина есть правила прямой левой рекурсии
		if (!leftRecRules.empty()) {
			std::string newNonterm;
			// для данного нетермина есть правила кроме правила с прямой левой рекурсии
			if (!nontermRules.empty()) {
				// создание нового нетерминала
				newNonterm = unoccupiedNonterminal(result.alphabet.nonterminal);
				result.alphabet.nonterminal += newNonterm;
				// создание новых правил с новым нетерминалом в правой части
				for (auto rule : nontermRules) {
					result.productionRules.emplace_back(std::vector<std::string>({ rule.front(), rule.back() + newNonterm }));
				}
			}
			// правил с данным нетерминалом нет, кроме с прямой левой рекурсии
			else {
				newNonterm = std::string(1, nonterminal);
			}
			// создание новых правил для нового нетерминала
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

	std::cout << "Преобразование КС-грамматики G = (T, N, P, S)" << std::endl
		<< "Символ 'e' зарезервирован для пустого символа!" << std::endl << "Введите терминальные символы T: ";
	getline(std::cin, grammar.alphabet.terminal);
	grammar.alphabet.terminal = std::regex_replace(grammar.alphabet.terminal, std::regex("[ ,]"), "");
	grammar.alphabet.terminal = shielding(grammar.alphabet.terminal);

	std::cout << "Введите нетерминальные символы N: ";
	getline(std::cin, grammar.alphabet.nonterminal);
	grammar.alphabet.nonterminal = std::regex_replace(grammar.alphabet.nonterminal, std::regex("[ ,]"), "");

	grammar.alphabet.alphabet = grammar.alphabet.terminal + grammar.alphabet.nonterminal;

	int count;
	std::cout << "Введите количество правил: ";
	std::cin >> count;
	std::cout << "Знак следует обозначается '->', знак или '|'" << std::endl;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	const std::regex pattern("^[" + grammar.alphabet.alphabet + "]+ *-> *" + "(?:(?:[" + grammar.alphabet.alphabet + "]+)|(?:e$))(?:(?: *\\| *)(?:(?:[" + grammar.alphabet.alphabet + "]+)|(?:e$)))*$");
	for (int i = 0; i < count; ++i) {
		std::cout << "Введите правило номер " << i + 1 << ": ";
		std::string inputString;
		getline(std::cin, inputString);
		// проверка корректности ввода
		if (std::regex_match(inputString.begin(), inputString.end(), pattern)) {
			// разделение по шаблону
			const std::regex split(" *-> *| *\\| *");
			std::regex_token_iterator<std::string::iterator> iter(inputString.begin(), inputString.end(), split, -1), iterEnd;
			auto leftRule = *iter;
			for (++iter; iter != iterEnd; ++iter) {
				grammar.productionRules.emplace_back(std::vector<std::string>({ leftRule, *iter }));
			}
		}
		else {
			std::cout << "Правило введено не верно!" << std::endl;
			system("pause");
			return 0;
		}
	}
	// добавляем знак пустой сроки в терминал для упрощения работы!
	grammar.alphabet.terminal = "e" + grammar.alphabet.terminal;

	std::cout << "Введите начальный символ: ";
	getline(std::cin, grammar.startSymbol);

	// сравнение первого символа первого правила с начальным символом
	if (grammar.productionRules[0][0] == grammar.startSymbol) {
		// проверяем на КС
		if (contextFree(grammar.productionRules, grammar.alphabet)) {
			// проверяем на регулярность
			if (false) {//(regular(grammar.productionRules, grammar.alphabet)) {
				std::cout << "Грамматика не является контекстно-свободной!" << std::endl;
				system("pause");
				return 0;
			}
		}
		else {
			std::cout << "Грамматика не является контекстно-свободной!" << std::endl;
			system("pause");
			return 0;
		}
	}
	else {
		std::cout << "Первый символ не соответствует начальному!" << std::endl;
		system("pause");
		return 0;
	}
	
	/* РАБОТА С ГРАММАТИКОЙ */

	std::cout << "\nПроверка существования языка:" << std::endl;
	if (emptyLanguage(grammar)) {
		std::cout << "\tЯзык пуст." << std::endl;
		system("pause");
		return 0;
	}
	else
		std::cout << "\tЯзык существует" << std::endl;

	int select;
	while (true) {
		std::cout << "\nВыберите действие:\n" << "\t1 - устранение бесполезных символов\n" << "\t2 - устранение недостижимых символов\n"
			<< "\t3 - удаление пустых правил\n" << "\t4 - устранение цепных правил\n" << "\t5 - устранение правил с левой факторизации\n"
			<< "\t6 - устранения правил с прямой левой рекурсии\n" << "\t0 - завершение программы\n";
		std::cin >> select;
		switch (select) {
		case 1:
			grammar = eliminateUselessSymbols(grammar);
			std::cout << "\nГрамматика после устранения бесполезных символов:" << std::endl;
			showGrammar(grammar);
			break;
		case 2:
			grammar = eliminateUnattainableSymbols(grammar);
			std::cout << "\nГрамматика после устранения недостижимых символов:" << std::endl;
			showGrammar(grammar);
			break;
		case 3:
			grammar = eliminateEmptyRules(grammar);
			std::cout << "\nГрамматика после удаления пустых правил:" << std::endl;
			showGrammar(grammar);
			break;
		case 4:
			grammar = eliminateChainRules(grammar);
			std::cout << "\nГрамматика после устранения цепных правил:" << std::endl;
			showGrammar(grammar);
			break;
		case 5:
			grammar = eliminateleftFactorization(grammar);
			std::cout << "\nГрамматика после устранения правил с левой факторизации:" << std::endl;
			showGrammar(grammar);
			break;
		case 6:
			grammar = eliminateleftDirectLeftRecursion(grammar);
			std::cout << "\nГрамматика после устранения правил с прямой левой рекурсии:" << std::endl;
			showGrammar(grammar);
			break;
		case 0:
			return 0;
		default:
			std::cout << "Данного действия нету!" << std::endl;
			break;
		}
	}

	return 0;
}