#include "display.h"

#include <cmath>

namespace display {
void renderVerticalLine(SDL_Renderer* renderer, float x, float h) {
	SDL_RenderLine(renderer, x, 0.0f, x, h);
}

void renderHorizonalLine(SDL_Renderer* renderer, float y, float w) {
	SDL_RenderLine(renderer, 0.0f, y, w, y);
}

void renderGrid(SDL_Renderer* renderer, float offset, float cellSize) {
	int w_i = 0;
	int h_i = 0;

	SDL_GetCurrentRenderOutputSize(renderer, &w_i, &h_i);

	float w = static_cast<float>(w_i);
	float h = static_cast<float>(h_i);

	offset = std::fmod(offset, cellSize);

	float x = offset;
	while (x < w) {
		renderVerticalLine(renderer, x, h);
		x += cellSize;
	}

	float y = offset;
	while (y < h) {
		renderHorizonalLine(renderer, y, w);
		y += cellSize;
	}
}

bool shouldRenderCellRect(SDL_FRect& rect, int w, int h) {
	if (rect.x > w || rect.y > h) return false;
	if (rect.x + rect.w < 0 || rect.y + rect.h < 0) return false;

	return true;
}

SDL_FPoint WorldWindow::sdlPointFromLifePoint(size_t x, size_t y) {
	return { xoffset + x*cellSize, yoffset + y*cellSize };
}

SDL_FPoint WorldWindow::sdlPointFromLifePoint(const life::Point& pt) {
	return sdlPointFromLifePoint(pt.x, pt.y);
}

life::Point WorldWindow::lifePointFromSdlPoint(float x, float y) {
	return { 
		static_cast<size_t>(std::floor((x-xoffset)/cellSize)),
		static_cast<size_t>(std::floor((y-yoffset)/cellSize))
	};
}

life::Point WorldWindow::lifePointFromSdlPoint(const SDL_FPoint& pt) {
	return lifePointFromSdlPoint(pt.x, pt.y);
}

void WorldWindow::initCellRect(SDL_FRect& rect) {
	rect.x = 0.0f;
	rect.y = 0.0f;
	rect.w = cellSize;
	rect.h = cellSize;
}

void WorldWindow::updateCellRectPos(SDL_FRect& rect, int x, int y) {
	auto pt = sdlPointFromLifePoint(x, y);
	rect.x = pt.x;
	rect.y = pt.y;
}
}
