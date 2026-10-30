#pragma once

#include "../mathematics/double3.hpp"

namespace Ray {

	// Unlike a (infinite) line, a ray has a start point (origin),
	// and the direction defines the part of the line it is on.
	// Pedantic, but semantic it fits with intersection. Maybe? >_<
	class Section final {

	public:

		Double3 origin;
		Double3 direction; // Unit vector

		Section()
			: origin( Double3::Zero )
			, direction( Double3::Zero ) {};

		Section(
			Double3 const& origin,
			Double3 const& direction // Unit vector is expected
		)
			: origin( origin )
			, direction( direction ) {};

		Section(
			Double3 const& origin,
			Double3 const& direction, // Unit vector is expected
			std::double_t const epsilon
		)
			: origin( origin + direction * epsilon )
			, direction( direction ) {};
	};

};
