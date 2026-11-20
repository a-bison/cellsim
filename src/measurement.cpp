#include "measurement.h"

#include <SDL3/SDL.h>
#include <limits>

namespace measurement {
void FrequencyMeasurement::clean() {
	if (timestamps.size() == 0) return;

	uint64_t timestamp = SDL_GetTicks();
	uint64_t deltaTime = std::numeric_limits<uint64_t>::max();
	while ((deltaTime = timestamp - timestamps.back()) > measurementPeriod) {
		timestamps.pop_back();
		if (timestamps.size() == 0) break;
	}
}

void FrequencyMeasurement::record(uint64_t timestamp) {
	timestamps.push_front(timestamp);
	clean();
}

float FrequencyMeasurement::calcFrequency() {
	clean();
	return timestamps.size() / static_cast<float>(measurementPeriod);
}

float FrequencyMeasurement::calcPeriod() {
	if (timestamps.size() == 0) return 0;
	clean();
	return static_cast<float>(measurementPeriod) / timestamps.size();
}
};
