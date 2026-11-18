#pragma once

#include <SDL3/SDL.h>

#include "ui.h"
#include "life.h"
#include "sdlutil.h"

using sdlutil::HorizontalAlignment;
using sdlutil::VerticalAlignment;

constexpr size_t WINDOW_WIDTH = 800;
constexpr size_t WINDOW_HEIGHT = WINDOW_WIDTH;
constexpr size_t WORLD_WIDTH = 400;
constexpr size_t WORLD_HEIGHT = WORLD_WIDTH;
constexpr float CELLSIZE = WINDOW_WIDTH / WORLD_WIDTH;

struct AppState;

class AppMode {
public:
	virtual SDL_AppResult onEvent(AppState& state, SDL_Event *event) {
		return SDL_APP_CONTINUE;
	}
	virtual void onIterate(AppState& state) {}
	virtual void onSwitchIn(AppState& state) {}
	virtual void onSwitchOut(AppState& state) {}
	virtual void onEndFrame(AppState& state) {}
	virtual unsigned int getId() = 0;

	virtual ~AppMode() = default;
};

struct LifeState {
	unsigned int tickms = 100;
	uint64_t lastTickTime = 0;
	float averageTickTime = 0.0f;
	bool paused = false;
};

// Deliberately using non-class enum here for more convenient cycling debug
// toggle
enum UiDebugMode {
	UI_DEBUG_NONE = 0,
	UI_DEBUG_NODES,
	UI_DEBUG_ALL,
	UI_DEBUG_MODE_COUNT
};

struct RuleStringModal {
	bool active = false;
	bool showError = false;

	void renderModal(ui::UiBuilder& uiBuilder, const std::string& buffer);
	void show(AppState& state);
	void open(AppState& state);
	void close(AppState& state);
};

struct UiState {
	RuleStringModal ruleStringModal;
	UiDebugMode debugMode = UI_DEBUG_NONE;
};

class LifeControlMode : public AppMode {
public:
	static constexpr unsigned int id = 0;

	unsigned int getId() override { return id; }

	SDL_AppResult handleKeyDown(AppState& state, SDL_KeyboardEvent& event);
	void handleMouseWheel(AppState& state, SDL_MouseWheelEvent& event);
	SDL_AppResult onEvent(AppState& state, SDL_Event *event) override;

	void processMouse(AppState& state);
	void onIterate(AppState& state) override;
};

class TextInputMode : public AppMode {
public:
	static constexpr unsigned int id = LifeControlMode::id + 1;

	unsigned int getId() override { return id; }

	unsigned int charLimit = 25;
	std::string inputBuffer;

	bool cancelled = false;
	bool enterPressed = false;
	bool textChanged = false;

	void pasteText();

	void handleKeyDown(AppState& state, SDL_KeyboardEvent& event);
	void handleTextInput(AppState& state, SDL_TextInputEvent& event);
	SDL_AppResult onEvent(AppState& state, SDL_Event *event) override;

	void resetFlags();

	void onEndFrame(AppState& state) override;
	void onSwitchIn(AppState& state) override;
	void onSwitchOut(AppState& state) override;
};

struct AppState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	ui::UiBuilder uiBuilder;
	UiState uiState;

	uint64_t lastTime = 0;
	float averageFrameTime = 0.0f;

	life::World<WORLD_WIDTH, WORLD_HEIGHT> world;
	ui::WorldWindow worldWindow;
	LifeState lifeState;

	LifeControlMode lifeControlMode;
	TextInputMode textInputMode;

	AppMode* currentMode;

	void setMode(unsigned int id);
};
