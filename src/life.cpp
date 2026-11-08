#include "life.h"

namespace life {
bool lifeRule(bool alive, unsigned int liveNeighbors) {
	if (alive && (liveNeighbors == 2 || liveNeighbors == 3)) return true;
	if (!alive && liveNeighbors == 3) return true;

	return false;
}
}
