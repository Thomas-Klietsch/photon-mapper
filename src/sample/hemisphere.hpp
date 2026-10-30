#pragma once

#include <cmath>

#include "../mathematics/constant.hpp"
#include "../mathematics/double3.hpp"
#include "../random/mersenne.hpp"

namespace Sample {

	// Measuring and Modeling Anisotropic Reflection, Ward et al., 1992
	// Notes on the Ward BRDF, Walter, 2005

	// Cosine weighted sampling, z is up, xy is (tangent) plane
	inline Double3 HemiSphere(
		Random::Mersenne& prng
	) {
		std::float_t const theta = two_pi * prng.get_float();
		std::float_t const z = prng.get_float();
		std::float_t const radius = std::sqrt( 1.f - z );
		return Double3( std::cos( theta ) * radius, std::sin( theta ) * radius, std::sqrt( z ) );
	};

};
