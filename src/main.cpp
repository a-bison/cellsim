#include <algorithm>
#include <ctime>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include "appstate.h"

static const std::string LIFE_STANDARD_RULE { "B3/S23" };

static AppState __state;

void printSdlInfo(SDL_Renderer* renderer) {
	auto* rendererName = SDL_GetRendererName(renderer);
	if (!renderer) {
		SDL_Log("Could not get renderer name: %s", SDL_GetError());
	} else {
		SDL_Log("Renderer name: %s", rendererName);
	}
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
	*appstate = &__state;

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

	printSdlInfo(__state.renderer);

	SDL_SetRenderVSync(__state.renderer, 1);

	// Init non-SDL state
	std::srand(std::time(0));
	__state.worldWindow.setCellSize(CELLSIZE);
	__state.lifeState.initializeWorld(LIFE_STANDARD_RULE);
	__state.currentMode = &__state.lifeControlMode;

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event *event) {
	auto& state = *reinterpret_cast<AppState*>(appstate);

	if (event->type == SDL_EVENT_QUIT) {
		return SDL_APP_SUCCESS;
	}

	if (event->type == SDL_EVENT_KEY_DOWN) {
		const auto& key = event->key.key;
		switch(key) {
		case SDLK_F1:
			state.uiState.debugMode.cycle();
			return SDL_APP_CONTINUE;
		case SDLK_F2:
			state.uiState.perfMode.cycle();
			return SDL_APP_CONTINUE;
		default:
			break;
		}
	}

	return state.currentMode->onEvent(state, event);
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	auto& state = *reinterpret_cast<AppState*>(appstate);
	auto* renderer = state.renderer;

	// Time stuff
	state.frameTracker.start();
	uint64_t frameDelta = (
		state.frameTracker.currentMeasurement.start -
		state.frameTracker.lastMeasurement().start
	);

	// Do state updates
	state.currentMode->onIterate(state);
	state.lifeState.update(frameDelta);

	// Render
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	state.worldWindow.renderWorld(renderer, state.lifeState.world);
	state.uiState.render(state);

	// End frame timer before presenting, since SDL_RenderPresent blocks
	// when waiting for vsync, but we want to accurately measure the real
	// time a frame takes.
	state.frameTracker.end();

	SDL_RenderPresent(renderer);

	state.currentMode->onEndFrame(state);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {}
