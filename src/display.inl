namespace display {
template<size_t W, size_t H>
void WorldWindow::renderWorld(SDL_Renderer* renderer, const life::World<W, H>& world) {
	// Draw cells
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_FRect cellRect;
	initCellRect(cellRect);
	for (int y = 0; y < H; y++) {
		for (int x = 0; x < W; x++) {
			if (!world.frontBuffer().get(x, y)) continue;

			updateCellRectPos(cellRect, x, y);
			SDL_RenderFillRect(renderer, &cellRect);
		}
	}

	// Draw grid
	float gridAlpha = 255.0f;
	if (cellSize <= gridFadeStart) {
		gridAlpha *= (cellSize - minCellSize) / (gridFadeStart - minCellSize);
	}
	uint8_t gridAlpha_int = static_cast<uint8_t>(gridAlpha);
	SDL_BlendMode oldMode;
	SDL_GetRenderDrawBlendMode(renderer, &oldMode);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 50, 50, 50, gridAlpha_int);
	renderGrid(renderer, xoffset, yoffset, cellSize);
	SDL_SetRenderDrawBlendMode(renderer, oldMode);

	// Draw border
	SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
	auto borderRect = makeBorderRect(W, H);
	SDL_RenderRect(renderer, &borderRect);
}
}
