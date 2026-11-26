template<size_t W, size_t H>
void LifeState<W, H>::initializeWorld(const std::string& ruleString) {
	world.rule.setString(ruleString);
	world.backBuffer().randomize();
	world.swap();
}

template<size_t W, size_t H>
void LifeState<W, H>::update(uint64_t deltaTime) {
	if (paused) return;

	// We accumulate the time delta between each frame, and then "spend" it
	// until there's not enough for a tick. This allows the simulation to
	// get very close the the target tick period, and allows sub-frame time
	// simulation speeds by running multiple ticks per frame.
	tickTimeBudget += deltaTime;

	uint64_t timeSpentTicking = 0;
	while (tickTimeBudget > tickTargetTime) {
		tickTracker.start();
		world.tick();
		tickTracker.end();
		uint64_t tickRealTime = tickTracker.lastMeasurement().delta();

		world.swap();

		timeSpentTicking += tickRealTime;
		if (timeSpentTicking > 30) {
			// If catching up on ticks is taking too long, bail and
			// draw a frame
			tickTimeBudget = 0;
			break;
		}

		uint64_t cost = std::max(tickRealTime, tickTargetTime);
		if (cost < tickTimeBudget) {
			tickTimeBudget -= cost;
		} else {
			tickTimeBudget = 0;
		}
	}
}
