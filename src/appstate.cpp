#include "appstate.h"

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
