#pragma once

#include <cmath>
#include <cstdint>
#include <tuple>

#include "../bxdf/polymorphic.hpp"

#include "../colour/colour.hpp"
#include "../epsilon.hpp"
#include "../mathematics/double3.hpp"
#include "../random/mersenne.hpp"
#include "../ray/intersection.hpp"

namespace BxDF {

	// Not physical based
	// Double sided material, delta dirac transmission
	class Dielectric final : public BxDF::Polymorphic {

	private:

		Colour const transmittance;

		// Relative eta
		std::double_t const eta_enter;
		std::double_t const eta_exit;

	public:

		Dielectric() = delete;

		Dielectric(
			// Eta of the surrounding media (1 for vacuum)
			std::float_t const eta_outer,
			// Eta of the material's media (1.33 for water, 1.5 for glass)
			std::float_t const eta_inner,
			Colour const& transmittance
		)
			: eta_enter( std::max( 1.f, eta_outer ) / std::max( 1.f, eta_inner ) )
			, eta_exit( std::max( 1.f, eta_inner ) / std::max( 1.f, eta_outer ) )
			, transmittance( transmittance ) {
				// TODO print warning if eta_outer >= eta inner ?
			};

		std::tuple<Colour, Double3, BxDF::Event, std::float_t, std::float_t> sample(
			Ray::Intersection const& idata,
			BxDF::TraceMode const trace_mode,
			Random::Mersenne&
		) const override {
			std::double_t const cos_theta = idata.from_direction.dot( idata.normal_shading );
			if ( std::abs( cos_theta ) < EPSILON_COS_THETA )
				return { Colour::Black, Double3::Zero, BxDF::Event::None, 0.f, 0.f };

			// Entering inner media (object)?
			bool const entering = cos_theta >= 0.;

			std::double_t const eta = entering ? eta_enter : eta_exit;

			// Snell's law, used to find refracted cos theta
			std::double_t const sin_theta_squared = std::max( 0., 1. - cos_theta * cos_theta );
			std::double_t const refract_sin_theta_squared = eta * eta * sin_theta_squared;
			if ( refract_sin_theta_squared >= 1. ) {
				// Total Internal Reflection (discarded)
				return { Colour::Black, Double3::Zero, BxDF::Event::None, 0.f, 0.f };
				// Double3 const reflected_direction = -idata.from_direction + idata.normal_shading * ( 2. * cos_theta );
				// return { Colour::White, reflected_direction, BxDF::Event::Reflect, 1.f, std::abs( reflected_direction.dot( idata.normal_shading ) ) };
			}
			std::double_t const refract_cos_theta = std::sqrt( std::max( 0., 1. - refract_sin_theta_squared ) );

			Double3 const refracted_direction = idata.normal_shading * ( eta * std::abs( cos_theta ) - refract_cos_theta ) * ( entering ? 1.f : -1.f ) - idata.from_direction * eta;

			Colour result = entering ? transmittance : Colour::White;

			// When tracing camera path, non-symmetry of mediums needs to be handled
			if ( trace_mode == BxDF::TraceMode::Radiance )
				result = result * ( eta * eta );

			std::double_t const cos_theta_sample = std::abs( refracted_direction.dot( idata.normal_shading ) );
			return { result, refracted_direction, BxDF::Event::Transmit, 1.f, cos_theta_sample };
		};

		std::tuple<Colour, std::float_t, std::float_t> evaluate(
			Double3 const&,
			Double3 const&,
			Ray::Intersection const&,
			BxDF::TraceMode const
		) const override {
			return { Colour::Black, 0.f, 0.f };
		};

		bool f_diffuse() const override { return false; };

		std::uint32_t emitter_id() const override {
			return UINT32_MAX;
		};
	};

};
