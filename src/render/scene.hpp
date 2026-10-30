#pragma once

#include <cstdint>
#include <memory>
#include <tuple>
#include <vector>

#include "../bxdf/dielectric.hpp"
#include "../bxdf/emission.hpp"
#include "../bxdf/lambert.hpp"
#include "../bxdf/mirror.hpp"
#include "../bxdf/polymorphic.hpp"
#include "../colour/colour.hpp"
#include "../emitter/polymorphic.hpp"
#include "../emitter/triangle.hpp"
#include "../geometry/polymorphic.hpp"
#include "../geometry/sphere.hpp"
#include "../geometry/triangle.hpp"
#include "../mathematics/double3.hpp"
#include "../random/mersenne.hpp"
#include "../ray/intersection.hpp"
#include "../ray/section.hpp"

namespace Render {

	class Scene final {

	private:

		std::vector<std::shared_ptr<Geometry::Polymorphic>> geometry;
		std::uint32_t n_geometry{ 0 };

		std::vector<std::shared_ptr<Emitter::Polymorphic>> emitter_list;
		std::uint32_t n_emitter{ 0 };

		std::vector<std::shared_ptr<BxDF::Polymorphic>> bxdf;
		std::uint32_t n_bxdf{ 0 };

		// AABB information
		Double3 scene_centre{ Double3::Zero };
		std::double_t scene_radius{ 0. };

	public:

		Scene() {
			Cornell_Box(
				false, // true=diffuse tall box, else mirror
				true // ceiling light triangles; true = two (2) , else four (4)
			);
		};

		// Find closest intersectable object given a ray
		std::tuple<bool, std::double_t, Ray::Intersection> intersect(
			Ray::Section const& ray
		) const {
			std::double_t distance = 1e42;
			std::uint32_t object_id = UINT32_MAX;
			bool f_hit{ false };
			for ( std::uint32_t i{ 0 }; i < n_geometry; ++i ) {
				std::double_t d = geometry[i]->intersect( ray );
				if ( d > 0.0 && d < distance ) {
					// TODO
					distance = d;
					object_id = i;
					f_hit = true;
				}
			}

			if ( !f_hit )
				return { false, {}, {} };

			return { true, distance, geometry[object_id]->post_intersect( ray, distance ) };
		};

		// Return true if there are any objects within ]0;distance[
		bool occluded(
			Ray::Section const& ray,
			std::double_t const& distance
		) const {
			for ( std::uint32_t i{ 0 }; i < n_geometry; ++i ) {
				std::double_t d = geometry[i]->intersect( ray );
				if ( d > 0.0 && d < distance )
					return true;
			}
			return false;
		};

		// Returns a (smart pointer) reference to material
		std::shared_ptr<BxDF::Polymorphic> const& material(
			std::uint32_t const id
		) const {
			if ( id >= n_bxdf )
				throw std::overflow_error( "Material ID: " + std::to_string( id ) + " , is out of bounds!\n" );
			return bxdf[id];
		};

		inline std::float_t emitter_select_probability(
			std::uint32_t const id
		) const {
			if ( id >= n_emitter )
				throw std::overflow_error( "Emitter ID: " + std::to_string( id ) + " , is out of bounds!\n" );
			// All emitters are sampled equally
			std::float_t static const pdf = 1.f / static_cast<std::float_t>( n_emitter );
			return pdf;
		}

		// Returns a (smart pointer) reference to emitter, and emitter select probability
		std::tuple<std::shared_ptr<Emitter::Polymorphic> const&, std::float_t> emitter(
			std::uint32_t const id
		) const {
			// Sampling of all emitters is equal
			if ( id >= n_emitter )
				throw std::overflow_error( "Emitter ID: " + std::to_string( id ) + " , is out of bounds!\n" );
			return { emitter_list[id], emitter_select_probability( id ) };
		};

		// Emitter ID, picked at random
		std::uint32_t random_emitter(
			Random::Mersenne& prng
		) const {
			// Sampling of all emitters is equal
			return prng.get_integer() % n_emitter;
		};

		// Returns true if the scene can be rendered
		bool is_valid() const { return ( n_geometry > 0 ) & ( n_emitter > 0 ) & ( n_bxdf > 0 ); };

		// Centre of scene AABB
		Double3 get_scene_centre() const { return scene_centre; };

		// Radius of scene AABB
		std::double_t get_scene_radius() const { return scene_radius; };

	private:

