#pragma once

#include <cmath>
#include <cstdint>
#include <iostream>

#include "../mathematics/double3.hpp"
#include "../random/mersenne.hpp"
#include "../ray/section.hpp"
#include "../render/config.hpp"

namespace Render {

	class Camera final {
		// Pinhole camera.
		// In the real world, the image plane is behind the pinhole.
		// But it is simpler to visualise when in front of it.

		// Mathness:
		//   The use of a 35mm film sensor and a focal length of 50mm, is simply because I use a full frame camera.

	private:

		// Sensor dimensions in mm
		std::double_t const sensor_width = 36.;
		std::double_t const sensor_height = 24.;

		std::double_t aspect_ratio{ 1. };

		std::uint16_t image_width{ 1 };
		std::uint16_t image_height{ 1 };

		Double3 position{ Double3::Zero };

		// View direction
		Double3 forward{ Double3::Y };

		// Image/sensor/film plane vectors
		Double3 right{ Double3::X };
		Double3 up{ Double3::Z };

		// In m^2
		std::double_t sensor_area{ 1. };

		// In mm
		std::double_t focal_length{ 1. };

		// Pinhole camera lens has no area, a value of one (1) means no effect
		std::double_t const lens_area{ 1. };

		// Scalar for sensor vectors
		std::double_t scalar{ 1. };

		// Convert pixels to right/up scale
		std::double_t dx{ 1. };
		std::double_t dy{ 1. };

	public:

		Camera() {};

		Camera(
			Double3 const& position,
			Double3 const& look_at,
			std::double_t const focal_length, // in mm
			Render::Config const& config
		)
			: position( position )
			, focal_length( focal_length )
			, image_width( config.image_width )
			, image_height( config.image_height )
			, aspect_ratio( static_cast<std::double_t>( config.image_width ) / static_cast<std::double_t>( config.image_height ) ) {
			// Placing the sensor plane at a distance of one (1) unit away, simplifies evaluation of pdf's. Planes of sensor and lens are parallel.
			// Areas and sensor vectors needs to scaled.
			/*
				^       ^
			   / \      |
			  / | \     focal length
			 /  |  \    |
			/   |   \   v
			-------->
			sensor vectors
			*/
			scalar = sensor_width / focal_length; // In reality a ratio, but named scalar to avoid confusion with aspect ratio

			// Square pixels are assumed, rescale sensor to distance one (1)
			// sensor_area = sensor_width * sensor_width / ( aspect_ratio * focal_length * focal_length );
			sensor_area = scalar * scalar / aspect_ratio;

			Double3 const delta = look_at - position;
			if ( delta.magnitude() < 0.001 ) {
				std::cout << "Camera position and view target are too close together!" << std::endl;
				throw std::invalid_argument( "Camera position and view target are too close together!" );
			}
			else
				forward = delta.normalise();

			// If view direction and World Up (Z axis) is collinear (or close to it), change World up axis.
			Double3 world_up = std::abs( forward.dot( Double3::Z ) ) < 0.99 ? Double3::Z : Double3::X;
			right = ( forward.cross( world_up ) ).normalise();
			up = -( right.cross( forward ) ).normalise();

			// Convertion factors for pixel to sensor
			dx = 1. / static_cast<std::double_t>( image_width - 1 );
			dy = 1. / static_cast<std::double_t>( image_height - 1 );
		};

		Ray::Section generate_ray(
			std::uint16_t const& x,
			std::uint16_t const& y,
			Random::Mersenne& prng
		) const {
			std::float_t const rnd_x = prng.get_float() - 0.5f;
			std::float_t const rnd_y = prng.get_float() - 0.5f;

			Double3 dir = forward + right * scalar * ( ( static_cast<std::float_t>( x ) + rnd_x ) * dx - 0.5f ) + up * scalar / aspect_ratio * ( ( static_cast<std::float_t>( y ) + rnd_y ) * dy - 0.5f );

			return Ray::Section( position, dir.normalise() );
		};

		// Sample a random point on a lens
		Double3 sample_lens(
			Random::Mersenne& prng
		) const {
			return position;
		};

		// Find sensor pixel (pinhole)
		std::tuple<std::float_t, std::float_t, bool> sensor(
			Double3 const& world_point,
			Double3 const& lens_point
		) const {
			Double3 evaluate_direction = ( world_point - position ).normalise();
			std::double_t const cos_theta = evaluate_direction.dot( forward );
			if ( cos_theta <= 0. )
				return { {}, {}, false };

			// Correct length of vector between lens and sensor plane, and check if on sensor
			// evaluate_direction = evaluate_direction / cos_theta;
			std::float_t const x = evaluate_direction.dot( right ) / ( cos_theta * scalar );
			std::float_t const y = evaluate_direction.dot( up ) / ( cos_theta * scalar / aspect_ratio );
			if ( std::abs( x ) > .5f || std::abs( y ) > 0.5f )
				return { {}, {}, false };

			return { ( x + 0.5f ) * image_width, ( y + 0.5f ) * image_height, true };
		};

		// PDF of sampling a ray in this direction (pdf_We)
		std::float_t pdf(
			Double3 const& world_point,
			Double3 const& lens_point
		) const {
			Double3 evaluate_direction = ( world_point - position ).normalise();
			std::double_t const cos_theta = evaluate_direction.dot( forward );
			if ( cos_theta <= 0. )
				return 0.;

			// Correct length of vector between lens and sensor plane, and check if on sensor
			// evaluate_direction = evaluate_direction / cos_theta;
			std::float_t const x = evaluate_direction.dot( right ) / ( cos_theta * scalar );
			std::float_t const y = evaluate_direction.dot( up ) / ( cos_theta * scalar / aspect_ratio );
			if ( std::abs( x ) > .5f || std::abs( y ) > 0.5f )
				return 0.;

			// See Veach and/or bpt_veach
			return 1. / ( sensor_area / ( cos_theta * cos_theta * cos_theta ) );
		};

		std::tuple<std::uint16_t, std::uint16_t> image_size() const { return { image_width, image_height }; };

		// No aperture, so impossible for a ray to hit it
		bool is_dirac() const { return true; };
	};

};
