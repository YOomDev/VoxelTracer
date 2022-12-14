#pragma once

#include <cstdint>

struct Random {
private:
	uint64_t state0;
	uint64_t state1;

public:
	Random(uint64_t state0, uint64_t state1) : state0(state0), state1(state1) {}

	uint32_t next() {
		uint64_t x = state0;
		uint64_t y = state1;
		uint64_t r = x + y;
		state0 = y;
		x ^= x << 23;
		state1 = x ^ y ^ (x >> 18) ^ (y >> 5);
		return static_cast<uint32_t>(r);
	}
};

auto random = Random(0xA6E9377DAF75BDFEULL, 0x863F5CB508510D95ULL);

inline float randDouble() { return float(random.next() / 4294967295.0f); }