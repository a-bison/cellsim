#pragma once

#include <SDL3/SDL.h>

#include "ui.h"
#include "life.h"
#include "measurement.h"
#include "sdlutil.h"

using sdlutil::HorizontalAlignment;
using sdlutil::VerticalAlignment;

constexpr size_t WINDOW_WIDTH = 800;
constexpr size_t WINDOW_HEIGHT = WINDOW_WIDTH;
constexpr size_t WORLD_WIDTH = 400;
constexpr size_t WORLD_HEIGHT = WORLD_WIDTH;
constexpr float CELLSIZE = WINDOW_WIDTH / WORLD_WIDTH;

constexpr uint64_t FPS_MEASUREMENT_PERIOD_MS = 2000;
constexpr uint64_t TPS_MEASUREMENT_PERIOD_MS = 2000;

struct AppState;

template<typename T> inline constexpr T ENUM_COUNT {};
#define DECLARE_ENUM_COUNT(T, C) template<> inline constexpr T ENUM_COUNT<T> = C;

template<typename T>
T cycleEnum(const T& e);

template<typename T>
struct CyclableEnum {
	T e;
	void cycle();
	operator T&();
};

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

template<size_t W, size_t H>
struct LifeState {
	bool paused = false;
	uint64_t tickTargetTime = 100;
	uint64_t tickTimeBudget = 0;
	measurement::TimeTracker tickTracker { TPS_MEASUREMENT_PERIOD_MS };

	life::World<W, H> world;

	void initializeWorld(const std::string& ruleString);
	void processTick(uint64_t deltaTime);
};

// Deliberately using non-class enum here for more convenient cycling debug
// toggle
enum UiDebugMode {
	UI_DEBUG_NONE = 0,
	UI_DEBUG_NODES,
	UI_DEBUG_ALL,
	UI_DEBUG_MODE_COUNT
};

DECLARE_ENUM_COUNT(UiDebugMode, UI_DEBUG_MODE_COUNT);

enum UiPerfMode {
	UI_PERF_NONE = 0,
	UI_PERF_BASIC,
	UI_PERF_ALL,
	UI_PERF_MODE_COUNT
};

DECLARE_ENUM_COUNT(UiPerfMode, UI_PERF_MODE_COUNT);

struct RuleStringModal {
	bool active = false;
	bool showError = false;

	void renderModal(ui::UiBuilder& uiBuilder, const std::string& buffer);
	void show(AppState& state);
	void open(AppState& state);
	void close(AppState& state);
};

struct UiState {
	ui::UiBuilder uiBuilder;
	RuleStringModal ruleStringModal;
	CyclableEnum<UiDebugMode> debugMode { UI_DEBUG_NONE };
	CyclableEnum<UiPerfMode> perfMode { UI_PERF_BASIC };

	void renderPerfDisplay(AppState& state);
	void render(AppState& state);
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

	measurement::TimeTracker frameTracker { FPS_MEASUREMENT_PERIOD_MS };

	UiState uiState;
	LifeState<WORLD_WIDTH, WORLD_HEIGHT> lifeState;
	ui::WorldWindow worldWindow;

	LifeControlMode lifeControlMode;
	TextInputMode textInputMode;

	AppMode* currentMode;

	void setMode(unsigned int id);
};

#include "appstate.inl"
#include "appstate_lifestate.inl"