		void Cornell_Box(
			// True for diffuse tall block, else a mirror
			bool const f_diffuse_box,
			// Test that emitter calculations are correct (e.g. same result for true and false)
			// With no BVH, render time for four (4) emitters is increased
			bool const f_simple_emitter
		) {
			// The Cornell Box
			// https://www.graphics.cornell.edu/online/box/

			// Note that the order, and sign, of the 3D data is altered here,
			// since the render uses z axis as world up.

			Colour const energy = ( Colour( 0.f, .929f, .659f ) * 8.f + Colour( 1.f, .447f, .0f ) * 15.6f + Colour( 0.376f, 0.f, 0.f ) * 18.4f );
			// std::cout << "Energy " << energy << '\n';
			// 22.5184 , 14.4052 , 5.272

			bxdf.emplace_back( std::make_shared<BxDF::Lambert>( Colour( .8f, .8f, .8f ) ) ); // White = 0
			bxdf.emplace_back( std::make_shared<BxDF::Lambert>( Colour( 0.6f, 0.01f, 0.01f ) ) ); // Red = 1
			bxdf.emplace_back( std::make_shared<BxDF::Lambert>( Colour( 0.01f, 0.25f, 0.01f ) ) ); // Green = 2

			bxdf.emplace_back( std::make_shared<BxDF::Mirror>( Colour::White ) ); // Mirror = 3

			// Emitter with ID
			bxdf.emplace_back( std::make_shared<BxDF::Emission>( 0 ) ); // 4
			bxdf.emplace_back( std::make_shared<BxDF::Emission>( 1 ) ); // 5
			bxdf.emplace_back( std::make_shared<BxDF::Emission>( 2 ) ); // 6
			bxdf.emplace_back( std::make_shared<BxDF::Emission>( 3 ) ); // 7

			bxdf.emplace_back( std::make_shared<BxDF::Dielectric>( 1.f, 1.5f, Colour::White ) ); // Glass = 8

			std::uint32_t const tall_block_material{ ( f_diffuse_box ? 0u : 3u ) }; // true for diffuse, false for mirror

			// Scene box
			Double3 const scene_box[8] = {
				Double3( 0.0, 0.0, 0.0 ),
				Double3( 0.0, 0.0, 548.8 ),
				Double3( 0.0, 559.2, 0.0 ),
				Double3( 0.0, 559.2, 548.8 ),
				Double3( -552.8, 0.0, 0.0 ),
				Double3( -556.0, 0.0, 548.8 ),
				Double3( -549.6, 559.2, 0.0 ),
				Double3( -556.0, 559.2, 548.8 ),
			};
			// Back
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( scene_box[2], scene_box[3], scene_box[7], 0 ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( scene_box[2], scene_box[7], scene_box[6], 0 ) );
			// Top
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( scene_box[1], scene_box[5], scene_box[7], 0 ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( scene_box[1], scene_box[7], scene_box[3], 0 ) );
			// Bottom
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( scene_box[0], scene_box[2], scene_box[6], 0 ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( scene_box[0], scene_box[6], scene_box[4], 0 ) );
			// Left
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( scene_box[4], scene_box[6], scene_box[7], 1 ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( scene_box[4], scene_box[7], scene_box[5], 1 ) );
			// Right
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( scene_box[0], scene_box[1], scene_box[3], 2 ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( scene_box[0], scene_box[3], scene_box[2], 2 ) );

			// Short block
			Double3 const short_block[8] = {
				Double3( -82.0, 225.0, 0.0 ),
				Double3( -82.0, 225.0, 165.0 ),
				Double3( -130.0, 65.0, 0.0 ),
				Double3( -130.0, 65.0, 165.0 ),
				Double3( -240.0, 272.0, 0.0 ),
				Double3( -240.0, 272.0, 165.0 ),
				Double3( -290.0, 114.0, 0.0 ),
				Double3( -290.0, 114.0, 165.0 ) };
			// Back
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( short_block[4], short_block[5], short_block[1], 0 ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( short_block[4], short_block[1], short_block[0], 0 ) );
			// Front
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( short_block[2], short_block[3], short_block[7], 0 ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( short_block[2], short_block[7], short_block[6], 0 ) );
			// Top
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( short_block[3], short_block[1], short_block[5], 0 ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( short_block[3], short_block[5], short_block[7], 0 ) );
			// Left
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( short_block[6], short_block[7], short_block[5], 0 ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( short_block[6], short_block[5], short_block[4], 0 ) );
			// Right
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( short_block[0], short_block[1], short_block[3], 0 ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( short_block[0], short_block[3], short_block[2], 0 ) );

			// Tall block
			Double3 const tall_block[8] = {
				Double3( -265.0, 296.0, 0.0 ),
				Double3( -265.0, 296.0, 330.0 ),
				Double3( -314.0, 456.0, 0.0 ),
				Double3( -314.0, 456.0, 330.0 ),
				Double3( -423.0, 247.0, 0.0 ),
				Double3( -423.0, 247.0, 330.0 ),
				Double3( -472.0, 406.0, 0.0 ),
				Double3( -472.0, 406.0, 330.0 ) };
			// Back
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( tall_block[6], tall_block[7], tall_block[3], tall_block_material ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( tall_block[6], tall_block[3], tall_block[2], tall_block_material ) );
			// Front
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( tall_block[0], tall_block[1], tall_block[5], tall_block_material ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( tall_block[0], tall_block[5], tall_block[4], tall_block_material ) );
			// Top
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( tall_block[5], tall_block[1], tall_block[3], tall_block_material ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( tall_block[5], tall_block[3], tall_block[7], tall_block_material ) );
			// Left
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( tall_block[4], tall_block[5], tall_block[7], tall_block_material ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( tall_block[4], tall_block[7], tall_block[6], tall_block_material ) );
			// Right
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( tall_block[2], tall_block[3], tall_block[1], tall_block_material ) );
			geometry.emplace_back( std::make_shared<Geometry::Triangle>( tall_block[2], tall_block[1], tall_block[0], tall_block_material ) );

