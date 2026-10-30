#pragma once

#include <cstdint>

#include "../geometry/polymorphic.hpp"

// TODO #include "../epsilon.hpp"
#include "../mathematics/double3.hpp"
#include "../mathematics/orthogonal.hpp"
#include "../ray/intersection.hpp"
#include "../ray/section.hpp"

namespace Geometry {

	class Sphere final : public Geometry::Polymorphic {

	private:

		Double3 const position;
		std::double_t const radius2; // Radius squared

		std::uint32_t material_id;

	public:

		Sphere() = delete;

		Sphere(
			Double3 const& position,
			std::double_t const radius,
			std::uint32_t const material_id
		)
			: position( position )
			, radius2( radius * radius )
			, material_id( material_id ) {
			};

		std::double_t intersect(
			Ray::Section const& ray
		) const override {
			// Vector, direction to move ray.origin to sphere centre,
			// hypotenuse in the Pythagorean triangle
			Double3 const offset = position - ray.origin;
			// Projection of offset onto ray.direction,
			// the point is where the sphere location is at an 90 angle to the ray direction
			std::double_t const projection = offset.dot( ray.direction );
			// Distance (squared) from sphere centre to projected point, using Pythagorean triangle
			std::double_t const distance2 = offset.dot( offset ) - projection * projection;
			if ( distance2 > radius2 )
				return -1.;
			// The distance difference between the two ray/sphere intersections
			std::double_t const t_delta = std::sqrt( radius2 - distance2 );
			std::double_t t_near = projection - t_delta;
			std::double_t t_far = projection + t_delta;

			if ( t_near > t_far )
				std::swap( t_near, t_far );

			// If negative, it is behind the ray origin
			if ( t_near < 0 ) {
				// Use t1
				t_near = t_far;
				// If negative, it is behind the ray origin
				if ( t_near < 0 )
					return -1.;
			}

			return t_near;
		};

		Ray::Intersection post_intersect(
			Ray::Section const& ray,
			std::double_t const distance
		) const override {
			Ray::Intersection idata;
			idata.point = ray.origin + ray.direction * distance;
			// TODO test for radius?
			idata.normal_geometry = ( idata.point - position ).normalise();
			idata.normal_shading = idata.normal_geometry;
			idata.orthogonal = Orthogonal( idata.normal_geometry );
			idata.material_id = material_id;
			idata.from_direction = -ray.direction;

			return idata;
		};

		std::tuple<Double3, Double3> get_AABB() const override {
			Double3 aabb_min{ position - Double3::One * std::sqrt( radius2 ) };
			Double3 aabb_max{ position + Double3::One * std::sqrt( radius2 ) };
			return { aabb_min, aabb_max };
		};
	};

};
