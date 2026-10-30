#pragma once

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
	// One sided material, delta dirac reflector
	class Mirror final : public BxDF::Polymorphic {

	private:

		Colour const reflectance;

	public:

		Mirror() = delete;

		Mirror( Colour const& reflectance )
			: reflectance( reflectance ) {};

		std::tuple<Colour, Double3, BxDF::Event, std::float_t, std::float_t> sample(
			Ray::Intersection const& idata,
			BxDF::TraceMode const trace_mode,
			Random::Mersenne&
		) const override {
			std::double_t const cos_theta = idata.from_direction.dot( idata.normal_shading );
			if ( cos_theta < EPSILON_COS_THETA )
				return { Colour::Black, Double3::Zero, BxDF::Event::None, 0.f, 0.f };
			Double3 const evaluate_direction = -idata.from_direction + idata.normal_shading * ( 2. * cos_theta );
			return { reflectance, evaluate_direction, BxDF::Event::Reflect, 1.f, evaluate_direction.dot( idata.normal_shading ) };
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
