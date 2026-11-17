#include <ctime>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include "appstate.h"

static const std::string LIFE_STANDARD_RULE { "B3/S23" };

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
	// Currently this modal is the only UI element that uses text
	// input
	if (state.currentMode->getId() != TextInputMode::id) return;

	auto& input = state.textInputMode;

	if (input.textChanged) showError = false;

	renderModal(state.uiBuilder, state.textInputMode.inputBuffer);

	if (input.enterPressed) {
		bool success = state.world.rule.setString(input.inputBuffer);
		if (!success) {
			showError = true;
		} else {
			state.setMode(LifeControlMode::id);
		}
	}
}

void RuleStringModal::open(AppState& state) {
	state.setMode(TextInputMode::id);
	showError = false;
}

SDL_AppResult LifeControlMode::handleKeyDown(AppState& state, SDL_KeyboardEvent& event) {
	switch (event.key) {
	case SDLK_ESCAPE:
		return SDL_APP_SUCCESS;
	case SDLK_SPACE:
		state.lifeState.paused = !state.lifeState.paused;
		state.lifeState.lastTickTime = SDL_GetTicks();
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
	case SDLK_C:
		state.world.backBuffer().clear();
		state.world.swap();
		break;
	case SDLK_S:
		state.uiState.ruleStringModal.open(state);
		break;
	case SDLK_UP:
		state.lifeState.tickms -= 10;
		if (state.lifeState.tickms < 10) state.lifeState.tickms = 10;
		break;
	case SDLK_DOWN:
		state.lifeState.tickms += 10;
		break;
	case SDLK_F1:
		state.uiDebug = static_cast<UiDebugMode>(
			(state.uiDebug + 1) % UI_DEBUG_MODE_COUNT
		);
		break;
	default:
		break;
	}

	return SDL_APP_CONTINUE;
}

void LifeControlMode::handleMouseWheel(AppState& state, SDL_MouseWheelEvent& event) {
	state.worldWindow.zoom(event.mouse_x, event.mouse_y, event.y);
}

SDL_AppResult LifeControlMode::onEvent(AppState& state, SDL_Event *event) {
	switch (event->type) {
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

void LifeControlMode::processMouse(AppState& state) {
	float relMouseX;
	float relMouseY;
	auto mouseButtons = SDL_GetRelativeMouseState(&relMouseX, &relMouseY);

	if (mouseButtons & SDL_BUTTON_MMASK) {
		state.worldWindow.xoffset += relMouseX;
		state.worldWindow.yoffset += relMouseY;
	}

	float mouseX;
	float mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);

	if (mouseButtons & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK)) {
		auto [cellX, cellY] = state.worldWindow.lifeSPointFromScreenPoint(mouseX, mouseY);
		bool value = bool(mouseButtons & SDL_BUTTON_LMASK);
		state.world.frontBufferWritable().set(cellX, cellY, value);
	}
}

void LifeControlMode::onIterate(AppState& state) {
	processMouse(state);
}

void TextInputMode::pasteText() {
	char* pasted = SDL_GetClipboardText();
	char* cursor = pasted;

	while (*cursor) {
		if (inputBuffer.size() < charLimit) {
			inputBuffer.push_back(*cursor);
			textChanged = true;
		} else {
			break;
		}
		cursor++;
	}

	SDL_free(pasted);
}

void TextInputMode::handleKeyDown(AppState& state, SDL_KeyboardEvent& event) {
	switch (event.key) {
	case SDLK_ESCAPE:
		cancelled = true;
		state.setMode(LifeControlMode::id);
		break;
	case SDLK_BACKSPACE:
		if (inputBuffer.size() > 0) {
			inputBuffer.pop_back();
			textChanged = true;
		}
		break;
	case SDLK_RETURN:
		enterPressed = true;
		break;
	case SDLK_V:
		pasteText();
		break;
	default:
		break;
	}
}

void TextInputMode::handleTextInput(AppState& state, SDL_TextInputEvent& event) {
	const char* ptr = event.text;
	while (*ptr) {
		if (inputBuffer.size() < charLimit) {
			inputBuffer.push_back(*ptr);
			textChanged = true;
		}
		ptr++;
	}
}

SDL_AppResult TextInputMode::onEvent(AppState& state, SDL_Event *event) {
	switch (event->type) {
	case SDL_EVENT_KEY_DOWN:
		handleKeyDown(state, event->key);
		break;
	case SDL_EVENT_TEXT_INPUT:
		handleTextInput(state, event->text);
		break;
	default:
		break;
	}

	return SDL_APP_CONTINUE;
}

void TextInputMode::resetFlags() {
	cancelled = false;
	enterPressed = false;
	textChanged = false;
}

void TextInputMode::onEndFrame(AppState& state) {
	resetFlags();
}

void TextInputMode::onSwitchIn(AppState& state) {
	SDL_StartTextInput(state.window);
	inputBuffer.clear();
	resetFlags();
}

void TextInputMode::onSwitchOut(AppState& state) {
	SDL_StopTextInput(state.window);
}

void AppState::setMode(unsigned int id) {
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

	return state.currentMode->onEvent(state, event);
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
		1000.0f / state.lifeState.averageTickTime,
		state.lifeState.averageTickTime,
		state.lifeState.tickms
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

	uiBuilder.styleStack.pop();

	state.uiState.ruleStringModal.show(state);

	ui::renderDrawData(state.renderer, uiBuilder.drawData);

	if (state.uiDebug > UI_DEBUG_NONE) {
		bool showElements = state.uiDebug == UI_DEBUG_ALL;
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
	processLifeTick(state.world, state.lifeState, currentTime);

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
