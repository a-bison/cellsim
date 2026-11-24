#include "appstate.h"

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

void UiState::cycleDebugMode() {
	debugMode = static_cast<UiDebugMode>((debugMode + 1) % UI_DEBUG_MODE_COUNT);
}

void UiState::render(AppState& state) {
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
