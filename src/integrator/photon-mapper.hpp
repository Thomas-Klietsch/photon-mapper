// Copyright (c) 2026 Thomas Klietsch, all rights reserved.
//
// Licensed under the GNU Lesser General Public License, version 3.0 or later
//
// This program is free software: you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or ( at your option ) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General
// Public License along with this program.If not, see < https://www.gnu.org/licenses/>.

#pragma once

#include <cstdint>

#include "../bxdf/shading_correction.hpp"
#include "../integrator/kd-tree.hpp"
#include "../integrator/photon.hpp"
#include "../mathematics/constant.hpp"
#include "../render/camera.hpp"
#include "../render/config.hpp"
#include "../render/scene.hpp"
#include "../render/sensor.hpp"

namespace Integrator {

	// Henrik Wann Jensen;
	// Global Illumination using Photon Maps (1996)

	// 1) Light/photon traces
	//   Store position, incident direction, energy of the photon:
	//     a) global (diffuse intersections)
	//     b) caustic (diffuse after specular event)
	// 2) k-d tree constructions
	// 3) Camera trace
	// 	 At diffuse intersection:
	//   a) low importance; sample k nearest points in global map
	//   b) high importance; sample emitters, caustic photon map, final gathering (# rays)

	class PhotonMapper final {

	private:

		// Number of surface bounces, from emitter(s) and camera
		std::uint8_t const max_path_length;

		Render::Camera const camera;
		Render::Scene const scene;
		Render::Sensor& sensor;

		Random::Mersenne prng;

		// Number of photons to emit
		std::uint32_t const n_global_sample;
		std::uint32_t const n_caustic_sample;

		// Traced photons
		std::vector<Integrator::Photon> global_photon;
		std::vector<Integrator::Photon> caustic_photon;

		// Number of photons to gather
		std::uint8_t const n_global_gather;
		std::uint8_t const n_caustic_gather;

		// k-d trees
		Integrator::KDTree global_kdtree;
		Integrator::KDTree caustic_kdtree;

		// Number of final gathering rays
		std::uint8_t const n_final_gathering;

		// Use high or low importance when camera tracing
		bool const f_high_importance;

	public:

		PhotonMapper() = delete;

		PhotonMapper(
			Render::Camera const& camera,
			Render::Sensor& sensor,
			Render::Scene const& scene,
			Render::Config const& config,
			// Number of global photons emitted
			std::uint32_t n_global_sample,
			// Number of k nearest neighbour global photons gathered
			std::uint8_t n_global_gather,
			// Use high or low importance
			bool const f_high_importance,
			// Number of (global) photons emitted
			std::uint32_t n_caustic_sample,
			// Number of k nearest neighbour caustic photons gathered
			std::uint8_t n_caustic_gather,
			// Number of final gathering rays
			std::uint8_t n_final_gathering
		)
			: camera( camera )
			, sensor( sensor )
			, scene( scene )
			, max_path_length( config.max_path_length )
			, n_global_sample( n_global_sample )
			, n_global_gather( n_global_gather )
			, f_high_importance( f_high_importance )
			, n_caustic_sample( n_caustic_sample )
			, n_caustic_gather( n_global_gather )
			, n_final_gathering( n_final_gathering ) {

			// Reserve memory
			global_photon.reserve( config.max_path_length * n_global_sample );
			caustic_photon.reserve( config.max_path_length * n_caustic_sample );
		};

		void process(
			std::uint32_t const iteration
		) {
			prng = Random::Mersenne( iteration * 0xc0c0 + 007 );

			// Clear storage
			global_photon.clear();
			caustic_photon.clear();

			// Trace global photons
			for ( std::uint32_t sample{ 0 }; sample < n_global_sample; ++sample )
				TraceEmission( false );
			// Build global photon maps
			global_kdtree.build( global_photon );

			if ( f_high_importance ) {
				// Trace caustic photons
				for ( std::uint32_t sample{ 0 }; sample < n_caustic_sample; ++sample )
					TraceEmission( true );
				// Build caustic photon maps
				caustic_kdtree.build( caustic_photon );
			}

			auto const [image_width, image_height] = camera.image_size();

			// Trace camera
			for ( std::uint16_t y{ 0 }; y < image_height; ++y )
				for ( std::uint16_t x{ 0 }; x < image_width; ++x )
					sensor.add_pixel( x, y, TraceCamera( x, y ) );
		};

	private:

