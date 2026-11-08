#pragma once

#include <SDL3/SDL.h>

namespace display {
void renderVerticalLine(SDL_Renderer* renderer, float x, float h);
void renderHorizonalLine(SDL_Renderer* renderer, float y, float w);
void renderGrid(SDL_Renderer* renderer, float offset, float cellSize);

void initCellRect(SDL_FRect& rect, float cellSize);
void fillCellRect(SDL_FRect& rect, float offset, float cellSize, int x, int y);
bool shouldRenderCellRect(SDL_FRect& rect, int w, int h);
}
