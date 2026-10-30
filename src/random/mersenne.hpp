#pragma once

#include <cmath>
#include <cstdint>

namespace Random {

	// Pseudo random number generator using Mersenne twister
	class Mersenne final {

	private:

		// Random number from a roll with a d20
		std::uint32_t seed{ 11 };

	public:

		Mersenne() {};

		Mersenne(
			std::uint32_t const seed
		)
			: seed( seed ) {};

		// Uniform PRNG in [0;1]
		std::float_t get_float() { return next() / 4294967295.f; };

		// Uniform PRNG for unsigned 32bit
		std::uint32_t get_integer() { return next(); };

	private:

		// Note: Inline syntax differs between compilers
		__attribute__(( always_inline )) inline std::uint32_t next() {
			std::uint32_t x = seed = ( 1812433253U * ( seed ^ ( seed >> 30 ) ) + 1 ) & 0xFFFFFFFFU;
			x ^= ( x >> 11 );
			x ^= ( x << 7 ) & 0x9D2C5680U;
			x ^= ( x << 15 ) & 0xEFC60000U;
			return x ^= ( x >> 18 );
		};
	};

};
