#include "ui.h"

#include <cmath>

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

SDL_FPoint calcTextSize(const std::string& str) {
	return {
		static_cast<float>(str.size() * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE),
		static_cast<float>(SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE)
	};
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
	SDL_BlendMode oldMode;
	SDL_GetRenderDrawBlendMode(renderer, &oldMode);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(
		renderer,
		50, 50, 50,
		static_cast<uint8_t>(gridAlpha)
	);
	renderGridLines(renderer, xoffset, yoffset, cellSize);
	SDL_SetRenderDrawBlendMode(renderer, oldMode);
}

void WorldWindow::renderBorder(SDL_Renderer* renderer, size_t worldWidth, size_t worldHeight) {
	SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
	auto borderRect = makeBorderRect(worldWidth, worldHeight);
	SDL_RenderRect(renderer, &borderRect);
}
}
