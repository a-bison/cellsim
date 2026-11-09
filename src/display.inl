namespace display {
template<typename... Args>
void renderTextFormat(
	SDL_Renderer* renderer,
	float x,
	float y,
	std::format_string<Args...> fmt,
	Args&&... args
) {
	auto s = std::format(fmt, std::forward<Args>(args)...);
	renderText(renderer, x, y, s);
}

template<size_t W, size_t H>
void WorldWindow::renderCells(SDL_Renderer* renderer, const life::World<W, H>& world) {
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
}

template<size_t W, size_t H>
void WorldWindow::renderWorld(SDL_Renderer* renderer, const life::World<W, H>& world) {
	renderCells(renderer, world);
	renderGrid(renderer);
	renderBorder(renderer, W, H);
}
}
