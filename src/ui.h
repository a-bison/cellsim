#pragma once

#include <vector>
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

SDL_FRect offsetRect(const SDL_FRect& rect, float offsetX, float offsetY);

struct Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

enum class HorizontalAlignment {
	left,
	right
};

enum class VerticalAlignment {
	top,
	bottom
};

struct ElementAlignment {
	HorizontalAlignment h;
	VerticalAlignment v;
};

template<typename T>
struct RenderProperty {
	T type;
	union {
		Color colorValue;
		SDL_BlendMode blendModeValue;
		SDL_FPoint fpointValue;
		float floatValue;
		ElementAlignment alignmentValue;
	};
};

enum class SdlRenderPropertyType {
	DrawColor,
	DrawBlendMode,
	Scale
};

using SdlRenderProperty = RenderProperty<SdlRenderPropertyType>;

struct SdlRenderPropertyStack {
	SDL_Renderer* renderer;
	std::vector<SdlRenderProperty> stack;

	SdlRenderPropertyStack(SDL_Renderer* _renderer);

	void push(const SdlRenderProperty& property);
	void pop();
};

SdlRenderPropertyStack& getSdlRenderPropertyStack(SDL_Renderer* renderer);
void pushSdlRenderProperty(SDL_Renderer* renderer, const SdlRenderProperty& property);
void popSdlRenderProperty(SDL_Renderer* renderer);

// Utility functions that handle proper juggling of colors with alpha
void beginSdlTemporaryColor(SDL_Renderer* renderer, const Color& color);
void endSdlTemporaryColor(SDL_Renderer* renderer, const Color& color);

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

using UiStyleProperty = RenderProperty<UiStylePropertyType>;

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
