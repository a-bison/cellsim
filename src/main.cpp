#include <ctime>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "display.h"
#include "life.h"

static constexpr size_t WINDOW_WIDTH = 800;
static constexpr size_t WINDOW_HEIGHT = WINDOW_WIDTH;
static constexpr size_t WORLD_WIDTH = 400;
static constexpr size_t WORLD_HEIGHT = WORLD_WIDTH;
static constexpr float CELLSIZE = WINDOW_WIDTH / WORLD_WIDTH;

const life::Point GLIDER[] {
	{ 1, 0 },
	{ 2, 1 },
	{ 0, 2 },
	{ 1, 2 },
	{ 2, 2 }
};

struct LifeState {
	unsigned int tickms = 100;
	uint64_t lastTickTime = 0;
	bool paused;
};

struct AppState {
	SDL_Window* window;
	SDL_Renderer* renderer;

	life::World<WORLD_WIDTH, WORLD_HEIGHT> world;
	display::WorldWindow worldWindow;
	LifeState lifeState;
};

static AppState __state;

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

	__state.world.backBuffer().randomize();
	__state.world.swap();

	return SDL_APP_CONTINUE;
}

SDL_AppResult handleKeyDown(AppState& state, SDL_KeyboardEvent& event) {
	switch (event.key) {
	case SDLK_ESCAPE:
		return SDL_APP_SUCCESS;
	case SDLK_SPACE:
		state.lifeState.paused = !state.lifeState.paused;
		break;
	case SDLK_E:
		if (state.lifeState.paused) {
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

void handleMouseWheel(AppState& state, SDL_MouseWheelEvent& event) {
	state.worldWindow.zoom(event.mouse_x, event.mouse_y, event.y);
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event *event) {
	auto& state = *reinterpret_cast<AppState*>(appstate);

	switch (event->type) {
	case SDL_EVENT_QUIT:
		return SDL_APP_SUCCESS;
	case SDL_EVENT_KEY_DOWN:
		return handleKeyDown(state, event->key);
	case SDL_EVENT_MOUSE_WHEEL:
		handleMouseWheel(state, event->wheel);
		break;
	default:
		break;
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	auto& state = *reinterpret_cast<AppState*>(appstate);
	auto* renderer = state.renderer;

	uint64_t currentTime = SDL_GetTicks();
	uint64_t deltaTime = currentTime - state.lifeState.lastTickTime;

	if (deltaTime > state.lifeState.tickms && !state.lifeState.paused) {
		state.world.tick();
		state.world.swap();
		state.lifeState.lastTickTime = currentTime;
	}

	float mouseX;
	float mouseY;
	auto mouseButtons = SDL_GetRelativeMouseState(&mouseX, &mouseY);
	if (mouseButtons & SDL_BUTTON_MMASK) {
		state.worldWindow.xoffset += mouseX;
		state.worldWindow.yoffset += mouseY;
	}

	int w = 0;
	int h = 0;
	SDL_GetCurrentRenderOutputSize(renderer, &w, &h);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	state.worldWindow.renderWorld(renderer, state.world);

	SDL_RenderPresent(renderer);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {}
