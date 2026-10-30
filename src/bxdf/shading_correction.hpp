#pragma once

#include "../bxdf/polymorphic.hpp"
#include "../mathematics/double3.hpp"
#include "../ray/intersection.hpp"

namespace BxDF {

	// Veach 150
	inline std::float_t ShadingCorrection(
		Double3 const& evaluate_direction, // Veach 85, wo
		Double3 const& from_direction, // Veach 85, wi
		Ray::Intersection const& idata,
		BxDF::TraceMode const trace_mode
	) {
		if ( trace_mode == BxDF::TraceMode::Importance ) {
			std::double_t const numerator = std::abs( evaluate_direction.dot( idata.normal_shading ) * from_direction.dot( idata.normal_geometry ) );
			std::double_t const denominator = std::abs( evaluate_direction.dot( idata.normal_geometry ) * from_direction.dot( idata.normal_shading ) );
			if ( denominator < EPSILON_BLACK )
				return 0.f;
			return numerator / denominator;
		}
		return 1.f;
	};

};
