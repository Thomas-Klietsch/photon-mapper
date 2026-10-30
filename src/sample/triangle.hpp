#pragma once

#include <cmath>
#include <tuple>

#include "../random/mersenne.hpp"

namespace Sample {

	// Low discrepancy constructions in the triangle, Basu and Owen, 2014
	// Shape Distributions, Osada et al.
	// https://math.stackexchange.com/questions/18686/uniform-random-point-in-triangle-in-3d

	// Uniform sampling, returns scalar for triangle edges
	inline std::tuple<std::float_t, std::float_t> Triangle(
		Random::Mersenne& prng
	) {
		// Inverse cumulative distribution technique
		std::float_t const e1 = std::sqrt( prng.get_float() );
		std::float_t const e2 = prng.get_float();
		return { e1 * e2, e1 * ( 1.f - e2 ) };
	};

};
