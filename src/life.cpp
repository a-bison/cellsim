#include "life.h"

#include <iostream>
#include <vector>

namespace life {
void LifeRule::setBorn(std::span<const unsigned int> _born) {
	born = 0;
	for (const auto& i : _born) born |= 1 << i;
}

void LifeRule::setSurvive(std::span<const unsigned int> _survive) {
	survive = 0;
	for (const auto& i : _survive) survive |= 1 << i;
}

bool LifeRule::setString(const std::string& ruleString) {
	unsigned int cursor = 0;

	if (ruleString.size() == 0) return false;

	if (ruleString[cursor] != 'b' && ruleString[cursor] != 'B') {
		return false;
	}

	cursor++;
	if (cursor >= ruleString.size()) return false;

	std::vector<unsigned int> born;
	while (ruleString[cursor] != '/') {
		char digit = ruleString[cursor];
		if (digit < '0' || digit > '9') return false;

		born.push_back(static_cast<unsigned int>(digit - '0'));

		cursor++;
		if (cursor >= ruleString.size()) return false;
	}

	cursor++;
	if (cursor >= ruleString.size()) return false;

	if (ruleString[cursor] != 's' && ruleString[cursor] != 'S') {
		return false;
	}

	cursor++;
	if (cursor >= ruleString.size()) return false;

	std::vector<unsigned int> survive;
	while (cursor < ruleString.size()) {
		char digit = ruleString[cursor];
		if (digit < '0' || digit > '9') return false;

		survive.push_back(static_cast<unsigned int>(digit - '0'));

		cursor++;
	}

	setBorn(born);
	setSurvive(survive);
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
