#pragma once

#include <cstdint>
#include <tuple>

#include "../bxdf/polymorphic.hpp"

#include "../colour/colour.hpp"
#include "../mathematics/double3.hpp"
#include "../random/mersenne.hpp"
#include "../ray/intersection.hpp"

namespace BxDF {

	// One sided diffuse emitter, no reflection
	// Handled by Emitter:: class
	class Emission final : public BxDF::Polymorphic {

	private:

		std::uint32_t const id;

	public:

		Emission() = delete;

		Emission(
			std::uint32_t const id
		)
			: id( id ) {};

		std::tuple<Colour, Double3, BxDF::Event, std::float_t, std::float_t> sample(
			Ray::Intersection const& idata,
			BxDF::TraceMode const trace_mode,
			Random::Mersenne&
		) const override {
			return { Colour::Black, Double3::Zero, BxDF::Event::Emission, 0.f, 0.f };
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
			return id;
		};
	};

};