		void TraceEmission(
			// True when tracing caustic photons
			bool const f_caustic_trace
		) {
			std::uint32_t const emitter_id = scene.random_emitter( prng );
			auto const [p_emitter, emitter_select_probability]
				= scene.emitter( emitter_id );

			auto const [emitter_factor, emitter_point, emitter_direction, emitter_pdf_W, emitter_pdf_A]
				= p_emitter->emit( prng );

			Colour throughput = emitter_factor / ( emitter_select_probability * emitter_pdf_W * emitter_pdf_A );

			Ray::Section ray( emitter_point, emitter_direction, EPSILON_RAY );
			std::uint8_t depth{ 1 };

			// True if previous BxDF event was a (delta) dirac/specular
			bool f_previous_dirac{ false };

			while ( 1 ) {
				auto [f_hit, hit_distance, idata] = scene.intersect( ray );
				if ( !f_hit )
					return;

				std::shared_ptr<BxDF::Polymorphic> const& p_material = scene.material( idata.material_id );
				auto [bxdf_colour, bxdf_direction, bxdf_event, bxdf_pdf_W, bxdf_cos_theta]
					= p_material->sample( idata, BxDF::TraceMode::Importance, prng );

				switch ( bxdf_event ) {
					default:
					case BxDF::Event::None:
					case BxDF::Event::Emission: {
						return;
					}
					case BxDF::Event::Diffuse: {
						if ( f_caustic_trace ) {
							// Store caustic photon
							if ( f_previous_dirac )
								caustic_photon.emplace_back( Photon{ idata.point, idata.from_direction, throughput } );
						}
						else {
							// Store photon
							global_photon.emplace_back( Photon{ idata.point, idata.from_direction, throughput } );
						}

						// Update path trace
						f_previous_dirac = false;
						throughput *= bxdf_colour * ( ( bxdf_cos_theta / bxdf_pdf_W ) * ShadingCorrection( bxdf_direction, idata.from_direction, idata, BxDF::TraceMode::Importance ) );
						break;
					}
					case BxDF::Event::Reflect:
					case BxDF::Event::Transmit: {
						f_previous_dirac = true;
						throughput *= bxdf_colour * ShadingCorrection( bxdf_direction, idata.from_direction, idata, BxDF::TraceMode::Importance );
						break;
					}
				} // end switch

				if ( ++depth > max_path_length || throughput.is_black() )
					break;

				ray = Ray::Section( idata.point, bxdf_direction, EPSILON_RAY );
			} // end trace loop

			return;
		};

