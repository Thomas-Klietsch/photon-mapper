#pragma once

#include "../mathematics/double3.hpp"
#include "../mathematics/orthogonal.hpp"

namespace Ray {

	// Storage for intersection data
	struct Intersection {
		// Filled in by post intersection in geometry

		// All in world space, unit vectors point away from the point
		Double3 point; // Intersect location
		Double3 from_direction; // unit vector
		Double3 normal_shading; // unit vector
		Double3 normal_geometry; // unit vector
		Orthogonal orthogonal; // Defined from normal_shading
		std::uint32_t material_id;
	};

};
