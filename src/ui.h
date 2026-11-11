#pragma once

#include <SDL3/SDL.h>

#include "life.h"

namespace ui {
void renderVerticalLine(SDL_Renderer* renderer, float x, float h);
void renderHorizonalLine(SDL_Renderer* renderer, float y, float w);
void renderGridLines(SDL_Renderer* renderer, float xoffset, float yoffset, float cellSize);

// Will eventually replace this with better text rendering, just abstracting
// the debug text for now.
void renderText(SDL_Renderer* renderer, float x, float y, const std::string& str);

// Returns width in x, height in y.
SDL_FPoint calcTextSize(const std::string& str, float textScale=1.0f);

struct Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct UiStyle {
	SDL_FPoint regionPadding { 2.0f, 2.0f };

	float textScale = 2.0f;
	float textPadding = 2.0f;
	Color textColor { 255, 255, 255, 255 };
};

enum class HorizontalAlignment {
	left,
	right
};

enum class VerticalAlignment {
	top,
	bottom
};

struct Ui {
	SDL_Renderer* renderer;
	UiStyle style;

	SDL_FPoint cursor;
	SDL_FRect renderArea;
	HorizontalAlignment halign = HorizontalAlignment::left;
	VerticalAlignment valign = VerticalAlignment::top;

	void setAlign(HorizontalAlignment align);
	void setAlign(VerticalAlignment align);
	void setAlign(HorizontalAlignment h, VerticalAlignment v);
	void setAlign(VerticalAlignment v, HorizontalAlignment h);

	void updateRenderArea();
	void resetCursor();
	void advanceCursor(float w, float h);
	SDL_FPoint calcRenderLocation(float w, float h);

	void text(const std::string& str);

	template<typename... Args>
	void textFormat(std::format_string<Args...> fmt, Args&&... args);
};

bool shouldRenderCellRect(SDL_FRect& rect, int w, int h);

life::SPoint sPointFromFPoint(const SDL_FPoint& pt);

struct WorldWindow {
	static constexpr float minCellSize = 1.0f;

	float zoomSpeed = 0.1f;
	float cellSize = minCellSize;
	float xoffset = 0.0f;
	float yoffset = 0.0f;
	float gridFadeStart = 10.0f;

	void setCellSize(float newCellSize);

	SDL_FPoint screenPointFromLifePoint(size_t x, size_t y);

	// Note: Returned as float values for convenience in a few places.
	SDL_FPoint lifePointFromScreenPoint(float x, float y);

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

#include "ui.inl"