			// Offset to avoid "z fighting"
			Double3 const light[5] = {
				Double3( -213.0, 227.0, 548.8 - 0.01 ),
				Double3( -213.0, 332.0, 548.8 - 0.01 ),
				Double3( -343.0, 227.0, 548.8 - 0.01 ),
				Double3( -343.0, 332.0, 548.8 - 0.01 ),
				// If using four (4) triangles for the ceiling emitter
				Double3(
					( -213.0 + -213.0 + -343.0 + -343.0 ) * 0.25,
					( 227.0 + 332.0 + 227.0 + 332.0 ) * 0.25,
					548.8 - 0.01
				),

			};

			if ( f_simple_emitter ) {
				// Two (2) triangles as ceiling emitter
				// Visible emitters
				geometry.emplace_back( std::make_shared<Geometry::Triangle>( light[2], light[3], light[1], 4 ) );
				geometry.emplace_back( std::make_shared<Geometry::Triangle>( light[2], light[1], light[0], 5 ) );
				// Emitters
				emitter_list.emplace_back( std::make_shared<Emitter::Triangle>( light[2], light[3], light[1], energy ) );
				emitter_list.emplace_back( std::make_shared<Emitter::Triangle>( light[2], light[1], light[0], energy ) );
			}
			else {
				// Four (4) triangles as ceiling emitter
				// Visible emitters
				geometry.emplace_back( std::make_shared<Geometry::Triangle>( light[1], light[0], light[4], 4 ) );
				geometry.emplace_back( std::make_shared<Geometry::Triangle>( light[0], light[2], light[4], 5 ) );
				geometry.emplace_back( std::make_shared<Geometry::Triangle>( light[2], light[3], light[4], 6 ) );
				geometry.emplace_back( std::make_shared<Geometry::Triangle>( light[3], light[1], light[4], 7 ) );
				// Emitters
				emitter_list.emplace_back( std::make_shared<Emitter::Triangle>( light[1], light[0], light[4], energy ) );
				emitter_list.emplace_back( std::make_shared<Emitter::Triangle>( light[0], light[2], light[4], energy ) );
				emitter_list.emplace_back( std::make_shared<Emitter::Triangle>( light[2], light[3], light[4], energy ) );
				emitter_list.emplace_back( std::make_shared<Emitter::Triangle>( light[3], light[1], light[4], energy ) );
			}

			// Not a Cornell Box object. Added to show caustic refraction
			geometry.emplace_back( std::make_shared<Geometry::Sphere>( Double3( -150, 350, 400 ), 60., 8 ) );

			// Update counters
			n_geometry = static_cast<std::uint32_t>( geometry.size() );
			n_emitter = static_cast<std::uint32_t>( emitter_list.size() );
			n_bxdf = static_cast<std::uint32_t>( bxdf.size() );

			if ( !n_geometry )
				return;

			// Calculate scene AABB
			auto [aabb_min, aabb_max] = geometry[0]->get_AABB();
			for ( auto& geo : geometry ) {
				auto [geo_min, geo_max] = geo->get_AABB();
				aabb_min = Min( aabb_min, geo_min );
				aabb_max = Max( aabb_max, geo_max );
			}
			scene_centre = ( aabb_min + aabb_max ) / 2.;
			scene_radius = ( aabb_max - aabb_min ).magnitude() / 2.;
		};
	};

};
