#include "life.h"

#include <fstream>
#include <iostream>
#include <vector>

void appendDigitList(uint16_t ruleInt, std::string& buffer) {
	for (char i = 0; i <= 8; i++) {
		if (ruleInt & (1 << i)) buffer.push_back('0' + i);
	}
}

std::string stripFirstChar(const std::string& s) {
	return s.substr(1, s.size()-1);
}

namespace life {
void CellFile::addCell(const Point& pt) {
	if (pt.x >= size.x) size.x = pt.x+1;
	if (pt.y >= size.y) size.y = pt.y+1;

	points.push_back(pt);
}

void CellFile::dump() const {
	std::cout << "CellFile: " << name << std::endl;

	for (const auto& p : points) {
		std::cout << '(' << p.x << ',' << p.y << ')' << std::endl;
	}
}

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
	char p = '\0';

	b = next();
	if (b != 'b' && b != 'B' || error) goto err;

	parseDigits(born);

	// only acceptable error case is if we encountered '/'
	if (previous() != '/') goto err;
	error = false;

	s = next();
	if (s != 's' && s != 'S' || error) goto err;

	parseDigits(survive);

	p = previous();
	if (!(!hasNext() && (isDigit(p) || p == 's' || p == 'S'))) goto err;
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

std::string LifeRule::toString() {
	std::string out;

	out.push_back('B');
	appendDigitList(born, out);
	out.push_back('/');
	out.push_back('S');
	appendDigitList(survive, out);

	return out;
}

bool isPlaintextComment(const std::string& s) {
	if (s.size() == 0) return false;
	if (s[0] != '!') return false;

	return true;
}

ResultType<CellFile> loadPlaintextFile(const std::string& filename) {
	ResultType<CellFile> result;
	result.error = true;

	std::ifstream file { filename };
	std::string line {};

	std::getline(file, line);
	if (!isPlaintextComment(line)) return result;

	result.value.name = stripFirstChar(line);

	while(std::getline(file, line)) {
		if (!isPlaintextComment(line)) break;
	}

	if (!file) return result;

	Point p { 0, 0 };
	do {
		if (line.size() == 0) continue;
		if (line.back() == '\r') line.pop_back();

		while (p.x < line.size()) {
			if (line[p.x] == 'O') {
				result.value.addCell(p);
				p.x++;
			} else if (line[p.x] == '.') {
				p.x++;
			} else {
				return result;
			}
		}

		p.x = 0;
		p.y++;
	} while(std::getline(file, line));

	std::cout << "Loaded .cells: " << result.value.name << std::endl;

	result.error = false;
	return result;
}

ResultType<CellFile> loadFile(const std::string& filename) {
	// Currently only one supported filetype, more planned.
	if (filename.ends_with(".cells")) {
		return loadPlaintextFile(filename);
	}

	return ResultType<CellFile>::badResult();
}
}
