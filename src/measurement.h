#pragma once

#include <cstdint>
#include <deque>

namespace measurement {
struct TimeMeasurement {
	uint64_t start;
	uint64_t end;

	uint64_t delta() const;
};

struct TimeTracker {
	std::deque<TimeMeasurement> measurements;
	uint64_t measurementPeriod;
	bool measuring = false;
	TimeMeasurement currentMeasurement;

	TimeTracker(uint64_t measurementPeriod)
	: measurementPeriod { measurementPeriod } {}

	// Clears measurements outside the measurement period, based on the
	// current time. Called automatically by most other functions.
	void clean();

	// Start/end a measurement.
	void start();
	void end();

	// Calculates the average time a recorded event takes.
	float calcAverageTime();

	// Calculates the average frequency of end timestamps within the
	// measurement period.
	float calcFrequency();

	// Calculates the average time between the end timestamp of measured
	// events within the measurement period.
	float calcPeriod();

	const TimeMeasurement& lastMeasurement() const;
};
};
