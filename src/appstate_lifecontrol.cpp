#include "appstate.h"

SDL_AppResult LifeControlMode::handleKeyDown(AppState& state, SDL_KeyboardEvent& event) {
	auto& lifeState = state.lifeState;
	auto& world = lifeState.world;

	switch (event.key) {
	case SDLK_ESCAPE:
		return SDL_APP_SUCCESS;
	case SDLK_SPACE:
		lifeState.paused = !lifeState.paused;
		break;
	case SDLK_E:
		if (lifeState.paused) {
			world.tick();
			world.swap();
		}
		break;
	case SDLK_R:
		world.backBuffer().randomize();
		world.swap();
		break;
	case SDLK_C:
		world.backBuffer().clear();
		world.swap();
		break;
	case SDLK_S:
		state.uiState.ruleStringModal.open(state);
		break;
	case SDLK_Z:
		if (lifeState.paused) world.swap();
		break;
	case SDLK_L:
		state.uiState.openLifeFileDialog(state);
		break;
	case SDLK_UP:
		lifeState.tickTargetTime -= 10;
		if (lifeState.tickTargetTime < 10) {
			lifeState.tickTargetTime = 10;
		}
		break;
	case SDLK_DOWN:
		lifeState.tickTargetTime += 10;
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

	auto keyMods = SDL_GetModState();
	bool shouldPan = (
		static_cast<bool>(mouseButtons & SDL_BUTTON_MMASK) ||
		static_cast<bool>(keyMods & SDL_KMOD_ALT) 
	);

	if (shouldPan) {
		state.worldWindow.xoffset += relMouseX;
		state.worldWindow.yoffset += relMouseY;
	}

	float mouseX;
	float mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);

	if (mouseButtons & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK)) {
		auto [cellX, cellY] = state.worldWindow.lifeSPointFromScreenPoint(mouseX, mouseY);
		bool value = bool(mouseButtons & SDL_BUTTON_LMASK);
		state.lifeState.world.frontBufferWritable().set(cellX, cellY, value);
	}
}

void LifeControlMode::onIterate(AppState& state) {
	processMouse(state);
}