		Colour TraceCamera(
			std::uint16_t const x,
			std::uint16_t const y
		) {
			Ray::Section ray = camera.generate_ray( x, y, prng );

			std::uint8_t depth{ 1 };

			Colour throughput{ Colour::White }; // Affected by each material on the traced path
			Colour accumulate( Colour::Black ); // Sum of all photon maps and NEE contributions

			// Random emitter, and random point on it
			std::uint32_t const emitter_id = scene.random_emitter( prng );
			auto const [p_emitter, emitter_select_probability]
				= scene.emitter( emitter_id );
			auto const emitter_point = p_emitter->sample( prng );

			bool f_previous_dirac{ false }; // Dirac state of previous intersection material
			bool f_dirac_path{ true }; // Whether or not the whole path is dirac materials

			// Trace loop
			while ( 1 ) {
				auto const [f_hit, hit_distance, idata] = scene.intersect( ray );
				if ( !f_hit )
					return accumulate;

				std::shared_ptr<BxDF::Polymorphic> const& p_material = scene.material( idata.material_id );
				auto const [bxdf_colour, bxdf_direction, bxdf_event, bxdf_pdf_W, bxdf_cos_theta]
					= p_material->sample( idata, BxDF::TraceMode::Radiance, prng );

				switch ( bxdf_event ) {
					default:
					case BxDF::Event::None: {
						return accumulate;
					}
					case BxDF::Event::Emission: {
						if ( depth == 1 || f_dirac_path || f_previous_dirac ) {
							// Evaluate:
							//   direct hit on emitter from camera (no material or NEE has been done)
							//   a complete dirac path (no NEE)
							//   previous surface was dirac (e.g. no NEE)
							// Otherwise pixel is oversampled with NEE on diffuse surfaces
							auto const [p_emitter_hit, select_prb] = scene.emitter( p_material->emitter_id() );
							Colour const emission = p_emitter_hit->radiance( idata.point, -ray.direction );
							return accumulate + throughput * emission;
						}
						return accumulate;
					}
					case BxDF::Event::Diffuse: {
						if ( f_high_importance ) {
							// Sample emitter
							{
								// Ld is the same as Path tracer NEE (next event estimator)
								Double3 const delta = emitter_point - idata.point;
								Double3 const evaluate_direction = delta.normalise();
								std::double_t const evaluate_distance = delta.magnitude();
								Ray::Section const evaluate_ray( idata.point, evaluate_direction, EPSILON_RAY );
								if ( !scene.occluded( evaluate_ray, evaluate_distance - 2. * EPSILON_RAY ) ) {
									auto const [nee_factor, nee_pdf_W, nee_cos_theta]
										= p_material->evaluate( evaluate_direction, idata.from_direction, idata, BxDF::TraceMode::Radiance );
									auto const [emitter_pdf_A, emitter_cos_theta]
										= p_emitter->pdf_Le( emitter_point, -evaluate_direction );

									if ( ( nee_cos_theta > 0.f ) && ( emitter_cos_theta > 0.f ) ) {
										std::double_t const emitter_pdf_A_to_W = emitter_pdf_A * emitter_select_probability * evaluate_distance * evaluate_distance / emitter_cos_theta;
										Colour const emission = p_emitter->radiance( emitter_point, -evaluate_direction );
										accumulate += throughput * emission * nee_factor * ( nee_cos_theta / emitter_pdf_A_to_W );
									}
								}
							}
							// Sample caustic photon map
							{
								// Gather k nearest photons
								auto const [photon_id, largest_radius2] = caustic_kdtree.knn_query( idata.point, n_caustic_gather );

								std::uint32_t const count = photon_id.size();
								if ( count ) {
									Colour sum{ Colour::Black };
									for ( std::size_t i{ 0 }; i < count; ++i ) {
										Photon const& particle = caustic_photon[photon_id[i]];
										auto const [eval_factor, eval_pdf_W, eval_cos_theta]
											= p_material->evaluate( -ray.direction, particle.in_ray_dir, idata, BxDF::TraceMode::Radiance );
										sum += particle.energy * eval_factor;
									}
									accumulate += throughput * sum / ( pi * largest_radius2 * n_caustic_sample );
								}
							}
							// Sample indirect (final gathering)
							if ( n_final_gathering ) {
								Colour sum{ Colour::Black };

								// Using the already sampled BxDF
								sum += bxdf_colour * EvaluateIndirect( idata.point, bxdf_direction ) * ( bxdf_cos_theta / bxdf_pdf_W );

								// Generate n-1 samples
								for ( std::uint8_t i{ 1 }; i < n_final_gathering; ++i ) {
									auto [bxdf_colour, bxdf_sample_direction, bxdf_event, bxdf_pdf_W, bxdf_cos_theta]
										= p_material->sample( idata, BxDF::TraceMode::Importance, prng );
									if ( bxdf_cos_theta > 0. )
										sum += bxdf_colour * EvaluateIndirect( idata.point, bxdf_sample_direction ) * ( bxdf_cos_theta / bxdf_pdf_W );
								}

								accumulate += throughput * sum / n_final_gathering;
							}
						}
						else {
							// Gather k nearest photons
							auto const [photon_id, largest_radius2] = global_kdtree.knn_query( idata.point, n_global_gather );
							std::uint32_t const count = photon_id.size();
							if ( count ) {
								Colour sum{ Colour::Black };
								for ( std::uint32_t i{ 0 }; i < count; ++i ) {
									Photon const& particle = global_photon[photon_id[i]];
									auto const [eval_factor, eval_pdf_W, eval_cos_theta]
										= p_material->evaluate( -ray.direction, particle.in_ray_dir, idata, BxDF::TraceMode::Radiance );
									sum += particle.energy * eval_factor;
								}
								accumulate += throughput * sum / ( pi * largest_radius2 * n_global_sample );
							}
						}

						// Photon mapping only use first diffuse surface
						return accumulate;
					}
					case BxDF::Event::Reflect:
					case BxDF::Event::Transmit: {
						f_previous_dirac = true;
						throughput *= bxdf_colour;
						break;
					}
				} // end switch

				if ( ++depth > max_path_length || throughput.is_black() )
					return accumulate;

				ray = Ray::Section( idata.point, bxdf_direction, EPSILON_RAY );
			} // end trace loop
		};

		inline Colour EvaluateIndirect(
			Double3 const& point,
			Double3 const& direction
		) {
			auto const [f_hit, hit_distance, idata] = scene.intersect( Ray::Section( point, direction ) );

			if ( f_hit ) {
				std::shared_ptr<BxDF::Polymorphic> const& p_material = scene.material( idata.material_id );
				if ( p_material->f_diffuse() ) {
					// Gather k nearest photons
					auto const [photon_id, largest_radius2] = global_kdtree.knn_query( idata.point, n_global_gather );

					std::uint32_t const count = photon_id.size();

					if ( count ) {
						Colour sum{ Colour::Black };
						for ( std::size_t i{ 0 }; i < count; ++i ) {
							Photon const& particle = global_photon[photon_id[i]];
							auto const [eval_factor, eval_pdf_W, eval_cos_theta]
								= p_material->evaluate( -direction, particle.in_ray_dir, idata, BxDF::TraceMode::Radiance );
							sum += particle.energy * eval_factor;
						}
						return sum / ( pi * largest_radius2 * n_global_sample );
					}
				}
			}

			return Colour::Black;
		};

	}; // end class
};
