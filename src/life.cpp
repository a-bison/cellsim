#include "life.h"

namespace life {
void LifeRule::setBorn(std::span<const unsigned int> _born) {
	born = 0;
	for (const auto& i : _born) born |= 1 << i;
}

void LifeRule::setSurvive(std::span<const unsigned int> _survive) {
	survive = 0;
	for (const auto& i : _survive) survive |= 1 << i;
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
