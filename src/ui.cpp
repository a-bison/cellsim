#include "ui.h"

#include <cmath>
#include <unordered_map>

static std::unordered_map<void*, ui::SdlRenderPropertyStack> __sdlPropStacks;

namespace ui {
void renderVerticalLine(SDL_Renderer* renderer, float x, float h) {
	SDL_RenderLine(renderer, x, 0.0f, x, h);
}

void renderHorizonalLine(SDL_Renderer* renderer, float y, float w) {
	SDL_RenderLine(renderer, 0.0f, y, w, y);
}

void renderGridLines(SDL_Renderer* renderer, float xoffset, float yoffset, float cellSize) {
	int w_i = 0;
	int h_i = 0;

	SDL_GetCurrentRenderOutputSize(renderer, &w_i, &h_i);

	float w = static_cast<float>(w_i);
	float h = static_cast<float>(h_i);

	xoffset = std::fmod(xoffset, cellSize);
	yoffset = std::fmod(yoffset, cellSize);

	float x = xoffset;
	while (x <= w) {
		renderVerticalLine(renderer, x, h);
		x += cellSize;
	}

	float y = yoffset;
	while (y <= h) {
		renderHorizonalLine(renderer, y, w);
		y += cellSize;
	}
}

void renderText(SDL_Renderer* renderer, float x, float y, const std::string& str) {
	SDL_RenderDebugText(renderer, x, y, str.c_str());
}

SDL_FPoint calcTextSize(const std::string& str, float textScale) {
	return {
		static_cast<float>(str.size() * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) * textScale,
		static_cast<float>(SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) * textScale
	};
}

SdlRenderPropertyStack::SdlRenderPropertyStack(SDL_Renderer* _renderer)
: renderer { _renderer } {}

void SdlRenderPropertyStack::push(const SdlRenderProperty& property) {
	SdlRenderProperty pushedProperty;
	pushedProperty.type = property.type;

	switch (property.type) {
	case SdlRenderPropertyType::DrawColor:
		SDL_GetRenderDrawColor(
			renderer,
			&pushedProperty.colorValue.r,
			&pushedProperty.colorValue.g,
			&pushedProperty.colorValue.b,
			&pushedProperty.colorValue.a
		);
		SDL_SetRenderDrawColor(
			renderer,
			property.colorValue.r,
			property.colorValue.g,
			property.colorValue.b,
			property.colorValue.a
		);
		break;
	case SdlRenderPropertyType::DrawBlendMode:
		SDL_GetRenderDrawBlendMode(renderer, &pushedProperty.blendModeValue);
		SDL_SetRenderDrawBlendMode(renderer, property.blendModeValue);
		break;
	case SdlRenderPropertyType::Scale:
		SDL_GetRenderScale(
			renderer,
			&pushedProperty.fpointValue.x,
			&pushedProperty.fpointValue.y
		);
		SDL_SetRenderScale(
			renderer,
			property.fpointValue.x,
			property.fpointValue.y
		);
		break;
	default:
		break;
	}

	stack.push_back(pushedProperty);
}

void SdlRenderPropertyStack::pop() {
	if (stack.size() == 0) return;

	auto& property = stack.back();

	switch (property.type) {
	case SdlRenderPropertyType::DrawColor:
		SDL_SetRenderDrawColor(
			renderer,
			property.colorValue.r,
			property.colorValue.g,
			property.colorValue.b,
			property.colorValue.a
		);
		break;
	case SdlRenderPropertyType::DrawBlendMode:
		SDL_SetRenderDrawBlendMode(renderer, property.blendModeValue);
		break;
	case SdlRenderPropertyType::Scale:
		SDL_SetRenderScale(
			renderer,
			property.fpointValue.x,
			property.fpointValue.y
		);
		break;
	default:
		break;
	}

	stack.pop_back();
}

SdlRenderPropertyStack& getSdlRenderPropertyStack(SDL_Renderer* renderer) {
	void* vptr = reinterpret_cast<void*>(renderer);

	if (!__sdlPropStacks.contains(renderer)) {
		__sdlPropStacks.emplace(vptr, renderer);
	}

	return __sdlPropStacks.at(vptr);
}

void pushSdlRenderProperty(SDL_Renderer* renderer, const SdlRenderProperty& property) {
	getSdlRenderPropertyStack(renderer).push(property);
}

void popSdlRenderProperty(SDL_Renderer* renderer) {
	getSdlRenderPropertyStack(renderer).pop();
}

void Ui::setAlign(HorizontalAlignment align) { halign = align; }

void Ui::setAlign(VerticalAlignment align) { valign = align; }

void Ui::setAlign(HorizontalAlignment h, VerticalAlignment v) {
	halign = h;
	valign = v;
}

void Ui::setAlign(VerticalAlignment v, HorizontalAlignment h) {
	halign = h;
	valign = v;
}

void Ui::updateRenderArea() {
	renderArea.x = style.regionPadding.x;
	renderArea.y = style.regionPadding.y;

	int w;
	int h;
	SDL_GetRenderOutputSize(renderer, &w, &h);

	renderArea.w = static_cast<float>(w) - style.regionPadding.x;
	renderArea.h = static_cast<float>(h) - style.regionPadding.y;
}

void Ui::resetCursor() {
	if (halign == HorizontalAlignment::left) {
		cursor.x = renderArea.x;
	} else {
		cursor.x = renderArea.x + renderArea.w;
	}

	if (valign == VerticalAlignment::top) {
		cursor.y = renderArea.y;
	} else {
		cursor.y = renderArea.y + renderArea.h;
	}
}

void Ui::advanceCursor(float w, float h) {
	if (valign == VerticalAlignment::top) {
		cursor.y += h;
	} else {
		cursor.y -= h;
	}
}

SDL_FPoint Ui::calcRenderLocation(float w, float h) {
	SDL_FPoint pt;

	if (halign == HorizontalAlignment::left) {
		pt.x = cursor.x;
	} else {
		pt.x = cursor.x - w;
	}

	if (valign == VerticalAlignment::top) {
		pt.y = cursor.y;
	} else {
		pt.y = cursor.y - h;
	}

	return pt;
}

void Ui::text(const std::string& str) {
	pushSdlRenderProperty(renderer, {
		.type = SdlRenderPropertyType::Scale,
		.fpointValue = { style.textScale, style.textScale }
	});

	pushSdlRenderProperty(renderer, {
		.type = SdlRenderPropertyType::DrawColor,
		.colorValue = style.textColor
	});

	auto [w, h] = calcTextSize(str, style.textScale);
	auto renderPt = calcRenderLocation(w, h);
	renderText(renderer, renderPt.x/style.textScale, renderPt.y/style.textScale, str);
	advanceCursor(w+style.textPadding, h+style.textPadding);

	popSdlRenderProperty(renderer);
	popSdlRenderProperty(renderer);
}

bool shouldRenderCellRect(SDL_FRect& rect, int w, int h) {
	if (rect.x > w || rect.y > h) return false;
	if (rect.x + rect.w < 0 || rect.y + rect.h < 0) return false;

	return true;
}

life::SPoint sPointFromFPoint(const SDL_FPoint& pt) {
	return {
		static_cast<life::signed_size_t>(std::floor(pt.x)),
		static_cast<life::signed_size_t>(std::floor(pt.y))
	};
}

void WorldWindow::setCellSize(float newCellSize) {
	cellSize = (newCellSize > minCellSize) ? newCellSize : minCellSize;
}

SDL_FPoint WorldWindow::screenPointFromLifePoint(size_t x, size_t y) {
	return { xoffset + x*cellSize, yoffset + y*cellSize };
}

SDL_FPoint WorldWindow::lifePointFromScreenPoint(float x, float y) {
	return { 
		(x-xoffset)/cellSize,
		(y-yoffset)/cellSize
	};
}

life::SPoint WorldWindow::lifeSPointFromScreenPoint(float x, float y) {
	auto pt = lifePointFromScreenPoint(x, y);
	return sPointFromFPoint(pt);
}

void WorldWindow::initCellRect(SDL_FRect& rect) {
	rect.x = 0.0f;
	rect.y = 0.0f;
	rect.w = cellSize;
	rect.h = cellSize;
}

void WorldWindow::updateCellRectPos(SDL_FRect& rect, int x, int y) {
	auto pt = screenPointFromLifePoint(x, y);
	rect.x = pt.x;
	rect.y = pt.y;
}

SDL_FRect WorldWindow::makeBorderRect(size_t worldWidth, size_t worldHeight) {
	return {
		xoffset,
		yoffset,
		cellSize*worldWidth,
		cellSize*worldHeight
	};
}

void WorldWindow::zoom(float x, float y, float amount) {
	// Position we're zooming in/out at, in life world coordinates.
	auto [gridx, gridy] = lifePointFromScreenPoint(x, y);

	float newCellSize = cellSize + cellSize*amount*zoomSpeed;
	if (newCellSize < minCellSize) newCellSize = minCellSize;
	cellSize = newCellSize;

	auto [newGridX, newGridY] = lifePointFromScreenPoint(x, y);

	float errorx = gridx - newGridX;
	float errory = gridy - newGridY;

	xoffset -= errorx*newCellSize;
	yoffset -= errory*newCellSize;
}

void WorldWindow::renderGrid(SDL_Renderer* renderer) {
	float gridAlpha = 255.0f;
	if (cellSize <= gridFadeStart) {
		gridAlpha *= (cellSize - minCellSize) / (gridFadeStart - minCellSize);
	}

	pushSdlRenderProperty(renderer, {
		.type = SdlRenderPropertyType::DrawBlendMode,
		.blendModeValue = SDL_BLENDMODE_BLEND
	});

	SDL_SetRenderDrawColor(
		renderer,
		50, 50, 50,
		static_cast<uint8_t>(gridAlpha)
	);
	renderGridLines(renderer, xoffset, yoffset, cellSize);

	popSdlRenderProperty(renderer);
}

void WorldWindow::renderBorder(SDL_Renderer* renderer, size_t worldWidth, size_t worldHeight) {
	SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
	auto borderRect = makeBorderRect(worldWidth, worldHeight);
	SDL_RenderRect(renderer, &borderRect);
}
}
