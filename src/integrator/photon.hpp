#pragma once

#include "../colour/colour.hpp"
#include "../mathematics/double3.hpp"

namespace Integrator {

	// Flux, emitted from a light source (emitter)
	struct Photon {
		// Location in world space
		Double3 position;
		// Direction towards incoming ray direction
		Double3 in_ray_dir;
		// Energy/power of the particle
		Colour energy;
	};

};
