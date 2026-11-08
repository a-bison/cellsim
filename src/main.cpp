#include <ctime>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "display.h"
#include "life.h"

static constexpr size_t WINDOW_WIDTH = 800;
static constexpr size_t WINDOW_HEIGHT = WINDOW_WIDTH;
static constexpr size_t WORLD_WIDTH = 100;
static constexpr size_t WORLD_HEIGHT = WORLD_WIDTH;
static constexpr float CELLSIZE = WINDOW_WIDTH / WORLD_WIDTH;

const life::Point GLIDER[] {
	{ 1, 0 },
	{ 2, 1 },
	{ 0, 2 },
	{ 1, 2 },
	{ 2, 2 }
};

struct SimState {
	unsigned int tickms = 100;
	uint64_t lastTickTime = 0;
	bool paused;
};

struct AppState {
	SDL_Window* window;
	SDL_Renderer* renderer;

	life::World<WORLD_WIDTH, WORLD_HEIGHT> world;
	SimState simState;
};

static AppState __state;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
	*appstate = &__state;

	std::srand(std::time(0));
	
	bool windowCreateSuccess = SDL_CreateWindowAndRenderer(
		"cellsim",
		WINDOW_WIDTH, WINDOW_HEIGHT,
		0, // No flags
		&__state.window,
		&__state.renderer
	);
	if (!windowCreateSuccess) {
		SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_SetRenderVSync(__state.renderer, 1);

	__state.world.backBuffer().randomize();
	__state.world.swap();

	return SDL_APP_CONTINUE;
}

SDL_AppResult handleKeyDown(AppState& state, SDL_KeyboardEvent& event) {
	switch (event.key) {
	case SDLK_ESCAPE:
		return SDL_APP_SUCCESS;
	case SDLK_SPACE:
		state.simState.paused = !state.simState.paused;
		break;
	case SDLK_E:
		if (state.simState.paused) {
			state.world.tick();
			state.world.swap();
		}
		break;
	case SDLK_R:
		state.world.backBuffer().randomize();
		state.world.swap();
		break;
	default:
		break;
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event *event) {
	auto& state = *reinterpret_cast<AppState*>(appstate);

	switch (event->type) {
	case SDL_EVENT_QUIT:
		return SDL_APP_SUCCESS;
	case SDL_EVENT_KEY_DOWN:
		return handleKeyDown(state, event->key);
	default:
		return SDL_APP_CONTINUE;
	}
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	auto& state = *reinterpret_cast<AppState*>(appstate);
	auto* renderer = state.renderer;

	uint64_t currentTime = SDL_GetTicks();
	uint64_t deltaTime = currentTime - state.simState.lastTickTime;

	if (deltaTime > state.simState.tickms && !state.simState.paused) {
		state.world.tick();
		state.world.swap();
		state.simState.lastTickTime = currentTime;
	}

	int w = 0;
	int h = 0;
	SDL_GetCurrentRenderOutputSize(renderer, &w, &h);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_FRect cellRect;
	display::initCellRect(cellRect, CELLSIZE);
	for (int y = 0; y < WORLD_HEIGHT; y++) {
		for (int x = 0; x < WORLD_WIDTH; x++) {
			if (!state.world.frontBuffer().get(x, y)) continue;

			display::fillCellRect(cellRect, 0.0f, CELLSIZE, x, y);
			SDL_RenderFillRect(renderer, &cellRect);
		}
	}

	SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
	display::renderGrid(renderer, 0.0f, CELLSIZE);

	SDL_RenderPresent(renderer);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {}
