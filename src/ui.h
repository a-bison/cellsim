#pragma once

#include <vector>
#include <SDL3/SDL.h>

#include "life.h"
#include "sdlutil.h"

using sdlutil::Color;
using sdlutil::ElementAlignment;
using sdlutil::HorizontalAlignment;
using sdlutil::VerticalAlignment;

namespace ui {
struct UiStyle {
	SDL_FPoint regionPadding { 2.0f, 2.0f };

	float textScale = 2.0f;
	float textPadding = 2.0f;
	Color textColor { 255, 255, 255, 255 };
	Color textBackground { 0, 0, 0, 0 };
	ElementAlignment alignment {
		HorizontalAlignment::left,
		VerticalAlignment::top
	};
};

enum class UiStylePropertyType {
	RegionPadding,
	TextScale,
	TextPadding,
	TextColor,
	TextBackground,
	Alignment
};

using UiStyleProperty = sdlutil::GenericProperty<UiStylePropertyType>;

struct UiStylePropertyStack {
	UiStyle* style;
	std::vector<UiStyleProperty> stack;

	UiStylePropertyStack(UiStyle* style);

	void push(const UiStyleProperty& property);
	void pop();
};

enum class UiDrawDataType {
	Rectangle,
	Text
};

struct UiDrawElement {
	UiDrawDataType type;
	SDL_FRect bounds;
	Color color;
	unsigned int dataId;
};

struct UiDrawNode {
	SDL_FRect bounds;
	std::vector<UiDrawNode> children;
	std::vector<UiDrawElement> elements;
};

struct UiDrawRectangleData {
	bool fill;
};

struct UiDrawTextData {
	std::string text;
	float scale;
};

struct UiDrawData {
	std::vector<UiDrawRectangleData> rectData;
	std::vector<UiDrawTextData> textData;

	UiDrawNode root;

	void reset(const SDL_FRect& bounds);
};

void debugDrawData(
	SDL_Renderer* renderer,
	const UiDrawData& data,
	bool showElements
);

void debugDrawNode(
	SDL_Renderer* renderer,
	float offsetX,
	float offsetY,
	const UiDrawNode& node,
	bool showElements
);

void renderDrawData(SDL_Renderer* renderer, const UiDrawData& data);

void renderDrawNode(
	SDL_Renderer* renderer,
	float offsetX,
	float offsetY,
	const UiDrawData& data,
	const UiDrawNode& node
);

void renderDrawRectangleData(
	SDL_Renderer* renderer,
	float offsetX,
	float offsetY,
	const UiDrawElement& element,
	const UiDrawRectangleData& rectData
);

void renderDrawTextData(
	SDL_Renderer* renderer,
	float offsetX,
	float offsetY,
	const UiDrawElement& element,
	const UiDrawTextData& textData
);

struct Ui {
	SDL_Renderer* renderer;
	UiStyle style;
	UiStylePropertyStack styleStack { &style };

	UiDrawData drawData;
	std::vector<UiDrawNode*> nodeStack;

	SDL_FPoint cursor;
	SDL_FRect renderArea;

	Ui() = default;
	Ui(const Ui&) = delete;
	Ui(Ui&&) = delete;

	void beginFrame();

	UiDrawNode& currentNode();
	void beginNode(float w, float h);
	void endNode();
	void emitRectangle(
		const SDL_FRect& bounds,
		const Color& color,
		bool fill
	);
	void emitText(
		const SDL_FRect& bounds,
		const Color& color,
		float textScale,
		const std::string& text
	);

	void setCursorFromBounds(const SDL_FRect& bounds);
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

	life::SPoint lifeSPointFromScreenPoint(float x, float y);

	void startCellRect(SDL_FRect& rect, int x, int y);
	void expandCellRect(SDL_FRect& rect);

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
