#include "appstate.h"

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
