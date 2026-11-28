#include "appstate.h"

static const SDL_DialogFileFilter LIFE_FILE_FILTERS[] {
	{ "Plaintext", "cells" },
	{ "All files", "*" }
};

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

	renderModal(state.uiState.uiBuilder, state.textInputMode.inputBuffer);

	if (input.enterPressed) {
		bool success = state.lifeState.world.rule.setString(input.inputBuffer);
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

void UiState::openLifeFileDialog(AppState& state) {
	if (openLifeFileContext.opened) return;
	sdlutil::openFileDialog(openLifeFileContext, state.window, LIFE_FILE_FILTERS);
}

bool UiState::handleFileLoad(AppState& state) {
	bool loaded = false;
	auto& world = state.lifeState.world;
	life::ResultType<life::CellFile> cellFile;

	if (!openLifeFileContext.done) goto done;
	if (openLifeFileContext.fileList.size() == 0) goto done;

	cellFile = life::loadFile(openLifeFileContext.fileList[0]);
	if (cellFile.error) {
		SDL_Log("Failed to load selected cell file");
		goto done;
	}

	world.backBuffer().clear();
	world.backBuffer().setFromFile(cellFile);
	world.swap();

	state.lifeState.tickTimeBudget = 0;

	loaded = true;
done:
	openLifeFileContext.reset();
	return loaded;
}

void UiState::renderPerfDisplay(AppState& state) {
	if (perfMode == UI_PERF_NONE) return;

	uiBuilder.style.alignment = {
		HorizontalAlignment::left,
		VerticalAlignment::top
	};
	uiBuilder.resetCursor();

	if (perfMode == UI_PERF_BASIC) {
		uiBuilder.textFormat(
			"{:.2f} FPS",
			1000.0f * state.frameTracker.calcFrequency()
		);

		uiBuilder.textFormat(
			"{:.2f} TPS (TARGET: {:.2f})",
			1000.0f * state.lifeState.tickTracker.calcFrequency(),
			1000.0f / state.lifeState.tickTargetTime
		);
	}

	if (perfMode == UI_PERF_ALL) {
		uiBuilder.textFormat(
			"{:.2f} FPS (P{:.2f}:R{:.2f} ms)",
			1000.0f * state.frameTracker.calcFrequency(),
			state.frameTracker.calcPeriod(),
			state.frameTracker.calcAverageTime()
		);

		uiBuilder.textFormat(
			"{:.2f} TPS (P{:.2f}:T{}:R{:.2f} ms)",
			1000.0f * state.lifeState.tickTracker.calcFrequency(),
			state.lifeState.tickTracker.calcPeriod(),
			state.lifeState.tickTargetTime,
			state.lifeState.tickTracker.calcAverageTime()
		);
	}
}

void UiState::render(AppState& state) {
	uiBuilder.beginFrame(state.renderer);

	uiBuilder.styleStack.push({
		.type = ui::UiStylePropertyType::TextBackground,
		.colorValue = { 0, 0, 0, 130 }
	});

	renderPerfDisplay(state);

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

	uiBuilder.textFormat("R: {}", state.lifeState.world.rule.toString());

	uiBuilder.styleStack.pop();

	ruleStringModal.show(state);

	ui::renderDrawData(state.renderer, uiBuilder.drawData);

	if (debugMode > UI_DEBUG_NONE) {
		bool showElements = (debugMode == UI_DEBUG_ALL);
		ui::debugDrawData(
			state.renderer,
			uiBuilder.drawData,
			showElements
		);
	}
}
