#pragma once

#include <cstdint>
#include <deque>

namespace measurement {
struct FrequencyMeasurement {
	std::deque<uint64_t> timestamps;
	uint64_t measurementPeriod;

	FrequencyMeasurement(uint64_t measurementPeriod)
	: measurementPeriod { measurementPeriod } {}

	void clean();
	void record(uint64_t timestamp);
	float calcFrequency();
	float calcPeriod();
};
};
