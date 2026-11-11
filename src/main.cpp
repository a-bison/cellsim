#include <ctime>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "ui.h"
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
	float averageTickTime = 0.0f;
	bool paused = false;
};

struct AppState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	ui::Ui uiState;

	uint64_t lastTime = 0;
	float averageFrameTime = 0.0f;

	life::World<WORLD_WIDTH, WORLD_HEIGHT> world;
	ui::WorldWindow worldWindow;
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

	__state.uiState.renderer = __state.renderer;

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

float rollingAverage(float currentAverage, float newValue) {
	return 0.95*currentAverage + 0.05*newValue;
}

template<size_t W, size_t H>
void processLifeTick(life::World<W, H>& world, LifeState& lifeState, uint64_t currentTime) {
	uint64_t deltaTime = currentTime - lifeState.lastTickTime;

	if (deltaTime > lifeState.tickms && !lifeState.paused) {
		world.tick();
		world.swap();
		lifeState.lastTickTime = currentTime;
		lifeState.averageTickTime = rollingAverage(lifeState.averageTickTime, deltaTime);
	}
}

void renderUi(AppState& state) {
	auto uiState = state.uiState;

	uiState.updateRenderArea();

	uiState.setAlign(ui::VerticalAlignment::top, ui::HorizontalAlignment::left);
	uiState.resetCursor();

	// FPS counter
	uiState.textFormat(
		"{:.2f} FPS ({:.2f} ms)",
		1000.0f / state.averageFrameTime,
		state.averageFrameTime
	);

	// TPS counter
	uiState.textFormat(
		"{:.2f} TPS ({:.2f}/{} ms)",
		1000.0f / state.lifeState.averageTickTime,
		state.lifeState.averageTickTime,
		state.lifeState.tickms
	);

	uiState.setAlign(ui::VerticalAlignment::bottom, ui::HorizontalAlignment::right);
	uiState.resetCursor();

	// Life grid position
	float mouseX;
	float mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	auto pt = state.worldWindow.lifePointFromScreenPoint(mouseX, mouseY);
	auto pt_int = ui::sPointFromFPoint(pt);
	uiState.textFormat("({},{})", pt_int.x, pt_int.y);

	// Zoom
	uiState.textFormat("Z{:.2f}", state.worldWindow.cellSize);
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	auto& state = *reinterpret_cast<AppState*>(appstate);
	auto* renderer = state.renderer;

	uint64_t currentTime = SDL_GetTicks();
	uint64_t frameDelta = currentTime - state.lastTime;
	state.lastTime = currentTime;
	state.averageFrameTime = rollingAverage(state.averageFrameTime, frameDelta);

	processLifeTick(state.world, state.lifeState, currentTime);

	float mouseX;
	float mouseY;
	auto mouseButtons = SDL_GetRelativeMouseState(&mouseX, &mouseY);
	if (mouseButtons & SDL_BUTTON_MMASK) {
		state.worldWindow.xoffset += mouseX;
		state.worldWindow.yoffset += mouseY;
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	state.worldWindow.renderWorld(renderer, state.world);

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	renderUi(state);

	SDL_RenderPresent(renderer);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {}
