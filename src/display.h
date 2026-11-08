#pragma once

#include <SDL3/SDL.h>

#include "life.h"

namespace display {
void renderVerticalLine(SDL_Renderer* renderer, float x, float h);
void renderHorizonalLine(SDL_Renderer* renderer, float y, float w);
void renderGrid(SDL_Renderer* renderer, float xoffset, float yoffset, float cellSize);

bool shouldRenderCellRect(SDL_FRect& rect, int w, int h);

struct WorldWindow {
	float cellSize = 1.0f;
	float xoffset = 0.0f;
	float yoffset = 0.0f;

	SDL_FPoint sdlPointFromLifePoint(size_t x, size_t y);
	SDL_FPoint sdlPointFromLifePoint(const life::Point& pt);

	life::Point lifePointFromSdlPoint(float x, float y);
	life::Point lifePointFromSdlPoint(const SDL_FPoint& pt);

	void initCellRect(SDL_FRect& rect);
	void updateCellRectPos(SDL_FRect& rect, int x, int y);

	SDL_FRect makeBorderRect(size_t worldWidth, size_t worldHeight);

	template<size_t W, size_t H>
	void renderWorld(SDL_Renderer* renderer, const life::World<W, H>& world);
};
}

#include "display.inl"
