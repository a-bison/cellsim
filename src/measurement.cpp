#include "measurement.h"

#include <SDL3/SDL.h>
#include <limits>

namespace measurement {
uint64_t TimeMeasurement::delta() const {
	return end - start;
}

void TimeTracker::clean() {
	if (measurements.size() == 0) return;

	uint64_t timestamp = SDL_GetTicks();
	uint64_t deltaTime = std::numeric_limits<uint64_t>::max();
	while ((deltaTime = timestamp - measurements.back().end) > measurementPeriod) {
		measurements.pop_back();
		if (measurements.size() == 0) break;
	}
}

void TimeTracker::start() {
	measuring = true;
	currentMeasurement.start = SDL_GetTicks();
	currentMeasurement.end = 0;
}

void TimeTracker::end() {
	currentMeasurement.end = SDL_GetTicks();
	measurements.push_front(currentMeasurement);
	measuring = false;
	clean();
}

float TimeTracker::calcAverageTime() {
	clean();
	uint64_t sum = 0;

	for (const auto& measurement : measurements) {
		sum += measurement.delta();
	}

	return static_cast<float>(sum) / measurements.size();
}

float TimeTracker::calcFrequency() {
	clean();
	return measurements.size() / static_cast<float>(measurementPeriod);
}

float TimeTracker::calcPeriod() {
	if (measurements.size() == 0) return 0;
	clean();
	return static_cast<float>(measurementPeriod) / measurements.size();
}

const TimeMeasurement& TimeTracker::lastMeasurement() const {
	return measurements.front();
}
};
