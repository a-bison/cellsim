#include "life.h"

#include <iostream>
#include <vector>

namespace life {
bool LifeRuleParser::hasNext() {
	return cursor < str->size();
}

char LifeRuleParser::next() {
	if (!hasNext()) {
		error = true;
		return '\0';
	}

	char c = (*str)[cursor];
	cursor++;
	return c;
}

bool LifeRuleParser::isDigit(char c) {
	return c >= '0' && c <= '9';
}

unsigned int LifeRuleParser::nextDigit() {
	char c = next();
	if (!isDigit(c)) {
		error = true;
		return 0;
	}

	return static_cast<unsigned int>(c - '0');
}

void LifeRuleParser::parseDigits(std::vector<unsigned int>& v) {
	while (true) {
		unsigned int d = nextDigit();
		if (error) break;
		v.push_back(d);
	}
}

char LifeRuleParser::previous() {
	if (cursor == 0) {
		error = true;
		return '\0';
	}

	return (*str)[cursor - 1];
}

void LifeRuleParser::parse() {
	char b = '\0';
	char s = '\0';

	b = next();
	if (b != 'b' && b != 'B' || error) goto err;

	parseDigits(born);

	// only acceptable error case is if we encountered '/'
	if (previous() != '/') goto err;
	error = false;

	s = next();
	if (s != 's' && s != 'S' || error) goto err;

	parseDigits(survive);

	// only acceptable case is 
	// !hasNext() && isDigit(previous())
	if (hasNext() || !isDigit(previous())) goto err;
	error = false;

	return;
err:
	error = true;
	return;
}

void LifeRule::setBorn(std::span<const unsigned int> _born) {
	born = 0;
	for (const auto& i : _born) born |= 1 << i;
}

void LifeRule::setSurvive(std::span<const unsigned int> _survive) {
	survive = 0;
	for (const auto& i : _survive) survive |= 1 << i;
}

bool LifeRule::setString(const std::string& ruleString) {
	LifeRuleParser parser { ruleString };
	parser.parse();

	if (parser.error) return false;

	setBorn(parser.born);
	setSurvive(parser.survive);

	return true;
}

void LifeRule::clear() {
	born = 0;
	survive = 0;
}

bool LifeRule::eval(bool alive, unsigned int liveNeighbors) {
	if (!alive) {
		return bool(born & (1 << liveNeighbors));
	} else {
		return bool(survive & (1 << liveNeighbors));
	}
}
}
