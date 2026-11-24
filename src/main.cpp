#include <algorithm>
#include <ctime>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include "appstate.h"

static const std::string LIFE_STANDARD_RULE { "B3/S23" };

static AppState __state;

void RuleStringModal::renderModal(ui::UiBuilder& uiBuilder, const std::string& buffer) {
	uiBuilder.beginModal();
	uiBuilder.style.alignment = {
		HorizontalAlignment::center,
		VerticalAlignment::top
	};
	uiBuilder.resetCursor();

	uiBuilder.text("Enter Rulestring");
	uiBuilder.textBox(buffer, 400);
	if (!showError) {
		uiBuilder.text(" ");
	} else {
		uiBuilder.styleStack.push({
			.type = ui::UiStylePropertyType::TextColor,
			.colorValue = { 255, 0, 0, 255 }
		});
		uiBuilder.text("Invalid rulestring.");
		uiBuilder.styleStack.pop();
	}

	uiBuilder.endModal();
}

void RuleStringModal::show(AppState& state) {
	if (!active) return;

	auto& input = state.textInputMode;

	if (input.cancelled) {
		close(state);
		return;
	}

	if (input.textChanged) showError = false;

	renderModal(state.uiBuilder, state.textInputMode.inputBuffer);

	if (input.enterPressed) {
		bool success = state.world.rule.setString(input.inputBuffer);
		if (success) {
			close(state);
		} else {
			showError = true;
		}
	}
}

void RuleStringModal::open(AppState& state) {
	state.setMode(TextInputMode::id);
	showError = false;
	active = true;
}

void RuleStringModal::close(AppState& state) {
	state.setMode(LifeControlMode::id);
	active = false;
}

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
		state.uiState.debugMode = static_cast<UiDebugMode>(
			(state.uiState.debugMode + 1) % UI_DEBUG_MODE_COUNT
		);
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
	lifeState.tickTimeScore += deltaTime;

	uint64_t timeSpentTicking = 0;
	while (lifeState.tickTimeScore > lifeState.tickTargetTime) {
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
			lifeState.tickTimeScore = 0;
			break;
		}

		uint64_t cost = std::max(tickRealTime, lifeState.tickTargetTime);
		if (cost < lifeState.tickTimeScore) {
			lifeState.tickTimeScore -= cost;
		} else {
			lifeState.tickTimeScore = 0;
		}
	}
}

void renderUi(AppState& state) {
	auto& uiBuilder = state.uiBuilder;
	auto& uiState = state.uiState;

	uiBuilder.beginFrame(state.renderer);

	uiBuilder.styleStack.push({
		.type = ui::UiStylePropertyType::TextBackground,
		.colorValue = { 0, 0, 0, 130 }
	});

	uiBuilder.style.alignment = {
		HorizontalAlignment::left,
		VerticalAlignment::top
	};
	uiBuilder.resetCursor();

	// FPS counter
	uiBuilder.textFormat(
		"{:.2f} FPS ({:.2f} ms)",
		1000.0f / state.averageFrameTime,
		state.averageFrameTime
	);

	// TPS counter
	uiBuilder.textFormat(
		"{:.2f} TPS ({:.2f}/{} ms)",
		1000.0f * state.lifeState.tickMeasurement.calcFrequency(),
		state.lifeState.tickMeasurement.calcPeriod(),
		state.lifeState.tickTargetTime
	);

	uiBuilder.style.alignment = {
		HorizontalAlignment::right,
		VerticalAlignment::bottom
	};
	uiBuilder.resetCursor();

	// Life grid position
	float mouseX;
	float mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	auto pt = state.worldWindow.lifeSPointFromScreenPoint(mouseX, mouseY);
	uiBuilder.textFormat("({},{})", pt.x, pt.y);

	// Zoom
	uiBuilder.textFormat("Z{:.2f}", state.worldWindow.cellSize);

	uiBuilder.style.alignment = {
		HorizontalAlignment::center,
		VerticalAlignment::bottom
	};
	uiBuilder.resetCursor();

	if (state.lifeState.paused) {
		uiBuilder.text("PAUSED");
	}

	uiBuilder.style.alignment = {
		HorizontalAlignment::left,
		VerticalAlignment::bottom
	};
	uiBuilder.resetCursor();

	uiBuilder.textFormat("R: {}", state.world.rule.toString());

	uiBuilder.styleStack.pop();

	state.uiState.ruleStringModal.show(state);

	ui::renderDrawData(state.renderer, uiBuilder.drawData);

	if (state.uiState.debugMode > UI_DEBUG_NONE) {
		bool showElements = state.uiState.debugMode == UI_DEBUG_ALL;
		ui::debugDrawData(
			state.renderer,
			uiBuilder.drawData,
			showElements
		);
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
	renderUi(state);
	SDL_RenderPresent(renderer);

	state.currentMode->onEndFrame(state);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {}
