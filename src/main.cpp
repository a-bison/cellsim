#include <algorithm>
#include <ctime>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include "appstate.h"

static const std::string LIFE_STANDARD_RULE { "B3/S23" };

static AppState __state;

void AppState::setMode(unsigned int id) {
	if (currentMode->getId() == id) return;

	currentMode->onSwitchOut(*this);

	switch (id) {
	case LifeControlMode::id:
		currentMode = &lifeControlMode;
		break;
	case TextInputMode::id:
		currentMode = &textInputMode;
		break;
	default:
		break;
	}

	currentMode->onSwitchIn(*this);
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
	*appstate = &__state;

	std::srand(std::time(0));

	bool windowCreateSuccess = SDL_CreateWindowAndRenderer(
		"cellsim",
		WINDOW_WIDTH, WINDOW_HEIGHT,
		SDL_WINDOW_RESIZABLE,
		&__state.window,
		&__state.renderer
	);
	if (!windowCreateSuccess) {
		SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	__state.worldWindow.setCellSize(CELLSIZE);

	SDL_SetRenderVSync(__state.renderer, 1);

	__state.world.rule.setString(LIFE_STANDARD_RULE);

	__state.world.backBuffer().randomize();
	__state.world.swap();

	__state.currentMode = &__state.lifeControlMode;

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event *event) {
	auto& state = *reinterpret_cast<AppState*>(appstate);

	if (event->type == SDL_EVENT_QUIT) {
		return SDL_APP_SUCCESS;
	}

	if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_F1) {
		state.uiState.cycleDebugMode();
		return SDL_APP_CONTINUE;
	}

	return state.currentMode->onEvent(state, event);
}

float rollingAverage(float currentAverage, float newValue) {
	return 0.95*currentAverage + 0.05*newValue;
}

template<size_t W, size_t H>
void processLifeTick(life::World<W, H>& world, LifeState& lifeState, uint64_t deltaTime) {
	if (lifeState.paused) return;

	// We accumulate the time delta between each frame, and then "spend" it
	// until there's not enough for a tick. This allows the simulation to
	// get very close the the target tick period, and allows sub-frame time
	// simulation speeds by running multiple ticks per frame.
	lifeState.tickTimeBudget += deltaTime;

	uint64_t timeSpentTicking = 0;
	while (lifeState.tickTimeBudget > lifeState.tickTargetTime) {
		uint64_t before = SDL_GetTicks();
		world.tick();
		uint64_t after = SDL_GetTicks();
		lifeState.tickMeasurement.record(after);
		uint64_t tickRealTime = after - before;

		world.swap();

		timeSpentTicking += tickRealTime;
		if (timeSpentTicking > 30) {
			// If catching up on ticks is taking too long, bail and
			// draw a frame
			lifeState.tickTimeBudget = 0;
			break;
		}

		uint64_t cost = std::max(tickRealTime, lifeState.tickTargetTime);
		if (cost < lifeState.tickTimeBudget) {
			lifeState.tickTimeBudget -= cost;
		} else {
			lifeState.tickTimeBudget = 0;
		}
	}
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	auto& state = *reinterpret_cast<AppState*>(appstate);
	auto* renderer = state.renderer;

	// Time stuff
	uint64_t currentTime = SDL_GetTicks();
	uint64_t frameDelta = currentTime - state.lastTime;
	state.lastTime = currentTime;
	state.averageFrameTime = rollingAverage(state.averageFrameTime, frameDelta);

	// Do state updates
	state.currentMode->onIterate(state);
	processLifeTick(state.world, state.lifeState, frameDelta);

	// Render
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	state.worldWindow.renderWorld(renderer, state.world);
	state.uiState.render(state);
	SDL_RenderPresent(renderer);

	state.currentMode->onEndFrame(state);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {}
