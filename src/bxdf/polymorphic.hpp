#pragma once

#include <cstdint>
#include <tuple>

#include "../colour/colour.hpp"
#include "../mathematics/double3.hpp"
#include "../random/mersenne.hpp"
#include "../ray/intersection.hpp"

namespace BxDF {

	// Sample direction of BxDF (Veach p93)
	enum class TraceMode : std::uint8_t
	{
		// Sample direction (wi)
		// Light path, wo is direction to emitter (= -ray.direction)
		Importance,
		// Camera path (light flow), wo is direction to camera (= -ray.direction)
		Radiance
	};

	enum class Event : std::uint8_t
	{
		None,
		Diffuse,
		Emission,
		Reflect,
		Transmit
	};

	class Polymorphic {

	public:

		// BxDF factor, sample dir, event, pdf_W, cos_theta
		virtual std::tuple<Colour, Double3, BxDF::Event, std::float_t, std::float_t> sample(
			Ray::Intersection const& idata,
			BxDF::TraceMode const trace_mode,
			Random::Mersenne& prng
		) const = 0;

		// BxDF factor, pdf_W, cos_theta
		virtual std::tuple<Colour, std::float_t, std::float_t> evaluate(
			Double3 const& evaluate_direction,
			Double3 const& from_direction,
			Ray::Intersection const& idata,
			BxDF::TraceMode const trace_mode
		) const = 0;

		// True if material has, at least one, diffuse lobe/layer.
		// Used for photon mapping.
		virtual bool f_diffuse() const = 0;

		// Used for emission materials,
		// returns UINT32_MAX if not an emitter
		virtual std::uint32_t emitter_id() const = 0;
	};

};
