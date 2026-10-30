#pragma once

#include <cstdint>

#include "../geometry/polymorphic.hpp"

#include "../mathematics/double3.hpp"
#include "../mathematics/orthogonal.hpp"
#include "../ray/intersection.hpp"
#include "../ray/section.hpp"

namespace Geometry {

	class Triangle final : public Geometry::Polymorphic {

	private:

		Double3 position; // a
		Double3 edge1; // b-a
		Double3 edge2; // c-a
		Double3 normal; // edge1 cross edge2

		Orthogonal orthogonal;

		std::uint32_t material_id;

	public:

		Triangle() = delete;

		Triangle(
			Double3 const& a,
			Double3 const& b,
			Double3 const& c,
			std::uint32_t const material_id
		)
			: position( a )
			, edge1( b - a )
			, edge2( c - a )
			, material_id( material_id ) {
			normal = ( edge1.cross( edge2 ) ).normalise();
			orthogonal = Orthogonal( normal );
		};

		std::double_t intersect(
			Ray::Section const& ray
		) const override {
			// M�ller-Trumbore intersection algorithm
			// Fast, minimum storage ray/triangle intersection, 1997

			// The return of different negative numbers is simply for debuging

			// Calculating determinant
			Double3 const p = ray.direction.cross( edge2 );
			std::double_t const d = edge1.dot( p );

			// If determinant is near zero, ray lies in plane of triangle
			if ( std::abs( d ) < 0.000001 ) // TODO
				return -1.0;

			std::double_t const inv_d = 1.0 / d;

			Double3 const diff = ray.origin - position;

			// Calculate u parameter and test bound
			std::double_t const u = diff.dot( p ) * inv_d;
			if ( ( u < 0. ) || ( u > 1. ) )
				return -2.0;

			// Calculate v parameter and test bound
			Double3 const q = diff.cross( edge1 );
			std::double_t const v = ray.direction.dot( q ) * inv_d;
			if ( ( v < 0. ) || ( u + v > 1. ) )
				return -3.0;

			std::double_t const t = q.dot( edge2 ) * inv_d;

			if ( t < 0.000001 ) // TODO
				return -4.0;

			return t;
		};

		Ray::Intersection post_intersect(
			Ray::Section const& ray,
			std::double_t const distance
		) const override {
			Ray::Intersection idata;
			idata.point = ray.origin + ray.direction * distance;
			idata.orthogonal = orthogonal;
			idata.material_id = material_id;
			idata.from_direction = -ray.direction;
			idata.normal_shading = normal;
			idata.normal_geometry = normal;

			return idata;
		};

		std::tuple<Double3, Double3> get_AABB() const override {
			Double3 aabb_min{
				std::min( { position.x, position.x + edge1.x, position.x + edge2.x } ),
				std::min( { position.y, position.y + edge1.y, position.y + edge2.y } ),
				std::min( { position.z, position.z + edge1.z, position.z + edge2.z } ) };
			Double3 aabb_max{
				std::max( { position.x, position.x + edge1.x, position.x + edge2.x } ),
				std::max( { position.y, position.y + edge1.y, position.y + edge2.y } ),
				std::max( { position.z, position.z + edge1.z, position.z + edge2.z } ) };
			return { aabb_min, aabb_max };
		};
	};

};
