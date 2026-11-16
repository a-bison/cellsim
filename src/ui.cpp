#include "ui.h"

#include <cmath>
#include <unordered_map>

namespace ui {
UiStylePropertyStack::UiStylePropertyStack(UiStyle* _style) : style { _style } {}

void UiStylePropertyStack::push(const UiStyleProperty& property) {
	UiStyleProperty pushedProperty;
	pushedProperty.type = property.type;

	// TODO: This sucks.
	switch (property.type) {
	case UiStylePropertyType::RegionPadding:
		pushedProperty.fpointValue = style->regionPadding;
		style->regionPadding = property.fpointValue;
		break;
	case UiStylePropertyType::TextScale:
		pushedProperty.floatValue = style->textScale;
		style->textScale = property.floatValue;
		break;
	case UiStylePropertyType::TextPadding:
		pushedProperty.floatValue = style->textPadding;
		style->textPadding = property.floatValue;
		break;
	case UiStylePropertyType::TextColor:
		pushedProperty.colorValue = style->textColor;
		style->textColor = property.colorValue;
		break;
	case UiStylePropertyType::TextBackground:
		pushedProperty.colorValue = style->textBackground;
		style->textBackground = property.colorValue;
		break;
	case UiStylePropertyType::Alignment:
		pushedProperty.alignmentValue = style->alignment;
		style->alignment = property.alignmentValue;
	default:
		break;
	}

	stack.push_back(pushedProperty);
}

void UiStylePropertyStack::pop() {
	if (stack.size() == 0) return;

	auto& property = stack.back();

	switch (property.type) {
	case UiStylePropertyType::RegionPadding:
		style->regionPadding = property.fpointValue;
		break;
	case UiStylePropertyType::TextScale:
		style->textScale = property.floatValue;
		break;
	case UiStylePropertyType::TextPadding:
		style->textPadding = property.floatValue;
		break;
	case UiStylePropertyType::TextColor:
		style->textColor = property.colorValue;
		break;
	case UiStylePropertyType::TextBackground:
		style->textBackground = property.colorValue;
		break;
	case UiStylePropertyType::Alignment:
		style->alignment = property.alignmentValue;
		break;
	default:
		break;
	}

	stack.pop_back();
}

SDL_FRect UiDrawNode::autoFit() {
	std::vector<SDL_FPoint> points;

	for (const auto& child : children) {
		points.push_back({child.bounds.x, child.bounds.y});
		points.push_back({
			child.bounds.x + child.bounds.w,
			child.bounds.y + child.bounds.h
		});
	}

	SDL_FRect result;

	SDL_GetRectEnclosingPointsFloat(
		points.data(),
		points.size(),
		NULL,
		&result
	);

	return result;
}

void UiDrawData::reset(const SDL_FRect& bounds) {
	rectData.clear();
	textData.clear();
	root.children.clear();
	root.elements.clear();

	this->root.bounds = bounds;
}

void debugDrawData(
	SDL_Renderer* renderer,
	const UiDrawData& data,
	bool showElements
) {
	sdlutil::pushRenderProperty(renderer, {
		.type = sdlutil::RenderPropertyType::DrawColor,
		.colorValue = { 255, 0, 0, 255 }
	});
	debugDrawNode(renderer, 0.0f, 0.0f, data.root, showElements);
	sdlutil::popRenderProperty(renderer);
}

void debugDrawNode(
	SDL_Renderer* renderer,
	float offsetX,
	float offsetY,
	const UiDrawNode& node,
	bool showElements
) {
	auto nodeRect = sdlutil::translatedRect(node.bounds, offsetX, offsetY);
	SDL_RenderRect(renderer, &nodeRect);

	if (showElements) {
		sdlutil::pushRenderProperty(renderer, {
			.type = sdlutil::RenderPropertyType::DrawColor,
			.colorValue = { 255, 255, 0, 255 }
		});
		SDL_FRect elemRect;
		for (const auto& element : node.elements) {
			elemRect = sdlutil::translatedRect(node.bounds, offsetX, offsetY);
			SDL_RenderRect(renderer, &elemRect);
		}
		sdlutil::popRenderProperty(renderer);
	}

	for (const auto& child : node.children) {
		debugDrawNode(
			renderer,
			offsetX + node.bounds.x,
			offsetY + node.bounds.y,
			child,
			showElements
		);
	}
}

void renderDrawData(SDL_Renderer* renderer, const UiDrawData& data) {
	renderDrawNode(renderer, 0.0f, 0.0f, data, data.root);
}

void renderDrawNode(
	SDL_Renderer* renderer,
	float offsetX,
	float offsetY,
	const UiDrawData& data,
	const UiDrawNode& node
) {
	float newOffsetX = offsetX + node.bounds.x;
	float newOffsetY = offsetY + node.bounds.y;

	for (const auto& elem : node.elements) {
		switch (elem.type) {
		case UiDrawDataType::Rectangle:
			renderDrawRectangleData(
				renderer,
				newOffsetX,
				newOffsetY,
				elem,
				data.rectData[elem.dataId]
			);
			break;
		case UiDrawDataType::Text:
			renderDrawTextData(
				renderer,
				newOffsetX,
				newOffsetY,
				elem,
				data.textData[elem.dataId]
			);
			break;
		default:
			break;
		}
	}

	for (const auto& child : node.children) {
		renderDrawNode(
			renderer,
			newOffsetX,
			newOffsetY,
			data,
			child
		);
	}
}

void renderDrawRectangleData(
	SDL_Renderer* renderer,
	float offsetX,
	float offsetY,
	const UiDrawElement& element,
	const UiDrawRectangleData& rectData
) {
	auto tmpRect = sdlutil::translatedRect(element.bounds, offsetX, offsetY);

	sdlutil::beginTemporaryColor(renderer, element.color);
	if (rectData.fill) {
		SDL_RenderFillRect(renderer, &tmpRect);
	} else {
		SDL_RenderRect(renderer, &tmpRect);
	}
	sdlutil::endTemporaryColor(renderer, element.color);
}

void renderDrawTextData(
	SDL_Renderer* renderer,
	float offsetX,
	float offsetY,
	const UiDrawElement& element,
	const UiDrawTextData& textData
) {
	sdlutil::beginTemporaryColor(renderer, element.color);
	sdlutil::pushRenderProperty(renderer, {
		.type = sdlutil::RenderPropertyType::Scale,
		.fpointValue = {textData.scale, textData.scale}
	});
	sdlutil::renderText(
		renderer,
		(offsetX + element.bounds.x)/textData.scale,
		(offsetY + element.bounds.y)/textData.scale,
		textData.text
	);
	sdlutil::popRenderProperty(renderer);
	sdlutil::endTemporaryColor(renderer, element.color);
}

void UiBuilder::beginFrame(SDL_Renderer* renderer) {
	nodeStack.clear();

	int w;
	int h;
	SDL_GetRenderOutputSize(renderer, &w, &h);

	SDL_FRect bounds {
		style.regionPadding.x,
		style.regionPadding.y,
		static_cast<float>(w) - 2*style.regionPadding.x,
		static_cast<float>(h) - 2*style.regionPadding.y
	};

	drawData.reset(bounds);
	nodeStack.push_back(&drawData.root);
}

UiDrawNode& UiBuilder::currentNode() {
	return *nodeStack.back();
}

void UiBuilder::beginNode(float w, float h) {
	currentNode().children.push_back({});
	auto& newNode = currentNode().children.back();

	auto nodePos = calcRenderLocation(w, h);

	newNode.bounds.x = nodePos.x;
	newNode.bounds.y = nodePos.y;
	newNode.bounds.w = w;
	newNode.bounds.h = h;

	nodeStack.push_back(&newNode);
}

void UiBuilder::endNode() {
	nodeStack.pop_back();
}

void UiBuilder::emitRectangle(
	const SDL_FRect& bounds,
	const Color& color,
	bool fill
) {
	drawData.rectData.push_back({fill});
	unsigned int id = drawData.rectData.size() - 1;

	currentNode().elements.push_back({
		UiDrawDataType::Rectangle,
		bounds,
		color,
		id
	});
}

void UiBuilder::emitText(
	const SDL_FRect& bounds,
	const Color& color,
	float textScale,
	const std::string& text
) {
	drawData.textData.push_back({text, textScale});
	unsigned int id = drawData.textData.size() - 1;

	currentNode().elements.push_back({
		UiDrawDataType::Text,
		bounds,
		color,
		id
	});
}

void UiBuilder::pushCursor() {
	cursorStack.push_back(cursor);
}

void UiBuilder::popCursor() {
	cursor = cursorStack.back();
	cursorStack.pop_back();
}

void UiBuilder::resetCursorInBounds(const SDL_FRect& bounds) {
	switch (style.alignment.h) {
	case HorizontalAlignment::left:
		cursor.x = bounds.x;
		break;
	case HorizontalAlignment::center:
		cursor.x = (bounds.x + bounds.w) / 2.0f;
		break;
	case HorizontalAlignment::right:
		cursor.x = bounds.x + bounds.w;
		break;
	default:
		break;
	}

	switch (style.alignment.v) {
	case VerticalAlignment::top:
		cursor.y = bounds.y;
		break;
	case VerticalAlignment::center:
		cursor.y = (bounds.y + bounds.h) / 2.0f;
		break;
	case VerticalAlignment::bottom:
		cursor.y = bounds.y + bounds.h;
		break;
	default:
		break;
	}
}

void UiBuilder::resetCursor() {
	resetCursorInBounds(sdlutil::deoffsetRect(currentNode().bounds));
}

void UiBuilder::advanceCursor(float w, float h) {
	// For now, only advancing down is supported. Because of this,
	// VerticalAlignment::center is a little tricky. For now, it's only
	// possible by grouping everything together in a node and doing it
	// manually.
	if (style.alignment.v == VerticalAlignment::top) {
		cursor.y += h;
	} else {
		cursor.y -= h;
	}
}

SDL_FPoint UiBuilder::calcRenderLocation(float w, float h) {
	SDL_FPoint pt;

	switch (style.alignment.h) {
	case HorizontalAlignment::left:
		pt.x = cursor.x;
		break;
	case HorizontalAlignment::center:
		// NOTE: It is assumed that the cursor stays in the middle of
		// the node bounds horizontally if halign is center
		pt.x = cursor.x - w/2.0f;
		break;
	case HorizontalAlignment::right:
		pt.x = cursor.x - w;
		break;
	default:
		break;
	}

	switch (style.alignment.v) {
	case VerticalAlignment::top:
		pt.y = cursor.y;
		break;
	case VerticalAlignment::center:
		// NOTE: It is assumed that the cursor stays in the middle of
		// the node bounds vertically if valign is center
		pt.y = cursor.y - h/2.0f;
		break;
	case VerticalAlignment::bottom:
		pt.y = cursor.y - h;
		break;
	default:
		break;
	}

	return pt;
}

void UiBuilder::text(const std::string& str) {
	auto [w, h] = sdlutil::calcTextSize(str, style.textScale);
	SDL_FPoint nodeSize {
		w + 2*style.textPadding,
		h + 2*style.textPadding
	};

	beginNode(nodeSize.x, nodeSize.y);

	if (style.textBackground.a > 0) {
		SDL_FRect bgBounds { 0.0f, 0.0f, nodeSize.x, nodeSize.y };
		emitRectangle(bgBounds, style.textBackground, true);
	}

	SDL_FRect textBounds {
		style.textPadding,
		style.textPadding,
		w,
		h
	};
	emitText(textBounds, style.textColor, style.textScale, str);

	endNode();

	advanceCursor(nodeSize.x, nodeSize.y);
}

void UiBuilder::textBox(const std::string& str, float overrideWidth) {
	auto [w, h] = sdlutil::calcTextSize(str, style.textScale);

	float boxWidth = w;
	if (overrideWidth > 0) {
		boxWidth = overrideWidth;
	}

	SDL_FPoint nodeSize {
		boxWidth + 2*style.textPadding + 2*style.textBoxPadding,
		h + 2*style.textPadding + 2*style.textBoxPadding,
	};

	beginNode(nodeSize.x, nodeSize.y);

	SDL_FRect boxBounds {
		style.textBoxPadding,
		style.textBoxPadding,
		boxWidth + 2*style.textPadding,
		h + 2*style.textPadding
	};
	emitRectangle(boxBounds, style.textBoxBackground, true);
	emitRectangle(boxBounds, style.textBoxBorder, false);

	SDL_FRect textBounds {
		style.textBoxPadding + style.textPadding,
		style.textBoxPadding + style.textPadding,
		w,
		h
	};
	emitText(textBounds, style.textColor, style.textScale, str);

	endNode();

	advanceCursor(nodeSize.x, nodeSize.y);
}

void UiBuilder::beginModal() {
	const auto& b = currentNode().bounds;

	resetCursor();

	// Emit a rectangle that covers the entire containing node
	// must be a node to ensure it gets rendered last. The beginModal()
	// call should be the last call in whatever the enclosing node is.
	beginNode(b.w, b.h);
	emitRectangle(sdlutil::deoffsetRect(b), style.modalOverlayColor, true);
	endNode();

	beginNode(0, 0);
	// Frame rects. Must be corrected once interior of modal is done.
	emitRectangle({ 0, 0, 0, 0 }, style.modalBackgroundColor, true);
	emitRectangle({ 0, 0, 0, 0 }, style.modalBorderColor, false);
}

void UiBuilder::endModal() {
	auto& node = currentNode();

	// Correct size of node and location of children
	auto autofit = node.autoFit();

	node.bounds = sdlutil::deoffsetRect(autofit);
	node.elements[0].bounds = node.bounds;
	node.elements[1].bounds = node.bounds;

	for (auto& child : node.children) {
		child.bounds.x -= autofit.x;
		child.bounds.y -= autofit.y;
	}

	endNode();

	// Move modal node so it's properly in the center of the parent node.
	styleStack.push({
		.type = UiStylePropertyType::Alignment,
		.alignmentValue = {
			HorizontalAlignment::center,
			VerticalAlignment::center
		}
	});
	resetCursor();
	auto loc = calcRenderLocation(node.bounds.w, node.bounds.h);
	node.bounds.x = loc.x;
	node.bounds.y = loc.y;
	styleStack.pop();
}

bool shouldRenderCellRect(SDL_FRect& rect, int w, int h) {
	if (rect.x > w || rect.y > h) return false;
	if (rect.x + rect.w < 0 || rect.y + rect.h < 0) return false;

	return true;
}

life::SPoint sPointFromFPoint(const SDL_FPoint& pt) {
	return {
		static_cast<life::signed_size_t>(std::floor(pt.x)),
		static_cast<life::signed_size_t>(std::floor(pt.y))
	};
}

void WorldWindow::setCellSize(float newCellSize) {
	cellSize = (newCellSize > minCellSize) ? newCellSize : minCellSize;
}

SDL_FPoint WorldWindow::screenPointFromLifePoint(size_t x, size_t y) {
	return { xoffset + x*cellSize, yoffset + y*cellSize };
}

SDL_FPoint WorldWindow::lifePointFromScreenPoint(float x, float y) {
	return {
		(x-xoffset)/cellSize,
		(y-yoffset)/cellSize
	};
}

life::SPoint WorldWindow::lifeSPointFromScreenPoint(float x, float y) {
	auto pt = lifePointFromScreenPoint(x, y);
	return sPointFromFPoint(pt);
}

void WorldWindow::startCellRect(SDL_FRect& rect, int x, int y) {
	auto pt = screenPointFromLifePoint(x, y);
	rect.x = pt.x;
	rect.y = pt.y;
	rect.w = cellSize;
	rect.h = cellSize;
}

void WorldWindow::expandCellRect(SDL_FRect& rect) {
	rect.w += cellSize;
}

SDL_FRect WorldWindow::makeBorderRect(size_t worldWidth, size_t worldHeight) {
	return {
		xoffset,
		yoffset,
		cellSize*worldWidth,
		cellSize*worldHeight
	};
}

void WorldWindow::zoom(float x, float y, float amount) {
	// Position we're zooming in/out at, in life world coordinates.
	auto [gridx, gridy] = lifePointFromScreenPoint(x, y);

	float newCellSize = cellSize + cellSize*amount*zoomSpeed;
	if (newCellSize < minCellSize) newCellSize = minCellSize;
	cellSize = newCellSize;

	auto [newGridX, newGridY] = lifePointFromScreenPoint(x, y);

	float errorx = gridx - newGridX;
	float errory = gridy - newGridY;

	xoffset -= errorx*newCellSize;
	yoffset -= errory*newCellSize;
}

void WorldWindow::renderGrid(SDL_Renderer* renderer) {
	float gridAlpha = 255.0f;
	if (cellSize <= gridFadeStart) {
		gridAlpha *= (cellSize - minCellSize) / (gridFadeStart - minCellSize);
	}

	sdlutil::pushRenderProperty(renderer, {
		.type = sdlutil::RenderPropertyType::DrawBlendMode,
		.blendModeValue = SDL_BLENDMODE_BLEND
	});

	SDL_SetRenderDrawColor(
		renderer,
		50, 50, 50,
		static_cast<uint8_t>(gridAlpha)
	);
	sdlutil::renderGridLines(renderer, xoffset, yoffset, cellSize);

	sdlutil::popRenderProperty(renderer);
}

void WorldWindow::renderBorder(SDL_Renderer* renderer, size_t worldWidth, size_t worldHeight) {
	SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
	auto borderRect = makeBorderRect(worldWidth, worldHeight);
	SDL_RenderRect(renderer, &borderRect);
}
}
