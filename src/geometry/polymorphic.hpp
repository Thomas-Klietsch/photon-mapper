#pragma once

#include <tuple>

#include "../ray/intersection.hpp"
#include "../ray/section.hpp"

namespace Geometry {

	class Polymorphic {

	public:

		// Returns positive distance, if object is intersected by ray
		virtual std::double_t intersect(
			Ray::Section const& ray
		) const = 0;

		// Fill in intersection data (should only be used on final object)
		virtual Ray::Intersection post_intersect(
			Ray::Section const& ray,
			std::double_t const distance
		) const = 0;

		// Get Axis Aligned Bound Box
		// Double3 [min, max]
		virtual std::tuple<Double3, Double3> get_AABB() const = 0;
	};

};
