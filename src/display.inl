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
	SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
	renderGrid(renderer, xoffset, yoffset, cellSize);

	// Draw border
	SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
	auto borderRect = makeBorderRect(W, H);
	SDL_RenderRect(renderer, &borderRect);
}
}
