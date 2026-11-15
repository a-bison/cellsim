#include "sdlutil.h"

#include <cmath>
#include <unordered_map>

static std::unordered_map<void*, sdlutil::RenderPropertyStack> __propStacks;

namespace sdlutil {
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

SDL_FRect offsetRect(const SDL_FRect& rect, float offsetX, float offsetY) {
	return {
		rect.x + offsetX,
		rect.y + offsetY,
		rect.w,
		rect.h
	};
}

RenderPropertyStack::RenderPropertyStack(SDL_Renderer* _renderer)
: renderer { _renderer } {}

void RenderPropertyStack::push(const RenderProperty& property) {
	RenderProperty pushedProperty;
	pushedProperty.type = property.type;

	switch (property.type) {
	case RenderPropertyType::DrawColor:
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
	case RenderPropertyType::DrawBlendMode:
		SDL_GetRenderDrawBlendMode(renderer, &pushedProperty.blendModeValue);
		SDL_SetRenderDrawBlendMode(renderer, property.blendModeValue);
		break;
	case RenderPropertyType::Scale:
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

void RenderPropertyStack::pop() {
	if (stack.size() == 0) return;

	auto& property = stack.back();

	switch (property.type) {
	case RenderPropertyType::DrawColor:
		SDL_SetRenderDrawColor(
			renderer,
			property.colorValue.r,
			property.colorValue.g,
			property.colorValue.b,
			property.colorValue.a
		);
		break;
	case RenderPropertyType::DrawBlendMode:
		SDL_SetRenderDrawBlendMode(renderer, property.blendModeValue);
		break;
	case RenderPropertyType::Scale:
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

RenderPropertyStack& getRenderPropertyStack(SDL_Renderer* renderer) {
	void* vptr = reinterpret_cast<void*>(renderer);

	if (!__propStacks.contains(renderer)) {
		__propStacks.emplace(vptr, renderer);
	}

	return __propStacks.at(vptr);
}

void pushRenderProperty(SDL_Renderer* renderer, const RenderProperty& property) {
	getRenderPropertyStack(renderer).push(property);
}

void popRenderProperty(SDL_Renderer* renderer) {
	getRenderPropertyStack(renderer).pop();
}

void beginTemporaryColor(SDL_Renderer* renderer, const Color& color) {
	pushRenderProperty(renderer, {
		.type = RenderPropertyType::DrawColor,
		.colorValue = color
	});

	if (color.a < 255) {
		pushRenderProperty(renderer, {
			.type = RenderPropertyType::DrawBlendMode,
			.blendModeValue = SDL_BLENDMODE_BLEND
		});
	}
}

void endTemporaryColor(SDL_Renderer* renderer, const Color& color) {
	popRenderProperty(renderer);
	if (color.a < 255) popRenderProperty(renderer);
}
}
