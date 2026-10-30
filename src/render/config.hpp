#pragma once

#include <algorithm>
#include <cstdint>

namespace Render {

	struct Config {
		// Image resolution
		std::uint16_t image_width{ 32 };
		std::uint16_t image_height{ 32 };
		// Samples per pixels
		std::uint16_t n_pixel_sample{ 1 };
		// Number of path vertices
		std::uint8_t max_path_length{ 5 };

		Config() = default;

		Config(
			std::uint16_t const& image_width,
			std::uint16_t const& image_height,
			std::uint16_t const& n_pixel_sample,
			std::uint8_t const& max_path_length
		)
			: image_width( image_width )
			, image_height( image_height )
			, n_pixel_sample( std::max<std::uint16_t>( 1, n_pixel_sample ) )
			, max_path_length( std::max<std::uint16_t>( 3, max_path_length ) ) {};
	};

};
