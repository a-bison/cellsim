#pragma once

#include <SDL3/SDL.h>

#include "life.h"

namespace display {
void renderVerticalLine(SDL_Renderer* renderer, float x, float h);
void renderHorizonalLine(SDL_Renderer* renderer, float y, float w);
void renderGridLines(SDL_Renderer* renderer, float xoffset, float yoffset, float cellSize);

// Will eventually replace this with better text rendering, just abstracting
// the debug text for now.
void renderText(SDL_Renderer* renderer, float x, float y, const std::string& str);

template<typename... Args>
void renderTextFormat(
	SDL_Renderer* renderer,
	float x,
	float y,
	std::format_string<Args...> fmt,
	Args&&... args
);

// Returns width in x, height in y.
SDL_FPoint calcTextSize(const std::string& str);

bool shouldRenderCellRect(SDL_FRect& rect, int w, int h);

struct WorldWindow {
	static constexpr float minCellSize = 1.0f;

	float zoomSpeed = 0.1f;
	float cellSize = minCellSize;
	float xoffset = 0.0f;
	float yoffset = 0.0f;
	float gridFadeStart = 10.0f;

	void setCellSize(float newCellSize);

	SDL_FPoint sdlPointFromLifePoint(size_t x, size_t y);
	SDL_FPoint sdlPointFromLifePoint(const life::Point& pt);

	life::Point lifePointFromSdlPoint(float x, float y);
	life::Point lifePointFromSdlPoint(const SDL_FPoint& pt);

	void initCellRect(SDL_FRect& rect);
	void updateCellRectPos(SDL_FRect& rect, int x, int y);

	SDL_FRect makeBorderRect(size_t worldWidth, size_t worldHeight);

	// Zooms the window. Negative `amount` zooms out, positive zooms in.
	// x, y are the screen coordinates the zoom was performed at.
	void zoom(float x, float y, float amount);

	template<size_t W, size_t H>
	void renderCells(SDL_Renderer* renderer, const life::World<W, H>& world);
	void renderGrid(SDL_Renderer* renderer);
	void renderBorder(SDL_Renderer* renderer, size_t worldWidth, size_t worldHeight);

	template<size_t W, size_t H>
	void renderWorld(SDL_Renderer* renderer, const life::World<W, H>& world);
};
}

#include "display.inl"
