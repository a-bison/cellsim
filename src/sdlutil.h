#pragma once

#include <string>
#include <vector>
#include <SDL3/SDL.h>

namespace sdlutil {
void renderVerticalLine(SDL_Renderer* renderer, float x, float h);
void renderHorizonalLine(SDL_Renderer* renderer, float y, float w);
void renderGridLines(SDL_Renderer* renderer, float xoffset, float yoffset, float cellSize);

// Will eventually replace this with better text rendering, just abstracting
// the debug text for now.
void renderText(SDL_Renderer* renderer, float x, float y, const std::string& str);

// Returns width in x, height in y.
SDL_FPoint calcTextSize(const std::string& str, float textScale=1.0f);

// Translates a rectangle.
SDL_FRect offsetRect(const SDL_FRect& rect, float offsetX, float offsetY);

// Zeros out the x,y coordinates of a rectangle.
SDL_FRect deoffsetRect(const SDL_FRect& rect);

struct Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

enum class HorizontalAlignment {
	left,
	center,
	right
};

enum class VerticalAlignment {
	top,
	center,
	bottom
};

struct ElementAlignment {
	HorizontalAlignment h;
	VerticalAlignment v;
};

template<typename T>
struct GenericProperty {
	T type;
	union {
		Color colorValue;
		SDL_BlendMode blendModeValue;
		SDL_FPoint fpointValue;
		float floatValue;
		ElementAlignment alignmentValue;
	};
};

enum class RenderPropertyType {
	DrawColor,
	DrawBlendMode,
	Scale
};

using RenderProperty = GenericProperty<RenderPropertyType>;

struct RenderPropertyStack {
	SDL_Renderer* renderer;
	std::vector<RenderProperty> stack;

	RenderPropertyStack(SDL_Renderer* _renderer);

	void push(const RenderProperty& property);
	void pop();
};

RenderPropertyStack& getRenderPropertyStack(SDL_Renderer* renderer);
void pushRenderProperty(SDL_Renderer* renderer, const RenderProperty& property);
void popRenderProperty(SDL_Renderer* renderer);

// Utility functions that handle proper juggling of colors with alpha
void beginTemporaryColor(SDL_Renderer* renderer, const Color& color);
void endTemporaryColor(SDL_Renderer* renderer, const Color& color);
}
