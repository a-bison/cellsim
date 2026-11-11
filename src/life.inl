#include <algorithm>
#include <cstdlib>
#include <iostream>

namespace life {
template<size_t W, size_t H>
Buffer<W, H>::Buffer() {
	clear();
}

template<size_t W, size_t H>
size_t Buffer<W, H>::__ix(signed_size_t x, signed_size_t y) {
	return W*y+x;
}

template<size_t W, size_t H>
bool Buffer<W, H>::inBounds(signed_size_t x, signed_size_t y) {
	return x >=0 && x < W && y >=0 && y < H;
}

template<size_t W, size_t H>
void Buffer<W, H>::clear() {
	std::fill(__data.begin(), __data.end(), false);
}

template<size_t W, size_t H>
bool Buffer<W, H>::get(signed_size_t x, signed_size_t y) const {
	if (!inBounds(x, y)) return false;
	return __data[__ix(x, y)];
}

template<size_t W, size_t H>
bool Buffer<W, H>::get(Point pt) const { return get(pt.x, pt.y); }

template<size_t W, size_t H>
void Buffer<W, H>::set(signed_size_t x, signed_size_t y, bool val) {
	if (!inBounds(x, y)) return;
	__data[__ix(x, y)] = val;
}

template<size_t W, size_t H>
void Buffer<W, H>::set(Point pt, bool val) { set(pt.x, pt.y, val); }

template<size_t W, size_t H>
unsigned int Buffer<W, H>::countLiveNeighbors(signed_size_t x, signed_size_t y) const {
	unsigned int count = 0;

	for (signed_size_t yOfs = -1; yOfs <= 1; yOfs++) {
		for (signed_size_t xOfs = -1; xOfs <= 1; xOfs++) {
			if (yOfs == 0 && xOfs == 0) continue;

			if (get(x+xOfs, y+yOfs)) count++;
		}
	}

	return count;
}

template<size_t W, size_t H>
unsigned int Buffer<W, H>::countLiveNeighbors(Point pt) const {
	return countLiveNeighbors(pt.x, pt.y);
}

template<size_t W, size_t H>
void Buffer<W, H>::massSet(signed_size_t x, signed_size_t y, std::span<const Point> offsets, bool val) {
	for (auto& offset : offsets) {
		set(x+offset.x, y+offset.y, val);
	}
}

template<size_t W, size_t H>
void Buffer<W, H>::randomize() {
	for (auto& cell : __data) {
		cell = static_cast<bool>(std::rand() % 2);
	}
}

template<size_t W, size_t H>
void Buffer<W, H>::dumpGrid() const {
	for (size_t y = 0; y < H; y++) {
		for (size_t x = 0; x < W; x++) {
			std::cout << get(x, y) ? '1' : '0';
		}

		std::cout << std::endl;
	}
}

template<size_t W, size_t H>
const Buffer<W, H>& World<W, H>::frontBuffer() const {
	return __buffers[__bufferIndex];
}

template<size_t W, size_t H>
Buffer<W, H>& World<W, H>::frontBufferWritable() {
	return __buffers[__bufferIndex];
}

template<size_t W, size_t H>
Buffer<W, H>& World<W, H>::backBuffer() {
	return __buffers[1-__bufferIndex];
}

template<size_t W, size_t H>
void World<W, H>::swap() {
	__bufferIndex = (__bufferIndex == 1) ? 0 : 1;
}

template<size_t W, size_t H>
void World<W, H>::tick() {
	auto& front = frontBuffer();
	auto& back = backBuffer();

	for (size_t y = 0; y < H; y++) {
		for (size_t x = 0; x < W; x++) {
			bool alive = front.get(x, y);
			auto count = front.countLiveNeighbors(x, y);
			bool nextState = lifeRule(alive, count);
			back.set(x, y, nextState);
		}
	}
}
}
