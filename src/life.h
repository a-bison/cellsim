#pragma once

#include <array>
#include <span>

namespace life {
using signed_size_t = std::make_signed_t<size_t>;

struct Point {
	size_t x;
	size_t y;
};

struct SPoint {
	signed_size_t x;
	signed_size_t y;
};

template<size_t W, size_t H>
struct Buffer {
	std::array<bool, W*H> __data;

	Buffer();

	static size_t __ix(signed_size_t x, signed_size_t y);
	static bool inBounds(signed_size_t x, signed_size_t y);

	bool get(signed_size_t x, signed_size_t y) const;
	bool get(Point pt) const;

	void set(signed_size_t x, signed_size_t y, bool val);
	void set(Point pt, bool val);

	unsigned int countLiveNeighbors(signed_size_t x, signed_size_t y) const;
	unsigned int countLiveNeighbors(Point pt) const;

	void massSet(signed_size_t x, signed_size_t y, std::span<const Point> offsets, bool val);

	void randomize();

	// Prints the grid to the console. Used for debug.
	void dumpGrid() const;
};

// Given previous cell state and number of live neighbors, returns whether the
// cell should be alive or dead next tick
bool lifeRule(bool alive, unsigned int liveNeighbors);

template<size_t W, size_t H>
struct World {
	std::array<Buffer<W, H>, 2> __buffers;
	size_t __bufferIndex = 0;

	// The buffer that is ready to display.
	const Buffer<W, H>& frontBuffer() const;
	Buffer<W, H>& frontBufferWritable();

	// The buffer that is being written to for a new tick.
	Buffer<W, H>& backBuffer();

	// Uses the frontbuffer to fill the backbuffer with the next
	// generation.
	void tick();

	// Swaps the backbuffer and frontbuffer.
	void swap();
};
}

#include "life.inl"
