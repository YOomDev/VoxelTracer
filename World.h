#pragma once

#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <vector>
#include "Randomizer.h"

constexpr uint32_t CHUNKS = 1024;
constexpr int32_t CHUNK_WIDTH = 16;
constexpr uint32_t CHUNK_SIZE = CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_WIDTH;
constexpr uint32_t CHUNK_ARRAY_SIZE = CHUNK_SIZE * sizeof(uint32_t);

static uint32_t _BACKUP_ = 0;

struct ChunkLocation {
	int x, y, z;

	ChunkLocation() { x = 0; y = 0; z = 0; }
	ChunkLocation(int X, int Y, int Z) : x(X), y(Y), z(Z) {}
};

struct Chunk {
private:
	uint32_t* voxels = nullptr; // Voxel storage

public:
	ChunkLocation loc; // Chunk position

	Chunk() { loc = { 0,0,0 }; }
	Chunk(int x, int y, int z) : loc(x,y,z) {} // Constructor sets location but does not allocate
	~Chunk() { if (is_used()) { delete[] voxels; voxels = nullptr; } }

	inline const bool is(int X, int Y, int Z) const { return loc.x == X && loc.y == Y && loc.z == Z; }
	inline const bool is_used() const { return voxels == nullptr; }
	void unload() { if (is_used()) { delete[] voxels; voxels = nullptr; } }
	bool allocate(ChunkLocation Loc) {
		// set chunk location while arguments are still hot in memory
		loc = Loc;

		// Attempt allocation of voxel storage
		voxels = (uint32_t*) malloc(CHUNK_ARRAY_SIZE);
		if (voxels == nullptr) { return false; }
		memset(voxels, 0, CHUNK_ARRAY_SIZE);

		// TODO: generate chunk voxels

		return true;
	}

	const uint32_t operator[](uint32_t i) const { 
		if (is_used() && i < CHUNK_SIZE) { return voxels[i]; }
		// Warn user that they tried to read data from a non-allocated chunk instead of panicking
		printf_s("WARNING: Tried reading data from a non-allocated chunk!\n");
		return 0;
	};

	uint32_t& operator[](uint32_t i) {
		if (is_used() && i < CHUNK_SIZE) { return voxels[i]; }
		// Warn user that they tried to read data from a non-allocated chunk instead of panicking
		printf_s("WARNING: Tried reading a reference to data from a non-allocated chunk!\n");
		return _BACKUP_;
	};
};

struct World {
private:
	Chunk chunks[CHUNKS] { Chunk(0, 0, 0) };
	std::vector<ChunkLocation> toAllocate = {};

	int findFirstEmpty() {
		for (int i = 0; i < CHUNKS; i++) { if (!chunks[i].is_used()) { return i; } }
		return -1;
	}

public:
	World() {}
	~World() {}

	void loadChunks() {
		int empty = findFirstEmpty();
		while (empty > -1) {
			if (toAllocate.size() == 0) { break; }
			if (!chunks[empty].allocate(toAllocate[toAllocate.size()-1])) { break; }
			empty = findFirstEmpty();
		}
	}

	uint32_t& get_voxel(long x, long y, long z) {
		// get chunk location and voxel coords in chunk
		int cx = 0, cy = 0, cz = 0;
		while (x < -CHUNK_WIDTH) { x += CHUNK_WIDTH; cx--; }
		while (x >= CHUNK_WIDTH) { x -= CHUNK_WIDTH; cx++; }
		while (y < -CHUNK_WIDTH) { y += CHUNK_WIDTH; cy--; }
		while (y >= CHUNK_WIDTH) { y -= CHUNK_WIDTH; cy++; }
		while (z < -CHUNK_WIDTH) { z += CHUNK_WIDTH; cz--; }
		while (z >= CHUNK_WIDTH) { z -= CHUNK_WIDTH; cz++; }
		const uint32_t index = z * CHUNK_WIDTH * CHUNK_WIDTH + y * CHUNK_WIDTH + x;

		// check if a chunk is allocated at this chunk coord
		int found = -1;
		int count = 0;
		for (int i = 0; i < CHUNKS; i++) {
			if (chunks[i].is_used()) {
				count++;
				if (chunks[i].is(cx, cy, cz)) {
					return chunks[i][index]; // return voxel if found
				}
			}
		}

		// return default voxel
		printf_s("WARNING: Unable to find correct chunk, %d are currently allocated and in use", count);
		toAllocate.push_back({cx, cy, cz});
		return _BACKUP_;
	}
};