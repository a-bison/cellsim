#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <span>
#include <type_traits>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

using signed_size_t = std::make_signed_t<size_t>;

static constexpr size_t WINDOW_WIDTH = 800;
static constexpr size_t WINDOW_HEIGHT = WINDOW_WIDTH;
static constexpr size_t WORLD_WIDTH = 100;
static constexpr size_t WORLD_HEIGHT = WORLD_WIDTH;
static constexpr float CELLSIZE = WINDOW_WIDTH / WORLD_WIDTH;

struct LifePoint {
	size_t x;
	size_t y;
};

const LifePoint GLIDER[] {
	{ 1, 0 },
	{ 2, 1 },
	{ 0, 2 },
	{ 1, 2 },
	{ 2, 2 }
};

template<size_t W, size_t H>
struct LifeBuffer {
	std::array<bool, W*H> __world;

	LifeBuffer() {
		std::fill(__world.begin(), __world.end(), false);
	}

	static size_t __ix(signed_size_t x, signed_size_t y) {
		return W*y+x;
	}

	static bool inBounds(signed_size_t x, signed_size_t y) {
		return x >=0 && x < W && y >=0 && y < H;
	}

	bool get(signed_size_t x, signed_size_t y) const {
		if (!inBounds(x, y)) return false;
		return __world[__ix(x, y)];
	}

	bool get(LifePoint pt) const { return get(pt.x, pt.y); }

	void set(signed_size_t x, signed_size_t y, bool val) {
		if (!inBounds(x, y)) return;
		__world[__ix(x, y)] = val;
	}

	void set(LifePoint pt, bool val) { set(pt.x, pt.y, val); }

	unsigned int countLiveNeighbors(signed_size_t x, signed_size_t y) const {
		unsigned int count = 0;

		for (signed_size_t yOfs = -1; yOfs <= 1; yOfs++) {
			for (signed_size_t xOfs = -1; xOfs <= 1; xOfs++) {
				if (yOfs == 0 && xOfs == 0) continue;

				if (get(x+xOfs, y+yOfs)) count++;
			}
		}

		return count;
	}

	unsigned int countLiveNeighbors(LifePoint pt) const {
		return countLiveNeighbors(pt.x, pt.y);
	}

	void massSet(signed_size_t x, signed_size_t y, std::span<const LifePoint> offsets, bool val) {
		for (auto& offset : offsets) {
			set(x+offset.x, y+offset.y, val);
		}
	}
	
	void randomize() {
		for (auto& cell : __world) {
			cell = static_cast<bool>(std::rand() % 2);
		}
	}

	void dumpGrid() const {
		for (size_t y = 0; y < H; y++) {
			for (size_t x = 0; x < W; x++) {
				std::cout << get(x, y) ? '1' : '0';
			}

			std::cout << std::endl;
		}
	}
};

// Given previous cell state and number of live neighbors, returns whether the
// cell should be alive or dead next tick
bool lifeRule(bool alive, unsigned int liveNeighbors) {
	if (alive && (liveNeighbors == 2 || liveNeighbors == 3)) return true;
	if (!alive && liveNeighbors == 3) return true;

	return false;
}

template<size_t W, size_t H>
struct LifeWorld {
	std::array<LifeBuffer<W, H>, 2> __buffers;
	size_t __bufferIndex = 0;

	void swap() {
		__bufferIndex = (__bufferIndex == 1) ? 0 : 1;
	}

	// The buffer that is ready to display. Should only ever be read from.
	const LifeBuffer<W, H>& frontBuffer() const {
		return __buffers[__bufferIndex];
	}

	// The buffer that is being written to for a new tick.
	LifeBuffer<W, H>& backBuffer() {
		return __buffers[1-__bufferIndex];
	}

	void tick() {
		auto& front = frontBuffer();
		auto& back = backBuffer();

		for (size_t y = 0; y < H; y++) {
			for (size_t x = 0; x < W; x++) {
				bool alive = front.get(x, y);
				auto count = front.countLiveNeighbors(x, y);
				bool nextState = lifeRule(alive, count);
				back.set(x, y, nextState);
			}
		}
	}
};

struct SimState {
	unsigned int tickms = 100;
	uint64_t lastTickTime = 0;
	bool paused;
};

struct AppState {
	SDL_Window* window;
	SDL_Renderer* renderer;

	LifeWorld<WORLD_WIDTH, WORLD_HEIGHT> world;
	SimState simState;
};

static AppState __state;

void renderVerticalLine(SDL_Renderer* renderer, float x, float h) {
	SDL_RenderLine(renderer, x, 0.0f, x, h);
}

void renderHorizonalLine(SDL_Renderer* renderer, float y, float w) {
	SDL_RenderLine(renderer, 0.0f, y, w, y);
}

void renderGrid(SDL_Renderer* renderer, float offset, float cellSize) {
	int w_i = 0;
	int h_i = 0;

	SDL_GetCurrentRenderOutputSize(renderer, &w_i, &h_i);

	float w = static_cast<float>(w_i);
	float h = static_cast<float>(h_i);

	offset = std::fmod(offset, cellSize);

	float x = offset;
	while (x < w) {
		renderVerticalLine(renderer, x, h);
		x += cellSize;
	}

	float y = offset;
	while (y < h) {
		renderHorizonalLine(renderer, y, w);
		y += cellSize;
	}
}

void initCellRect(SDL_FRect& rect, float cellSize) {
	rect.x = 0.0f;
	rect.y = 0.0f;
	rect.w = cellSize;
	rect.h = cellSize;
}

void fillCellRect(SDL_FRect& rect, float offset, float cellSize, int x, int y) {
	rect.x = offset+cellSize*x;
	rect.y = offset+cellSize*y;
}

bool shouldRenderCellRect(SDL_FRect& rect, int w, int h) {
	if (rect.x > w || rect.y > h) return false;
	if (rect.x + rect.w < 0 || rect.y + rect.h < 0) return false;

	return true;
}

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
	initCellRect(cellRect, CELLSIZE);
	for (int y = 0; y < WORLD_HEIGHT; y++) {
		for (int x = 0; x < WORLD_WIDTH; x++) {
			if (!state.world.frontBuffer().get(x, y)) continue;

			fillCellRect(cellRect, 0.0f, CELLSIZE, x, y);
			SDL_RenderFillRect(renderer, &cellRect);
		}
	}

	SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
	renderGrid(renderer, 0.0f, CELLSIZE);

	SDL_RenderPresent(renderer);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {}
